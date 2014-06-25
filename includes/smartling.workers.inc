<?php

/**
 * @file
 * Workers functions for Smartling module.
 */

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_download_update_translated_item_process($entity_data) {

  if (smartling_is_configured() && smartling_supported_type($entity_data->entity_type, $entity_data->bundle)) {
    $log = new SmartlingLog();
    $smartling_settings = smartling_settings_get_handler();

    $entity_type = $entity_data->entity_type;
    $d_locale = $entity_data->target_language;
    $rid = $entity_data->rid;
    $link_to_entity = smartling_get_link_to_entity($rid, $entity_type);
    $file_name = $entity_data->file_name;
    $file_name = substr($file_name, 0, strlen($file_name) - 4);
    $translated_filename = smartling_clean_filename($file_name . '_' . $d_locale . '.xml', TRUE);

    // Init api object.
    $api = new SmartlingApiWrapper($smartling_settings, $log);

    $download_result = $api->downloadFile($entity_data, $link_to_entity);
    if ($download_result !== FALSE) {
      // This is a $download_result
      $xml = new DOMDocument();
      $xml->loadXML($download_result);

      $save = smartling_save_xml($xml, $rid, $d_locale, $translated_filename, TRUE, $entity_type);
      if (is_object($save)) {
        smartling_update_translated_fields($rid, $d_locale, $entity_type, $entity_data->bundle);
      }
    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_check_status_process($args) {
  if (smartling_is_configured()) {
    $log = new SmartlingLog();
    $smartling_settings = smartling_settings_get_handler();
    $link_to_entity = smartling_get_link_to_entity($args->rid, $args->entity_type);
    $entity_data = smartling_entity_load_by_conditions(array(
      'eid' => $args->eid,
      'rid' => $args->rid,
      'target_language' => $args->d_locale,
    ));

    $api = new SmartlingApiWrapper($smartling_settings, $log);
    $result = $api->getStatus($args, $entity_data, $link_to_entity);

    if (!empty($result)) {
      if ($result['response_data']->approvedStringCount == $result['response_data']->completedStringCount) {

        $smartling_queue = DrupalQueue::get('smartling_download');
        $smartling_queue->createQueue();
        $smartling_queue->createItem($result['entity_data']);
      }

      smartling_entity_data_save($result['entity_data']);
    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_send_to_translate_process($args) {
  if (smartling_is_configured()) {
    $log = new SmartlingLog();
    $smartling_settings = smartling_settings_get_handler();
    $entity_type = $args->entity_type;
    // Check filename length.
    if (!smartling_check_filename_length($args->title, $entity_type, $args->rid)) {
      return NULL;
    }

    $entity_data_array = array();
    foreach ($args->d_locales as $d_locale) {
      $entity_data = smartling_entity_load_by_conditions(array(
        'rid' => $args->rid,
        'entity_type' => $entity_type,
        'target_language' => $d_locale,
      ));

      if ($entity_data == FALSE) {
        $entity_data = smartling_entity_data_create();
        $entity_data->rid = $args->rid;
        $entity_data->entity_type = $args->entity_type;
        $entity_data->bundle = $args->bundle;
        $entity_data->submission_date = time();
      }

      $entity_data->original_language = $args->original_language;
      $entity_data->target_language = $d_locale;
      $entity_data->translated_file_name = FALSE;
      $entity_data->submitter = $args->submitter;
      $entity_data->progress = 0;
      smartling_entity_data_save($entity_data);
      $entity_data_array[] = $entity_data;
    }

    $saves[] = smartling_build_xml($args->rid, $args->d_locales, $args->title, $entity_type);

    if (!in_array(FALSE, $saves) && is_string($saves[0])) {
      global $base_url;
      $smartling_settings = smartling_settings_get_handler();
      $file_name = $saves[0];
      $file_name_unic = $file_name;
      $file_path = smartling_clean_filename(smartling_get_dir($file_name), TRUE);
      $file_type = 'xml';

      // Init api object.
      $api = new SmartlingAPI($smartling_settings->getApiUrl(), $smartling_settings->getKey(), $smartling_settings->getProjectId(), SMARTLING_PRODUCTION_MODE);

      foreach ($args->d_locales as $d_locale) {
        $locales_to_approve[] = smartling_convert_locale_drupal_to_smartling($d_locale);
      }

      $upload_params = new FileUploadParameterBuilder();
      $upload_params->setFileUri($file_name_unic)
        ->setFileType($file_type)
        ->setLocalesToApprove($locales_to_approve)
        ->setOverwriteApprovedLocales(0)
        ->setApproved(0);
      if ($smartling_settings->getCallbackUrlUse()) {
        $upload_params->setCallbackUrl($base_url . '/smartling/callback/' . variable_get('cron_key', 'drupal'));
      }

      $upload_params = $upload_params->buildParameters();

      // Try to upload file.
      $upload_result = $api->uploadFile($file_path, $upload_params);
      $upload_result = json_decode($upload_result);

      if ($api->getCodeStatus() == 'SUCCESS') {
        foreach ($entity_data_array as $entity) {
          smartling_set_translation_status($entity, SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE);
        }

        $log->setMessage('Smartling uploaded @entity_type @nid for locales: @locales')
          ->setVariables(array(
            '@entity_type' => $entity_type,
            '@nid' => $args->rid,
            '@locales' => implode('; ', $locales_to_approve),
          ))
          ->setLink(l(t('View file'), $file_path))
          ->execute();

        if (module_exists('rules') && ($entity_type == 'node')) {
          $node_event = node_load($args->rid);
          rules_invoke_event('smartling_uploading_original_to_smartling_event', $node_event);
        }
      }
      elseif (is_object($upload_result)) {
        $link_to_entity = smartling_get_link_to_entity($args->rid, $entity_type);
        $log->setMessage('Smartling failed to upload xml file: <br/>
          Project Id: @project_id <br/>
          Action: upload <br/>
          URI: @file_uri <br/>
          Error: response code -> @code and message -> @message
          Upload aparms: @upload_params')
          ->setVariables(array(
            '@project_id' => $smartling_settings->getProjectId(),
            '@file_uri' => $file_name_unic,
            '@code' => $upload_result->response->code,
            '@message' => $upload_result->response->messages[0],
            '@upload_params' => $upload_params,
          ))
          ->setConsiderLog(FALSE)
          ->setSeverity(WATCHDOG_ERROR)
          ->setLink($link_to_entity)
          ->execute();

        foreach ($entity_data_array as $entity) {
          smartling_set_translation_status($entity, SMARTLING_STATUS_EVENT_FAILED_UPLOAD);
        }
      }
    }
  }
}

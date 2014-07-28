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
    $log = smartling_log_get_handler();
    $smartling_settings = smartling_settings_get_handler();

    $entity_type = $entity_data->entity_type;
    $d_locale = $entity_data->target_language;
    $rid = $entity_data->rid;
    $link_to_entity = smartling_get_link_to_entity($rid, $entity_type);
    $file_name = $entity_data->file_name;
    $file_name = substr($file_name, 0, strlen($file_name) - 4);
    $translated_filename = smartling_clean_filename($file_name . '_' . $d_locale . '.xml', TRUE);

    // Init api object.
    $api = drupal_container()->get('smartling.api_wrapper'); //new SmartlingApiWrapper($smartling_settings, $log);

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
    $link_to_entity = smartling_get_link_to_entity($args->rid, $args->entity_type);
    $entity_data = smartling_entity_load_by_conditions(array(
      'eid' => $args->eid,
      'rid' => $args->rid,
      'target_language' => $args->d_locale,
    ));

    $api = drupal_container()->get('smartling.api_wrapper'); //new SmartlingApiWrapper($smartling_settings, $log);
    $result = $api->getStatus($entity_data, $link_to_entity);

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

      $file_name = $saves[0];
      $file_name_unic = $file_name;
      $file_path = smartling_clean_filename(smartling_get_dir($file_name), TRUE);
      $file_type = 'xml';



      $locales_to_approve = array();
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

      $link_to_entity = smartling_get_link_to_entity($args->rid, $entity_type);

      // Init api object.
      $api = drupal_container()->get('smartling.api_wrapper');
      $result_status = $api->uploadFile($file_path, $upload_params, $args, $link_to_entity);

      foreach ($entity_data_array as $entity) {
        smartling_set_translation_status($entity, $result_status);
      }

      if ($result_status == SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE) {
        if (module_exists('rules') && ($entity_type == 'node')) {
          $node_event = node_load($args->rid);
          rules_invoke_event('smartling_uploading_original_to_smartling_event', $node_event);
        }
      }
    }
  }
}

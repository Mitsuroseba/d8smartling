<?php

/**
 * @file
 * Workers functions for Smartling module.
 */

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_download_update_translated_item_process($eid) {
  $smartling_entity = entity_load_single('smartling_entity_data', $eid);
  if ($smartling_entity && smartling_is_configured() && smartling_supported_type($smartling_entity->entity_type, $smartling_entity->bundle)) {
    $processor = smartling_get_entity_processor($smartling_entity);
    $processor->downloadTranslation();
    $processor->updateEntityFromXML();
//    $entity_type = $smartling_entity->entity_type;
//    $d_locale = $smartling_entity->target_language;
//    $rid = $smartling_entity->rid;
//    $file_name = $smartling_entity->file_name;
//
//    $file_name = substr($file_name, 0, strlen($file_name) - 4);
//    $translated_filename = smartling_clean_filename($file_name . '_' . $d_locale . '.xml', TRUE);
//
//    // Init api object.
//    $api = drupal_container()->get('smartling.api_wrapper');
//
//    $download_result = $api->downloadFile($smartling_entity);
//
//    if ($download_result !== FALSE) {
//      // This is a $download_result
//      $xml = new DOMDocument();
//      $xml->loadXML($download_result);
//
//      $save = smartling_save_xml($xml, $rid, $d_locale, $translated_filename, TRUE, $entity_type);
//      if (is_object($save)) {
//        smartling_update_translated_fields($smartling_entity);
//      }
//    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_check_status_process($eid) {
  $smartling_entity = entity_load_single('smartling_entity_data', $eid);

  if (smartling_is_configured()) {
    $api = drupal_container()->get('smartling.api_wrapper');
    $result = $api->getStatus($smartling_entity);

    if (!empty($result)) {
      if ($result['response_data']->approvedStringCount == $result['response_data']->completedStringCount) {

        $smartling_queue = DrupalQueue::get('smartling_download');
        $smartling_queue->createQueue();
        $smartling_queue->createItem($eid);
      }

      smartling_entity_data_save($result['entity_data']);
    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_send_to_translate_process($eid) {
  $smartling_entity = entity_load_single('smartling_entity_data', $eid);

  if ($smartling_entity && smartling_is_configured()) {
    $entity_type = $smartling_entity->entity_type;

//    $entity_data_array = array();
    // @todo why do we need internal loops?
//    foreach ($smartling_entity->d_locales as $d_locale) {
//      $entity_data = smartling_entity_load_by_conditions(array(
//        'rid' => $smartling_entity->rid,
//        'entity_type' => $entity_type,
//        'target_language' => $d_locale,
//      ));
//
//      if ($entity_data == FALSE) {
//        $entity_data = smartling_entity_data_create();
//        $entity_data->rid = $smartling_entity->rid;
//        $entity_data->entity_type = $smartling_entity->entity_type;
//        $entity_data->bundle = $smartling_entity->bundle;
//        $entity_data->submission_date = time();
//      }
//
//      $entity_data->original_language = $smartling_entity->original_language;
//      $entity_data->target_language = $d_locale;
//      $entity_data->translated_file_name = FALSE;
//      $entity_data->submitter = $smartling_entity->submitter;
//      $entity_data->progress = 0;
//      smartling_entity_data_save($entity_data);
//      $entity_data_array[] = $entity_data;
//    }

    $processor = smartling_get_entity_processor($smartling_entity);
    $content_to_translate = $processor->exportContentToTranslation();
    $file_name = $processor->buildXmlFileName();
    $xml = smartling_build_xml($file_name, $content_to_translate, $smartling_entity->rid);
    if ($xml instanceof DOMNode) {
      $success = smartling_save_xml($xml, $smartling_entity, $file_name, FALSE);
    }

    if ($success) {
      $file_name_unic = $file_name;
      $file_path = drupal_realpath(smartling_clean_filename(smartling_get_dir($file_name), TRUE));

      // Init api object.
      $api = drupal_container()->get('smartling.api_wrapper');
      $result_status = $api->uploadFile($file_path, $file_name_unic, array($smartling_entity->target_language));

      $processor->setProgressStatus($result_status);

      if ($result_status == SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE) {
        if (module_exists('rules') && ($entity_type == 'node')) {
          $node_event = node_load($smartling_entity->rid);
          rules_invoke_event('smartling_uploading_original_to_smartling_event', $node_event);
        }
      }
    }
  }
}

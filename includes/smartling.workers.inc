<?php

/**
 * @file
 * Workers functions for Smartling module.
 */

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_download_update_translated_item_process($eid) {
  $status = FALSE;
  $smartling_entity = entity_load_single('smartling_entity_data', $eid);
  if ($smartling_entity && smartling_is_configured() && smartling_supported_type($smartling_entity->entity_type, $smartling_entity->bundle)) {
    $processor = smartling_get_entity_processor($smartling_entity);
    if ($processor->downloadTranslation()) {
      $status = $processor->updateEntityFromXML();
    }

  }

  if ($status) {
    drupal_set_message(t('The translation was successfully downloaded.'));
  }
  else {
    drupal_set_message(t('Ooops, something went wrong while we tried to download/apply the translation. Please see the logs.'), 'error');
  }
  return $status;
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
      if (($result['response_data']->approvedStringCount == $result['response_data']->completedStringCount) && ($args->entity_type != 'smartling_interface_entity')) {
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
function smartling_queue_send_to_translate_process($eids) {
  if (!is_array($eids)) {
    $eids = array($eids);
  }

  $target_locales = array();
  $entity_data_array = array();

  foreach($eids as $eid) {
    $smartling_entity = entity_load_single('smartling_entity_data', $eid);
    $target_locales[] = $smartling_entity->target_language;
    $entity_data_array[] = $smartling_entity;
  }

  //$smartling_entity = entity_load_single('smartling_entity_data', $eid);

  if (!$smartling_entity || !smartling_is_configured()) {
    return;
  }

  $entity_type = $smartling_entity->entity_type;

  $processor = smartling_get_entity_processor($smartling_entity);
  $file_name = $processor->buildXmlFileName();
  $xml = smartling_build_xml($processor, $smartling_entity->rid);
  $success = FALSE;
  if ($xml instanceof DOMNode) {
    $success = TRUE;
    foreach ($entity_data_array as $entity) {
      $success = (smartling_save_xml($xml, $entity, $file_name, FALSE))?$success:FALSE;
    }
  }

  if ($success) {
    $file_name_unic = $file_name;
    $file_path = drupal_realpath(smartling_clean_filename(smartling_get_dir($file_name), TRUE));

    // Init api object.
    $api = drupal_container()->get('smartling.api_wrapper');
    $result_status = $api->uploadFile($file_path, $file_name_unic, 'xml', $target_locales);

    //$processor->setProgressStatus($result_status);
    foreach ($entity_data_array as $entity) {
      $proc = smartling_get_entity_processor($entity);
      $proc->setProgressStatus($result_status);
    }

    if ($result_status == SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE) {
      if (module_exists('rules') && ($entity_type == 'node')) {
        $node_event = node_load($smartling_entity->rid);
        rules_invoke_event('smartling_uploading_original_to_smartling_event', $node_event);
      }
    }
  }
  //@todo We lost this functionality in OOP branch, but it was introduced in 2.x. Must be restored
  else {
    foreach ($entity_data_array as $entity) {
      smartling_set_translation_status($entity, SMARTLING_STATUS_EVENT_FAILED_UPLOAD);
    }
  }
}

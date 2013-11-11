<?php

/**
 * @file
 */

/**
 * Worker callback defined in hook_cron_queue_info()
 */
function smartling_queue_download_update_translated_item_process($entity_data) {
  if (smartling_is_configured() === TRUE) {
    $d_locale = $entity_data->target_language;
    $s_locale = smartling_convert_locale_drupal_to_smatrtling($d_locale);
    $rid = $entity_data->rid;

    $file_name = $entity_data->file_name;

    $file_path = SMARTLING_DIRECTORY . '/' . $file_name;
    $file_uri = file_create_url('public://smartling/' . $file_name);
    //$new_file_uri = SMARTLING_DIRECTORY . '/new_' . $file_name;
    $file_name = substr($file_name, 0, strlen($file_name) - 4);
    $new_file_uri = file_create_url('public://smartling/' . $file_name . '_' . $s_locale . '.xml');
    $translated_filename = $file_name . '_' . $s_locale . '.xml';

    $key = variable_get('smartling_key');
    $project_id = variable_get('smartling_project_id');
    $mode = variable_get('smartling_mode');
    //init api object
    $api = new SmartlingAPI($key, $project_id, $mode);

    $retrieval_type = variable_get('smartling_retrieval_type');
    $download_param = array(
      'retrievalType' => $retrieval_type,
    );

    smartling_watchdog('Smartling upload xml file for node id - @nid.', TRUE, array('@nid' => $rid), WATCHDOG_INFO, l(t('View file'), $file_path));

    //try to download file
    $download_result = $api->downloadFile($file_uri, $s_locale, $download_param);
    //This is a $download_result
    $xml = new DOMDocument();
    $xml->loadXML($download_result);

    $save = smartling_save_xml($xml, $rid, $d_locale, $translated_filename, TRUE);
    if (is_object($save)) {
      smartling_update_translated_fields($rid, $d_locale);
    }

    //try get files list
    //$result = $api->getList($locale);
    //This is a get list
    //try to import
    //$result = $api->import($new_file_uri, $file_type, $locale, $file_path, true, $translation_state);
    //This is a import file
    //try to rename file
    //$result = $api->renameFile($file_uri, $new_file_uri);
    //This is a rename file
    //try to delete file
    //$result = $api->deleteFile($new_file_uri);
    //This is delete file
  }
}

/**
 * Worker callback defined in hook_cron_queue_info()
 */
function smartling_queue_check_status_process($args) {
  if (smartling_is_configured() === TRUE) {
    $rid = $args->rid;
    $d_locale = $args->d_locale;

    $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $d_locale));
    if ($entity_data !== FALSE) {
      $file_name = $entity_data->file_name;

      $file_uri = file_create_url('public://smartling/' . $file_name);

      $key = variable_get('smartling_key');
      $project_id = variable_get('smartling_project_id');
      $mode = variable_get('smartling_mode');
      //init api object
      $api = new SmartlingAPI($key, $project_id, $mode);
      //try to retrieve file status
      $s_locale = smartling_convert_locale_drupal_to_smatrtling($d_locale);
      $status_result = $api->getStatus($file_uri, $s_locale);
      $status_result = json_decode($status_result);

      //This is a get status
      if ($status_result->response->code == 'SUCCESS') {
        // this code imitation translate file in service
        $finish_translate = rand(1, 2);
        if ($finish_translate == 1) {
          $status_result->response->data->completedStringCount = $status_result->response->data->approvedStringCount;
        }

        smartling_watchdog('Smartling check status for node id - @nid. approvedString = @as, completedString = @cs', TRUE, array(
          '@nid' => $args->rid,
          '@as' => $status_result->response->data->approvedStringCount,
          '@cs' => $status_result->response->data->completedStringCount
            ), WATCHDOG_INFO, l(t('View file'), $file_uri));

        if ($status_result->response->data->approvedStringCount == $status_result->response->data->completedStringCount) { // if true, file translated
          $entity_data->progress = 100;
          $entity_data->download = 0;

          $smartling_queue = DrupalQueue::get('smartling_download_update_translated_item');
          $smartling_queue->createQueue();
          $smartling_queue->createItem($entity_data);

          smartling_entity_data_save($entity_data);
        }
        else {
          // if progress != 100 add new task for this item
          $smartling_queue = DrupalQueue::get('smartling_check_status');
          $smartling_queue->createQueue();
          $smartling_queue->createItem($args);
        }
      }
      else {
        smartling_watchdog('Smartling check status for node id - @nid is FAIL! Smartling entity not exist.', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
      }
    }
    else {
      smartling_watchdog('Smartling check status for node id - @nid is FAIL!', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info()
 */
function smartling_queue_send_to_translate_process($args) {
  if (smartling_is_configured() === TRUE) {
    $d_locale = smartling_convert_locale_drupal_to_smatrtling($args->s_locale, TRUE);

    $entity_data = smartling_entity_load_by_conditions(array('rid' => $args->rid, 'target_language' => $d_locale));

    if ($entity_data == FALSE) {
      $entity_data = smartling_entity_data_create();
      $entity_data->rid = $args->rid;
      $entity_data->bundle = $args->type;
    }

    $entity_data->original_language = $args->original_language;
    $entity_data->target_language = $d_locale;
    $entity_data->translated_file_name = FALSE;
    $entity_data->submitter = $args->submitter;
    $entity_data->progress = 0;
    $entity_data->submission_date = time();
    smartling_entity_data_save($entity_data);

    $save = smartling_build_xml_by_node_id($args->rid, $d_locale, $args->title);

    if (($save !== FALSE) && is_string($save)) {
      $file_name = $save;
      $file_path = SMARTLING_DIRECTORY . '/' . $file_name;
      $file_uri = file_create_url('public://smartling/' . $file_name);
      $file_name = substr($file_name, 0, strlen($file_name) - 4);
      $file_type = 'xml';

      $key = variable_get('smartling_key');
      $project_id = variable_get('smartling_project_id');
      $mode = variable_get('smartling_mode');
      //init api object
      $api = new SmartlingAPI($key, $project_id, $mode);

      $upload_params = array(
        'approved' => TRUE,
      );

      //try to upload file
      $upload_result = $api->uploadFile($file_path, $file_type, $file_uri, $upload_params);
      $upload_result = json_decode($upload_result);

      if ($upload_result->response->code == 'SUCCESS') {
        smartling_watchdog('Smartling upload xml file for node id - @nid.', TRUE, array('@nid' => $args->rid), WATCHDOG_INFO, l(t('View file'), $file_path));
      }
      else {
        // add new queue item if some error
        $smartling_queue = DrupalQueue::get('smartling_send_to_translate');
        $smartling_queue->createQueue();
        $smartling_queue->createItem($args);
        smartling_watchdog('Smartling upload xml file for node id - @nid is FAIL!', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
      }
      //try get files list
      //$result = $api->getList($locale);
      //This is a get list
      //try to import
      //$result = $api->import($new_file_uri, $file_type, $locale, $file_path, true, $translation_state);
      //This is a import file
      //try to rename file
      //$result = $api->renameFile($file_uri, $new_file_uri);
      //This is a rename file
      //try to delete file
      //$result = $api->deleteFile($new_file_uri);
      //This is delete file
    }
  }
}

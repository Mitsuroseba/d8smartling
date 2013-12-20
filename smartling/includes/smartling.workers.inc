<?php

/**
 * @file
 */

/**
 * Worker callback defined in hook_cron_queue_info()
 */
function smartling_queue_download_update_translated_item_process($entity_data) {
  $nodes_method = smartling_nodes_method($entity_data->bundle);

  $s_locale = $entity_data->target_language;
  $rid = $entity_data->rid;

  $file_name = $entity_data->file_name;
  $file_name_unic = $entity_data->file_name;

  $file_path = SMARTLING_DIRECTORY . '/' . $file_name;

  $file_name = substr($file_name, 0, strlen($file_name) - 4);

  $translated_filename = $file_name . '_' . $s_locale . '.xml';

  $key = variable_get('smartling_key');
  $project_id = variable_get('smartling_project_id');
  $mode = variable_get('smartling_mode');

  // Init api object
  $api = new SmartlingAPI($key, $project_id, $mode);

  $retrieval_type = variable_get('smartling_retrieval_type');
  $download_param = array(
    'retrievalType' => $retrieval_type,
  );

  smartling_watchdog('Smartling queue start download xml file and update fields for node id - @nid.', TRUE, array('@nid' => $rid), WATCHDOG_INFO, l(t('View file'), $file_path));

  // Try to download file
  $download_result = $api->downloadFile($file_name_unic, $s_locale, $download_param);
  if ($mode == 'PRODUCTION') {
    if (isset($download_result->response->code)) {
      $download_result = json_decode($download_result);
      smartling_watchdog('smartling_queue_download_update_translated_item_process try to download @filename file: response code - @code, message - @message', FALSE, array('@filename' => $file_name_unic, '@code' => $download_result->response->code, '@message' => $download_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
    }
    else {
      // This is a $download_result
      $xml = new DOMDocument();
      $xml->loadXML($download_result);

      $save = smartling_save_xml($xml, $rid, $s_locale, $translated_filename, TRUE);
      if (is_object($save)) {
        smartling_update_translated_fields($rid, $s_locale, $nodes_method);
      }
    }
  }
  elseif ($mode == 'SANDBOX') {
    // This is a $download_result. In SANDBOX mode $download_result == xml content
    $xml = new DOMDocument();
    $xml->loadXML($download_result);

    $save = smartling_save_xml($xml, $rid, $s_locale, $translated_filename, TRUE);
    if (is_object($save)) {
      smartling_update_translated_fields($rid, $s_locale, $nodes_method);
    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info()
 */
function smartling_queue_check_status_process($args) {
  if (smartling_is_configured()) {
    $rid = $args->rid;
    $s_locale = $args->s_locale;

    $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
    if ($entity_data !== FALSE) {
      if ($entity_data->progress != 100) {
        $file_name = $entity_data->file_name;
        $file_name_unic = $entity_data->file_name;

        $file_uri = file_create_url('public://smartling/' . $file_name);

        $key = variable_get('smartling_key');
        $project_id = variable_get('smartling_project_id');
        $mode = variable_get('smartling_mode');

        // Init api object
        $api = new SmartlingAPI($key, $project_id, $mode);

        // Try to retrieve file status
        $status_result = $api->getStatus($file_name_unic, $s_locale);
        $status_result = json_decode($status_result);

        // This is a get status
        if ($api->getCodeStatus() == 'SUCCESS') {
          if ($mode == 'SANDBOX') {
            // This code imitation translate file in service
            $finish_translate = rand(1, 2);
            if ($finish_translate == 1) {
              $status_result->response->data->completedStringCount = $status_result->response->data->approvedStringCount;
            }
          }

          smartling_watchdog('Smartling check status for node id - @nid (@s_locale). approvedString = @as, completedString = @cs', TRUE, array(
            '@nid' => $args->rid,
            '@s_locale' => $args->s_locale,
            '@as' => $status_result->response->data->approvedStringCount,
            '@cs' => $status_result->response->data->completedStringCount
              ), WATCHDOG_INFO, l(t('View file'), $file_uri));

          // If true, file translated
          if ($status_result->response->data->approvedStringCount == $status_result->response->data->completedStringCount) {
            $entity_data->progress = 100;
            $entity_data->download = 0;

            $smartling_queue = DrupalQueue::get('smartling_download_update_translated_item');
            $smartling_queue->createQueue();
            $smartling_queue->createItem($entity_data);

            smartling_entity_data_save($entity_data);
          }
          else {
            if ($status_result->response->data->approvedStringCount !== 0) {
              $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);

              $entity_data->progress = $progress;
              smartling_entity_data_save($entity_data);
            }
            else {
              $progress = 100;

              $entity_data->progress = $progress;
              smartling_entity_data_save($entity_data);
            }
          }
        }
        else {
          smartling_watchdog('smartling_queue_check_status_process try check status file: response code - @code, message - @message', FALSE, array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
        }
      }
      else {
        return;
      }
    }
    else {
      smartling_watchdog('Smartling check status for node id - @nid is FAIL! Smartling entity not exist.', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
    }
  }
}

/**
 * Worker callback defined in hook_cron_queue_info()
 */
function smartling_queue_send_to_translate_process($args) {
  if (smartling_is_configured()) {
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $args->rid, 'target_language' => $args->s_locale));

    if ($entity_data == FALSE) {
      $entity_data = smartling_entity_data_create();
      $entity_data->rid = $args->rid;
      $entity_data->bundle = $args->type;
    }

    $entity_data->original_language = $args->original_language;
    $entity_data->target_language = $args->s_locale;
    $entity_data->translated_file_name = FALSE;
    $entity_data->submitter = $args->submitter;
    $entity_data->progress = 0;
    $entity_data->submission_date = time();
    smartling_entity_data_save($entity_data);

    $nodes_method = smartling_nodes_method($args->type);

    $save = smartling_build_xml_by_node_id($args->rid, $args->s_locale, $args->title, $nodes_method);

    if (($save !== FALSE) && is_string($save)) {
      $file_name = $save;
      $file_name_unic = $save;
      $file_path = SMARTLING_DIRECTORY . '/' . $file_name;
      $file_uri = file_create_url('public://smartling/' . $file_name);
      $file_type = 'xml';

      $key = variable_get('smartling_key');
      $project_id = variable_get('smartling_project_id');
      $mode = variable_get('smartling_mode');

      // Init api object
      $api = new SmartlingAPI($key, $project_id, $mode);

      $upload_params = array(
        'approved' => TRUE,
      );

      // Try to upload file
      $api->uploadFile($file_path, $file_type, $file_name_unic, $upload_params);

      if ($api->getCodeStatus() == 'SUCCESS') {
        smartling_watchdog('Smartling upload xml file to service for node id - @nid.', TRUE, array('@nid' => $args->rid), WATCHDOG_INFO, l(t('View file'), $file_path));
      }
      elseif (is_object($upload_result)) {
        smartling_watchdog('smartling_queue_send_to_translate_process try upload xml (fileUri - @filename) to service: response code - @code, message - @message', FALSE, array('@filename' => $file_name_unic, '@code' => $upload_result->response->code, '@message' => $upload_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));

        // Add new queue item if some error
        $smartling_queue = DrupalQueue::get('smartling_send_to_translate');
        $smartling_queue->createQueue();
        $smartling_queue->createItem($args);
        smartling_watchdog('Smartling upload xml file to service for node id - @nid is FAIL!', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $args->rid));
      }
    }
  }
}

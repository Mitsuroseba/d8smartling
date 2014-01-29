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

  // Init api object
  $api = new SmartlingAPI($key, $project_id, SMARTLING_PRODUCTION_MODE);

  $download_param = array(
    'retrievalType' => 'published',
  );

  smartling_watchdog('Smartling queue start download xml file and update fields for node id - @nid.', TRUE, array('@nid' => $rid), WATCHDOG_INFO, l('View file', $file_path));

  // Try to download file
  $download_result = $api->downloadFile($file_name_unic, $s_locale, $download_param);

  if (isset($download_result->response->code)) {
    $download_result = json_decode($download_result);
    smartling_watchdog('smartling_queue_download_update_translated_item_process try to download file:<br/>
      Project Id: @project_id <br/>
      Action: download <br/>
      URI: @file_uri <br/>
      Locale: @s_locale <br/>
      Error: response code -> @code and message -> @message', FALSE, array('@project_id' => $project_id, '@file_uri' => $file_name_unic, '@s_locale' => $s_locale, '@code' => $download_result->response->code, '@message' => $download_result->response->messages[0]), WATCHDOG_ERROR, l('Related entity', 'node/' . $args->rid));
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

        // Init api object
        $api = new SmartlingAPI($key, $project_id, SMARTLING_PRODUCTION_MODE);

        // Try to retrieve file status
        $status_result = $api->getStatus($file_name_unic, $s_locale);
        $status_result = json_decode($status_result);

        // This is a get status
        if ($api->getCodeStatus() == 'SUCCESS') {

          smartling_watchdog('Smartling check status for node id - @nid (@s_locale). approvedString = @as, completedString = @cs', TRUE, array(
            '@nid' => $args->rid,
            '@s_locale' => $args->s_locale,
            '@as' => $status_result->response->data->approvedStringCount,
            '@cs' => $status_result->response->data->completedStringCount
              ), WATCHDOG_INFO, l('View file', $file_uri));

          // If true, file translated
          if ($status_result->response->data->approvedStringCount == $status_result->response->data->completedStringCount) {
            $entity_data->progress = 100;
            $entity_data->download = 0;
            $entity_data->status = SMARTLING_STATUS_IN_TRANSLATE;

            $smartling_queue = DrupalQueue::get('smartling_download');
            $smartling_queue->createQueue();
            $smartling_queue->createItem($entity_data);

            smartling_entity_data_save($entity_data);
          }
          else {
            if ($status_result->response->data->approvedStringCount !== 0) {
              $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);

              $entity_data->progress = $progress;
              $entity_data->status = SMARTLING_STATUS_IN_TRANSLATE;
              smartling_entity_data_save($entity_data);
            }
            else {
              $progress = 100;

              $entity_data->progress = $progress;
              $entity_data->status = SMARTLING_STATUS_IN_TRANSLATE;
              smartling_entity_data_save($entity_data);
            }
          }
        }
        else {
          smartling_watchdog('Smartling check status for node id - @nid: <br/>
      Project Id: @project_id <br/>
      Action: status <br/>
      URI: @file_uri <br/>
      Locale: @s_locale <br/>
      Error: response code -> @code and message -> @message', FALSE, array('@nid' => $args->rid, '@project_id' => $project_id, '@file_uri' => $file_name_unic, '@s_locale' => $s_locale, '@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, l('Related entity', 'node/' . $args->rid));
        }
      }
      else {
        return;
      }
    }
    else {
      smartling_watchdog('Smartling check status for node id - @nid is FAIL! Smartling entity not exist.', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l('Related entity', 'node/' . $args->rid));
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
      $entity_data->submission_date = time();
    }

    $entity_data->original_language = $args->original_language;
    $entity_data->target_language = $args->s_locale;
    $entity_data->translated_file_name = FALSE;
    $entity_data->submitter = $args->submitter;
    $entity_data->progress = 0;
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

      // Init api object
      $api = new SmartlingAPI($key, $project_id, SMARTLING_PRODUCTION_MODE);

      $upload_params = array(
        'approved' => TRUE,
      );

      if (SMARTLING_CALLBACK_URL) {
        global $base_url;
        $upload_params['callbackUrl'] = $base_url . '/' . SMARTLING_CALLBACK_URL;
        smartling_watchdog('Parameter callbackUrl - @scu.', TRUE, array('@scu' => $upload_params['callbackUrl']), WATCHDOG_INFO);
      }

      // Try to upload file
      $upload_result = $api->uploadFile($file_path, $file_type, $file_name_unic, $upload_params);
      $upload_result = json_decode($upload_result);

      if ($api->getCodeStatus() == 'SUCCESS') {
        smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE);
        smartling_watchdog('Smartling upload xml file to service for node id - @nid.', TRUE, array('@nid' => $args->rid), WATCHDOG_INFO, l('View file', $file_path));
        if (module_exists('rules')) {
          $node_event = smartling_node_load_default($args->rid);
          rules_invoke_event('smartling_uploading_original_to_smartling_event', $node_event);
        }
      }
      elseif (is_object($upload_result)) {
        smartling_watchdog('smartling_queue_send_to_translate_process try upload xml to service: <br/>
      Project Id: @project_id <br/>
      Action: upload <br/>
      URI: @file_uri <br/>
      Error: response code -> @code and message -> @message', FALSE, array('@project_id' => $project_id, '@file_uri' => $file_name_unic, '@code' => $upload_result->response->code, '@message' => $upload_result->response->messages[0]), WATCHDOG_ERROR, l('Related entity', 'node/' . $args->rid));

        // Add new queue item if some error
        $smartling_queue = DrupalQueue::get('smartling_upload');
        $smartling_queue->createQueue();
        $smartling_queue->createItem($args);
        smartling_watchdog('Smartling upload xml file to service for node id - @nid is FAIL!', FALSE, array('@nid' => $args->rid), WATCHDOG_ERROR, l('Related entity', 'node/' . $args->rid));
      }
    }
  }
}

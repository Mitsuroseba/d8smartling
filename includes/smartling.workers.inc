<?php

/**
 * @file
 * Workers functions for Smartling module.
 */

/**
 * Worker callback defined in hook_cron_queue_info().
 * @todo: rename the function. "download-update" (we download or update?)
 */
function smartling_queue_download_update_translated_item_process($eid) {
  $status = FALSE;
  try {
    $status = drupal_container()->get('smartling.queue_managers.download')->execute($eid);
  }
  catch (\Drupal\smartling\SmartlingExceptions\SmartlingGenericException $e) {
    smartling_log_get_handler()->critical($e->getMessage() . '   ' . print_r($e, TRUE));
  }

  return $status;
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_check_status_process($eid) {
  $status = FALSE;
  try {
    $status = drupal_container()->get('smartling.queue_managers.check_status')->execute($eid);
  }
  catch (\Drupal\smartling\SmartlingExceptions\SmartlingGenericException $e) {
    smartling_log_get_handler()->critical($e->getMessage() . '   ' . print_r($e, TRUE));
  }
  return $status;
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_send_to_translate_process($eids) {
  $status = FALSE;
  try {
    $status = drupal_container()->get('smartling.queue_managers.upload')->execute($eids);
  }
  catch (\Drupal\smartling\SmartlingExceptions\SmartlingGenericException $e) {
    smartling_log_get_handler()->critical($e->getMessage() . '   ' . print_r($e, TRUE));
  }
  return $status;
}

<?php

/**
 * @file
 * Workers functions for Smartling module.
 */

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_download_update_translated_item_process($eid) {
  $status = drupal_container()->get('smartling.queue_managers.check_status')->execute($eid);

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
  return drupal_container()->get('smartling.queue_managers.check_status')->execute($eid);
}

/**
 * Worker callback defined in hook_cron_queue_info().
 */
function smartling_queue_send_to_translate_process($eids) {
  return drupal_container()->get('smartling.queue_managers.upload')->execute($eids);
}

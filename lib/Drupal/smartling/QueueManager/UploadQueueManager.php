<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class UploadQueueManager implements QueueManagerInterface {
  /**
   * @inheritdoc
   */
  public function add($eids) {
    if (empty($eids)) {
      return;
    }
    $smartling_queue = \DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();
    $smartling_queue->createItem($eids);
  }

  protected function getOriginalEntity($entity_type, $entity) {
    switch ($entity_type) {
      case 'node':
        $entity = smartling_get_original_node($entity);
        break;

      case 'taxonomy_term':
        $entity = smartling_get_original_taxonomy_term($entity);
        break;
    }
    return $entity;
  }

  public function addRawEntity($entity_type, $entity, $languages) {
    $log = smartling_log_get_handler();

    $entity = $this->getOriginalEntity($entity_type, $entity);

    if (empty($entity)) {
      return;
    }

    $wrapper = entity_metadata_wrapper($entity_type, $entity);
    $id      = $wrapper->getIdentifier();
    $bundle  = $wrapper->getBundle();
    $title   = $wrapper->label();
    $link    = smartling_get_link_to_entity($entity_type, $entity);

    if (!smartling_translate_fields_configured($bundle, $entity_type)) {
      drupal_set_message(t("Type '@type' is not supported or it's not configured in Smartling.", array('@type' => $bundle)), 'warning');
      $log->setMessage("Type '@type' is not supported or it's not configured in Smartling.")
        ->setVariables(array('@type' => $bundle))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link)
        ->execute();

      return;
    }

    // $d_locale_original = language_default()->language;
    // $d_locale_original = $entity->translations->original;
    $d_locale_original = entity_language($entity_type, $entity);
    $queued_eids = array();
    $langs = array();
    foreach ($languages as $target_language) {
      if ($target_language == $d_locale_original) {
        continue;
      }

      $queued_eids[] = drupal_container()->get('smartling.wrappers.entity_data_wrapper')
        ->loadSingleByConditions(array('rid' => $id, 'entity_type' => $entity_type, 'target_language' => $target_language))
        ->orCreateFromDrupalEntity($entity, $entity_type, $d_locale_original, $target_language)
        ->setStatusByEvent(SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE)
        ->setSubmitter()
        ->setSubmissionDate(REQUEST_TIME)
        ->save()
        ->getEID();

      $langs[] = $target_language;
    }

    $this->add($queued_eids);
    // Create content hash (Fake entity update).
    smartling_entity_update($entity, $entity_type);

    $langs = implode(', ', $langs);
    $log->setMessage('Add smartling queue task for entity id - @id, locale - @locale, type - @entity_type')
      ->setVariables(array(
        '@id' => $id,
        '@locale' => $langs,
        '@entity_type' => $entity_type,
      ))
      ->setLink($link)
      ->execute();

    drupal_set_message(t('The @entity_type "@title" has been scheduled to be sent to Smartling for translation to "@langs".', array(
      '@entity_type' => $entity_type,
      '@title' => $title,
      '@langs' => $langs,
    )));
  }

  /**
   * @inheritdoc
   */
  public function execute($eids) {
    if (!smartling_is_configured()) {
      return;
    }
    if (!is_array($eids)) {
      $eids = array($eids);
    }

    $smartling_entity  = NULL;
    $target_locales    = array();
    $entity_data_array = array();

    $entity_data_wrapper = drupal_container()->get('smartling.wrappers.entity_data_wrapper');
    foreach($eids as $eid) {
      $entity_data_wrapper->loadByID($eid);
      $file_name = $entity_data_wrapper->getFileName();
      $target_locales[$file_name][] = $entity_data_wrapper->getTargetLanguage();
      $entity_data_array[$file_name][] = $entity_data_wrapper->getEntity();
    }


    $api = drupal_container()->get('smartling.api_wrapper');
    foreach ($entity_data_array as $file_name => $entity_array) {
      $entity = reset($entity_array);
      $processor = smartling_get_entity_processor($entity);
      $xml = smartling_build_xml($processor, $$entity->rid);
      if (!($xml instanceof \DOMNode)) {
        continue;
      }

      $event   = SMARTLING_STATUS_EVENT_FAILED_UPLOAD;
      $success = (bool) smartling_save_xml($file_name, $xml, $entity);
      // Init api object.
      if ($success) {
        $file_path = drupal_realpath(smartling_get_dir($file_name), TRUE);
        $event = $api->uploadFile($file_path, $file_name, 'xml', $target_locales[$file_name]);
      }

      foreach ($entity_array as $entity) {
        $entity_data_wrapper->setEntity($entity)->setStatusByEvent($event)->save();

        //@todo: refactor this code to be compatible with any entity_type
        if (($event == SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE) && module_exists('rules') && ($entity_data_wrapper->getEntityType() == 'node')) {
            $node_event = node_load($entity_data_wrapper->getRID());
            rules_invoke_event('smartling_uploading_original_to_smartling_event', $node_event);
        }
      }
    }
  }
}

<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class DownloadQueueManager implements QueueManagerInterface {

  protected $entity_data_wrapper;
  protected $field_api_wrapper;
  protected $entity_processor_factory;
  protected $settings;
  protected $smartling_utils;
  protected $drupal_wrapper;


  public function __construct($entity_data_wrapper, $field_api_wrapper, $entity_processor_factory, $settings, $smartling_utils, $drupal_wrapper) {
    $this->entity_data_wrapper = $entity_data_wrapper;
    $this->field_api_wrapper = $field_api_wrapper;
    $this->entity_processor_factory = $entity_processor_factory;
    $this->settings = $settings;
    $this->smartling_utils = $smartling_utils;
    $this->drupal_wrapper = $drupal_wrapper;
  }

  /**
   * @inheritdoc
   */
  public function add($eids) {
    if (empty($eids)) {
      return;
    }
    $smartling_queue = \DrupalQueue::get('smartling_download');
    $smartling_queue->createQueue();
    $smartling_queue->createItem($eids);
  }

  /**
   * @inheritdoc
   */
  public function execute($eids) {
    if ($this->drupal_wrapper->getDefaultLanguage() != $this->field_api_wrapper->fieldValidLanguage(NULL, FALSE)) {
      drupal_set_message('The download failed. Please switch to the site\'s default language: ' . $this->drupal_wrapper->getDefaultLanguage(), 'error');
      return FALSE;
    }

    if (!is_array($eids)) {
      $eids = array($eids);
    }

    $global_status = TRUE;
    foreach ($eids as $eid) {
      $status = FALSE;

      $smartling_submission = $this->entity_data_wrapper->loadByID($eid)->getEntity();
      if ($smartling_submission && $this->smartling_utils->isConfigured() && !empty($this->settings->getFieldsSettingsByBundle($smartling_submission->bundle, $smartling_submission->entity_type))) {
        $processor = $this->$entity_processor_factory->getProcessor($smartling_submission);
        if ($processor->downloadTranslation()) {
          $status = $processor->updateEntityFromXML();
        }

      }
      $global_status = $global_status & $status;
    }

    return $global_status;
  }
}

<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class UploadQueueManager implements QueueManagerInterface {

  protected $api_wrapper;
  protected $smartling_utils;
  protected $entity_processor_factory;
  protected $smartling_submission_wrapper;
  protected $drupal_wrapper;
  protected $settings;

  public function __construct($api_wrapper, $smartling_submission_wrapper, $entity_processor_factory, $settings, $smartling_utils, $drupal_wrapper) {
    $this->api_wrapper = $api_wrapper;
    $this->smartling_submission_wrapper = $smartling_submission_wrapper;
    $this->entity_processor_factory = $entity_processor_factory;
    $this->smartling_utils = $smartling_utils;
    $this->drupal_wrapper = $drupal_wrapper;
    $this->settings = $settings;
  }

  /**
   * @inheritdoc
   */
  public function add($eids) {
    if (empty($eids)) {
      return FALSE;
    }
    $smartling_queue = \DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();
    return $smartling_queue->createItem($eids);
  }

  /**
   * @inheritdoc
   */
  public function execute($eids) {
    if (!$this->smartling_utils->isConfigured()) {
      throw new \Drupal\smartling\SmartlingExceptions\SmartlingNotConfigured(t('Smartling module is not configured. Please follow the page <a href="@link">"Smartling settings"</a> to setup Smartling configuration.', array('@link' => url('admin/config/regional/smartling'))));
    }

    if (!is_array($eids)) {
      $eids = array($eids);
    }
    $eids = array_unique($eids);

    $smartling_entity  = NULL;
    $target_locales    = array();
    $entity_data_array = array();

    foreach($eids as $eid) {
      $this->smartling_submission_wrapper->loadByID($eid);
      $file_name = $this->smartling_submission_wrapper->getFileName();
      $target_locales[$file_name][] = $this->smartling_submission_wrapper->getTargetLanguage();
      $entity_data_array[$file_name][] = $this->smartling_submission_wrapper->getEntity();
    }


    foreach ($entity_data_array as $file_name => $entity_array) {
      $submission = reset($entity_array);
      $processor = $this->entity_processor_factory->getProcessor($submission);
      $xml = $this->exportContentForTranslation();
      if (!($xml instanceof \DOMNode)) {
        continue;
      }

      $event   = SMARTLING_STATUS_EVENT_FAILED_UPLOAD;
      $success = (bool) $this->smartling_utils->saveXml($file_name, $xml, $submission);
      // Init api object.
      if ($success) {
        $file_path = $this->drupal_wrapper->drupalRealpath($this->settings->getDir($file_name), TRUE);
        $event = $this->api_wrapper->uploadFile($file_path, $file_name, 'xml', $target_locales[$file_name]);
      }

      foreach ($entity_array as $submission) {
        $this->smartling_submission_wrapper->setEntity($submission)->setStatusByEvent($event)->save();
      }
    }
  }
}

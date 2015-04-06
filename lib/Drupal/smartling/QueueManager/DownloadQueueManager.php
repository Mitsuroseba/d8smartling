<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class DownloadQueueManager implements QueueManagerInterface {

  protected $smartling_submission_wrapper;
  protected $field_api_wrapper;
  protected $entity_processor_factory;
  protected $settings;
  protected $smartling_utils;
  protected $drupal_wrapper;


  public function __construct($smartling_submission_wrapper, $field_api_wrapper, $entity_processor_factory, $settings, $smartling_utils, $drupal_wrapper) {
    $this->smartling_submission_wrapper = $smartling_submission_wrapper;
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
      throw new WrongSiteSettingsException('The download failed. Please switch to the site\'s default language: ' . $this->drupal_wrapper->getDefaultLanguage());
    }

    if (!$this->smartling_utils->isConfigured()) {
      throw new SmartlingNotConfigured(t('Smartling module is not configured. Please follow the page <a href="@link">"Smartling settings"</a> to setup Smartling configuration.', array('@link' => url('admin/config/regional/smartling'))));
    }

    if (!is_array($eids)) {
      $eids = array($eids);
    }

    $global_status = TRUE;
    foreach ($eids as $eid) {
      $status = FALSE;

      $translatable_fields = $this->settings->getFieldsSettingsByBundle($smartling_submission->entity_type, $smartling_submission->bundle);
      $smartling_submission = $this->smartling_submission_wrapper->loadByID($eid)->getEntity();
      if ($smartling_submission && !empty($translatable_fields)) {
        $processor = $this->entity_processor_factory->getProcessor($smartling_submission);
        if ($processor->downloadTranslation()) {
          $status = $processor->updateEntityFromXML();
        }

      }
      $global_status = $global_status & $status;
    }

    return $global_status;
  }
}

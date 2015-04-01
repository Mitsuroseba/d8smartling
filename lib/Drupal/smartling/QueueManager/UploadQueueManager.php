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
  protected $entity_data_wrapper;
  protected $drupal_wrapper;
  protected $settings;

  public function __construct($api_wrapper, $entity_data_wrapper, $entity_processor_factory, $settings, $smartling_utils, $drupal_wrapper) {
    $this->api_wrapper = $api_wrapper;
    $this->entity_data_wrapper = $entity_data_wrapper;
    $this->entity_processor_factory = $entity_processor_factory;
    $this->smartling_utils = $smartling_utils;
    $this->drupal_wrapper = $drupal_wrapper;
    $this->settings = $settings;
  }

  /**
   * Build xml document and save in file.
   *
   * @param object $processor
   *   Drupal entity processor
   * @param int $rid
   *
   * @return DOMDocument
   *   Returns XML object.
   */
  protected function buildXml($processor, $rid) {
    $xml = new \DOMDocument('1.0', 'UTF-8');

    $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string, data/localize/field_collection/string, data/localize/field_collection/field_collection/string, data/localize/field_collection/field_collection/field_collection/string, data/localize/field_collection/field_collection/field_collection/field_collection/string '));
    // @todo remove hardcoded mappping of nested field collections.
    $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string, html : data/localize/field_collection/string, html : data/localize/field_collection/field_collection/string, html : data/localize/field_collection/field_collection/field_collection/string '));
    $xml->appendChild($xml->createComment(' smartling.placeholder_format_custom = (@|%|!)[\w-]+ '));

    $data = $xml->createElement('data');

    $localize = $processor->exportContentToTranslation($xml, $rid);

    $data->appendChild($localize);
    $xml->appendChild($data);

    // @todo Verify how many child has $data. If zero, then write to log and stop upload
    // This logic was lost in OOP branch
    //  {
    //    smartling_entity_delete_all_by_conditions(array(
    //      'rid' => $rid,
    //      'entity_type' => $entity_type,
    //    ));
    //    $log->setMessage('Entity has no strings to translate for entity_type - @entity_type, id - @rid.')
    //      ->setVariables(array('@entity_type' => $entity_type, '@rid' => $rid))
    //      ->setSeverity(WATCHDOG_WARNING)
    //      ->execute();
    //    $file_name = FALSE;
    //  }

    return $xml;
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
      $this->entity_data_wrapper->loadByID($eid);
      $file_name = $this->entity_data_wrapper->getFileName();
      $target_locales[$file_name][] = $this->entity_data_wrapper->getTargetLanguage();
      $entity_data_array[$file_name][] = $this->entity_data_wrapper->getEntity();
    }


    foreach ($entity_data_array as $file_name => $entity_array) {
      $submission = reset($entity_array);
      $processor = $this->entity_processor_factory->getProcessor($submission);
      $xml = $this->buildXml($processor, $submission->rid);
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
        $this->entity_data_wrapper->setEntity($submission)->setStatusByEvent($event)->save();
      }
    }
  }
}

<?php

/**
 * @file
 * Contains Drupal\smartling\Processors\GenericEntityProcessor.
 *
 * @todo rename namespace to EntityProcessor or something else.
 */

namespace Drupal\smartling\Processors;

use DOMXPath;
use Drupal\smartling\ApiWrapper\SmartlingApiWrapper;
use Drupal\smartling\FieldProcessorFactory;
use Drupal\smartling\Log\SmartlingLog;

/**
 * Contains smartling entity and provide main Smartling connector business logic.
 *
 * @package Drupal\smartling\Processors
 */
class GenericEntityProcessor {

  /**
   * Contains Smartling data entity.
   *
   * Instance of SmartlingEntityData if processor was created from newly created
   * smartling data entity that has such type in other cases - just stdClass.
   *
   * @var \stdClass|\SmartlingEntityData
   */
  public $smartling_submission;

  /**
   * Contains Smartling data referenced drupal content entity, e.g. Node, User.
   *
   * @var \stdClass|\Entity
   *
   * @see smartling_entity_load().
   */
  public $contentEntity;

  /**
   * Abstracted wrapper for drupal content entity.
   *
   * @var \EntityDrupalWrapper
   */
  public $contentEntityWrapper;

  /**
   * Contain drupal content entity type.
   *
   * @var string
   */
  protected $drupalEntityType;

  /**
   * Contains Smartling log object.
   *
   * @var \Drupal\smartling\Log\SmartlingLog
   */
  protected $log;

  /**
   * @var string
   * @todo Rename suffix Language to Locale to make consistent with other properties
   */
  protected $targetFieldLanguage;

  /**
   * @var string
   */
  protected $drupalOriginalLocale;

  /**
   * Contains locale in drupal format, e.g. 'en', 'und'.
   *
   * @var string
   */
  protected $drupalTargetLocale;

  /**
   * Contains if drupal content bundle has "Entity translation" mode.
   *
   * @var bool
   */
  protected $ifFieldMethod;
  // @todo choose better name.
  /**
   * @var SmartlingApiWrapper
   */
  protected $smartlingAPI;

  /**
   * @var FieldProcessorFactory
   */
  protected $fieldProcessorFactory;

  /**
   * Helper internal flag to avoid duplicated execution.
   *
   * @var bool
   *
   * @see self::prepareOriginalEntity()
   */
  protected $isOriginalEntityPrepared;

  protected $entity_api_wrapper;
  protected $smartling_utils;
  protected $smartling_settings;



  /**
   * Translation handler factory.
   *
   * @param string $entity_type
   *   Entity type.
   * @param object $entity
   *   Entity.
   *
   * @return object
   *   Return translation handler object.
   */
  protected function getEntityTranslationHandler($entity_type, $entity) {
    $entity_info = entity_get_info($entity_type);
    $class = 'SmartlingEntityTranslationDefaultHandler';
    // @todo remove fourth parameter once 3rd-party translation handlers have
    // been fixed and no longer require the deprecated entity_id parameter.
    $handler = new $class($entity_type, $entity_info, $entity, NULL);
    return $handler;
  }

  /**
   * Create GenericEntityProcessor instance.
   *
   * @param object $smartling_submission
   *   Smartling data entity.
   * @param FieldProcessorFactory $field_processor_factory
   *   Factory instance for all field specific logic.
   * @param SmartlingApiWrapper $smartling_api
   *   Smartling API wrapper for Drupal.
   * @param SmartlingLog $log
   *   Smartling log object.
   *
   * @todo avoid procedural code in construct to achieve full DI.
   */
  public function __construct($smartling_submission, $field_processor_factory, $smartling_api, $smartling_settings, $log, $entity_api_wrapper, $smartling_utils) {
    $this->smartling_submission = $smartling_submission;
    $this->drupalTargetLocale = $smartling_submission->target_language;
    $this->drupalOriginalLocale = $smartling_submission->original_language;

    $this->log = $log;
    $this->fieldProcessorFactory = $field_processor_factory;
    $this->smartlingAPI = $smartling_api;
    $this->entity_api_wrapper = $entity_api_wrapper;
    $this->smartling_utils = $smartling_utils;
    $this->smartling_settings = $smartling_settings;


    $this->contentEntity = $this->entity_api_wrapper->entityLoadSingle($this->smartling_submission->entity_type, $this->smartling_submission->rid);
    $this->drupalEntityType = $this->smartling_submission->entity_type;
    $this->contentEntityWrapper = $this->entity_api_wrapper->entityMetadataWrapper($this->drupalEntityType, $this->contentEntity);
    $this->ifFieldMethod = $this->smartling_utils->isFieldsMethod($this->contentEntityWrapper->getBundle());

    $this->targetFieldLanguage = $this->ifFieldMethod ? $this->drupalTargetLocale : LANGUAGE_NONE;

  }

  /**
   * Fetch translation status from Smartling server.
   *
   * @return bool
   */
  public function getProgressStatus() {
    if (!empty($this->smartling_submission->file_name)) {
      $result = $this->smartlingAPI->getStatus($this->smartling_submission);

      if (!empty($result)) {
        return $result['entity_data']->progress;
      }
      else {
        return FALSE;
      }
    }
    else {
      return FALSE;
    }
  }

  /**
   * Wrapper for drupal entity saving.
   *
   * @todo move this logic to original entity Proxy object.
   */
  public function saveDrupalEntity() {
    $this->contentEntityWrapper->set($this->contentEntity);
    $this->contentEntityWrapper->save();
  }

  /**
   * Get link to drupal content.
   *
   * @todo move this logic to original entity Proxy object.
   */
  public function linkToContent($link_title = '') {
    $link_title = (!empty($link_title)) ? $link_title : $this->contentEntityWrapper->label();
    $link_title = (!empty($link_title)) ? $link_title : t('Related entity');

    $uri        = entity_uri($this->drupalEntityType, $this->contentEntity);
    $path       = $uri['path'];

    return l($link_title, $path);
  }

  /**
   * Downloads translation data from Smartling server.
   */
  public function downloadTranslation() {
    $progress = $this->getProgressStatus();
    if ($progress === FALSE) {
      return;
    }

    $download_result = $this->smartlingAPI->downloadFile($this->smartling_submission);

    libxml_use_internal_errors(true);
    if (FALSE === simplexml_load_string($download_result)) {
      return;
    }
    // This is a download result.
    $xml = new \DOMDocument();
    $xml->loadXML($download_result);

    $translated_file_name = drupal_container()->get('smartling.wrappers.smartling_submission_wrapper')->setEntity($this->smartling_submission)->getFileTranslatedName();
//    $file_name = substr($this->entity->file_name, 0, strlen($this->entity->file_name) - 4);
//    $translated_file_name = $file_name . '_' . $this->entity->target_language . '.xml';

    // Save result.
    $isSuccess = $this->smartling_utils->saveXML($translated_file_name, $xml, $this->smartling_submission);

    // If result is saved.
    // @todo finish converting.
    if ($isSuccess) {
      drupal_container()->get('smartling.wrappers.smartling_submission_wrapper')
        ->setEntity($this->smartling_submission)
        ->setStatusByEvent(SMARTLING_STATUS_EVENT_UPDATE_FIELDS)
        ->setProgress($progress)
        ->save();

      $isSuccess = $this->updateDrupalTranslation();
    }

    return $isSuccess;
  }

  /**
   * Contains preparation for entity before smartling processing.
   *
   * Should be overridden for node and term. E.g. before pushing translation we have to fetch data
   * from original node, so swap current node to original translation if necessary.
   * @todo move this logic to original entity Proxy object.
   */
  public function prepareDrupalEntity() {
    if (!$this->isOriginalEntityPrepared) {
      $this->isOriginalEntityPrepared = TRUE;

      if ($this->ifFieldMethod) {
        foreach ($this->getTranslatableFields() as $field_name) {
          // Still use entity object itself because entity wrapper hardcodes
          // language and disallow to fetch values from translated fields.
          // However all entities work with entities in the same way.
          if (!empty($this->contentEntity->{$field_name}[$this->drupalOriginalLocale]) && empty($this->contentEntity->{$field_name}[$this->drupalTargetLocale])) {
            $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->drupalEntityType, $this->smartling_submission);
            $this->contentEntity->{$field_name}[$this->drupalTargetLocale] = $fieldProcessor->prepareBeforeDownload($this->contentEntity->{$field_name}[$this->drupalOriginalLocale]);
          }
        }
      }
    }
  }

  /**
   * Implements entity_translation logic to update translation data in Drupal.
   */
  public function updateDrupalTranslation() {
    $smartling_submission = $this->entity_api_wrapper->entityLoadSingle($this->drupalEntityType, $this->smartling_submission->rid);
    $handler = $this->getEntityTranslationHandler($this->drupalEntityType, $smartling_submission);
    $translations = $handler->getTranslations();

    // Initialize translations if they are empty.
    if (empty($translations->original)) {
      $handler->initTranslations();
      $handler->saveTranslations();
      // Update the wrapped entity.
      $handler->setEntity($smartling_submission);
      $handler->smartlingEntityTranslationFieldAttach();
      $translations = $handler->getTranslations();
    }

    $entity_translation = array(
      'entity_type' => $this->drupalEntityType,
      'entity_id' => $this->smartling_submission->rid,
      'translate' => '0',
      'status' => !empty($smartling_submission->status) ? $smartling_submission->status : 1,
      'language' => $this->drupalTargetLocale,
      'uid' => $this->smartling_submission->submitter,
      'changed' => REQUEST_TIME,
    );

    if (isset($translations->data[$this->drupalTargetLocale])) {
      $handler->setTranslation($entity_translation);
    }
    else {
      // Add the new translation.
      $entity_translation += array(
        'source' => $translations->original,
        'created' => !empty($smartling_submission->created) ? $smartling_submission->created : REQUEST_TIME,
      );
      $handler->setTranslation($entity_translation);
    }
    $handler->saveTranslations();
    // Update the wrapped entity.
    $handler->setEntity($smartling_submission);
    $handler->smartlingEntityTranslationFieldAttach();

    return TRUE;
  }

  /**
   * Updates smartling data entity from given xml parsed object.
   *
   * @param $xml \DOMDocument
   */
  public function importSmartlingXMLToSmartlingEntity(\DOMDocument $xml) {
    $this->prepareDrupalEntity();

    $xpath = new DomXpath($xml);

    foreach ($this->getTranslatableFields() as $field_name) {
      // @TODO test if format could be set automatically.
      $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->smartling_submission->entity_type, $this->smartling_submission);
      $fieldValue = $fieldProcessor->fetchDataFromXML($xpath);
      $fieldProcessor->setDrupalContentFromXML($fieldValue);
    }

    $this->saveDrupalEntity();
  }

  protected function getFilePath($file_name) {
    return drupal_realpath($this->smartling_utils->cleanFileName($this->smartling_settings->getDir($file_name), TRUE));
  }

  /**
   * Process given xml parsed object using translated_file.
   */
  public function updateEntityFromXML() {
    $file_path = $this->getFilePath($this->smartling_submission->translated_file_name);

    $xml = new \DOMDocument();
    $xml->load($file_path);

    // Update smartling entity.
    $this->importSmartlingXMLToSmartlingEntity($xml);

    // Update translations information.
    return $this->updateDrupalTranslation();
  }

  public function getTranslatableContent() {
    $data = array();
    foreach ($this->getTranslatableFields() as $field_name) {
      $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->smartling_submission->entity_type, $this->smartling_submission);
      if ($fieldProcessor) {
        $data[$field_name] = $fieldProcessor->getSmartlingContent();
      }
    }

    return $data;
  }

  public function exportContentToTranslation($xml, $rid) {
    $localize = $xml->createElement('localize');
    $localize_attr = $xml->createAttribute('title');
    $localize_attr->value = $rid;
    $localize->appendChild($localize_attr);

    foreach ($this->getTranslatableFields() as $field_name) {
      /* @var $fieldProcessor \Drupal\smartling\FieldProcessors\BaseFieldProcessor */
      $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->smartling_submission->entity_type, $this->smartling_submission);
      if ($fieldProcessor) {
        $data = $fieldProcessor->getSmartlingContent();
        $fieldProcessor->putDataToXML($xml, $localize, $data);
      }
    }

    return $localize;
  }

  /**
   * Wrapper for Smartling settings storage.
   *
   * @return array()
   */
  public function getTranslatableFields() {
    return $this->smartling_settings->getFieldsSettingsByBundle($this->smartling_submission->entity_type, $this->smartling_submission->bundle);
  }
}
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
  public $entity;

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
   * Contains drupal content entity id.
   *
   * @var string|int
   */
  protected $relatedId;

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
   * Contains locale in drupal format, e.g. 'en-US', 'ru-RU'.
   *
   * @var string
   */
  protected $smartlingTargetLocale;

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

  /**
   * Create GenericEntityProcessor instance.
   *
   * @param object $entity
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
  public function __construct($entity, $field_processor_factory, $smartling_api, $log) {
    $this->entity = $entity;
    $this->drupalTargetLocale = $entity->target_language;
    $this->drupalOriginalLocale = $entity->original_language;
    $this->smartlingTargetLocale = smartling_convert_locale_drupal_to_smartling($entity->target_language);
    $this->relatedId = $entity->rid;

    $this->contentEntity = entity_load_single($this->entity->entity_type, $this->entity->rid);
    $this->drupalEntityType = $this->entity->entity_type;
    $this->contentEntityWrapper = entity_metadata_wrapper($this->drupalEntityType, $this->contentEntity);
    $this->ifFieldMethod = smartling_fields_method($this->contentEntityWrapper->getBundle());

    $this->targetFieldLanguage = $this->ifFieldMethod ? $this->drupalTargetLocale : LANGUAGE_NONE;

    $this->log = $log;
    $this->fieldProcessorFactory = $field_processor_factory;
    $this->smartlingAPI = $smartling_api;
  }

  /**
   * Fetch translation status from Smartling server.
   *
   * @return bool
   */
  public function getProgressStatus() {
    if (!empty($this->entity->file_name)) {
      $result = $this->smartlingAPI->getStatus($this->entity);

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
   * Set translation status|progress to smartling data entity.
   *
   * @param $status
   */
  public function setProgressStatus($status) {
    switch ($status) {
      case SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE:
        if (empty($this->entity->status) || ($this->entity->status == SMARTLING_STATUS_CHANGE)) {
          $this->entity->status = SMARTLING_STATUS_IN_QUEUE;
          $this->saveSmartlingEntity();
        }
        break;

      case SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE:
        if ($this->entity->status != SMARTLING_STATUS_CHANGE) {
          $this->entity->status = SMARTLING_STATUS_IN_TRANSLATE;
          $this->saveSmartlingEntity();
        }
        break;

      case SMARTLING_STATUS_EVENT_DOWNLOAD_FROM_SERVICE:
      case SMARTLING_STATUS_EVENT_UPDATE_FIELDS:
        if ($this->entity->status != SMARTLING_STATUS_CHANGE) {
          if ($this->entity->progress == 100) {
            $this->entity->status = SMARTLING_STATUS_TRANSLATED;
          }
          $this->saveSmartlingEntity();
        }
        break;

      case SMARTLING_STATUS_EVENT_NODE_ENTITY_UPDATE:
        $this->entity->status = SMARTLING_STATUS_CHANGE;
        $this->saveSmartlingEntity();
        break;

      case SMARTLING_STATUS_EVENT_FAILED_UPLOAD:
        $this->entity->status = SMARTLING_STATUS_FAILED;
        $this->saveSmartlingEntity();
        break;

      default:
        break;
    }
  }

  /**
   * Wrapper for smartling data entity saving.
   */
  public function saveSmartlingEntity() {
    smartling_entity_data_save($this->entity);
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
  public function linkToContent() {
    $uri_callback = $this->drupalEntityType . '_uri';
    $uri = $uri_callback($this->contentEntity);
    return l(t('Related entity'), $uri['path']);
  }

  /**
   * Downloads translation data from Smartling server.
   */
  public function downloadTranslation() {
    $progress = $this->getProgressStatus();
    if ($progress === FALSE) {
      return;
    }

    $download_result = $this->smartlingAPI->downloadFile($this->entity);

    libxml_use_internal_errors(true);
    if (FALSE === simplexml_load_string($download_result)) {
      return;
    }
    // This is a download result.
    $xml = new \DOMDocument();
    $xml->loadXML($download_result);

    // @todo Generating file name and saving on disk we need only for debugging purpose.
    // Try to simplify code and move all logic into smartling_save_xml
    // Also maybe use $file_name = $this->buildXmlFileName();
    $file_name = substr($this->entity->file_name, 0, strlen($this->entity->file_name) - 4);
    $translated_file_name = $file_name . '_' . $this->entity->target_language . '.xml';

    // Save result.
    $isSuccess = smartling_save_xml($xml, $this->entity, $translated_file_name, TRUE);

    // If result is saved.
    // @todo finish converting.
    if ($isSuccess) {
      $this->setProgressStatus(SMARTLING_STATUS_EVENT_UPDATE_FIELDS);
      $this->entity->progress = $progress;
      smartling_entity_data_save($this->entity);
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
            $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->drupalEntityType, $this->entity, $this->targetFieldLanguage);
            $this->contentEntity->{$field_name}[$this->drupalTargetLocale] = $fieldProcessor->prepareBeforeDownload($this->contentEntity->{$field_name}[$this->drupalOriginalLocale]);
          }
        }
      }
    }
  }

  /**
   * Implements entity_translation logic to update translation data in Drupal.
   *
   * @todo remove procedural code and use entities from properties.
   */
  public function updateDrupalTranslation() {
    $entity = entity_load_single($this->drupalEntityType, $this->entity->rid);
    $handler = smartling_entity_translation_get_handler($this->drupalEntityType, $entity);
    $translations = $handler->getTranslations();

    // Initialize translations if they are empty.
    if (empty($translations->original)) {
      $handler->initTranslations();
      $handler->saveTranslations();
      // Update the wrapped entity.
      $handler->setEntity($entity);
      $handler->smartlingEntityTranslationFieldAttach();
      $translations = $handler->getTranslations();
    }

    $entity_translation = array(
      'entity_type' => $this->drupalEntityType,
      'entity_id' => $this->entity->rid,
      'translate' => '0',
      'status' => !empty($entity->status) ? $entity->status : 1,
      'language' => $this->drupalTargetLocale,
      'uid' => $this->entity->submitter,
      'changed' => REQUEST_TIME,
    );

    if (isset($translations->data[$this->drupalTargetLocale])) {
      $handler->setTranslation($entity_translation);
    }
    else {
      // Add the new translation.
      $entity_translation += array(
        'source' => $translations->original,
        'created' => !empty($entity->created) ? $entity->created : REQUEST_TIME,
      );
      $handler->setTranslation($entity_translation);
    }
    $handler->saveTranslations();
    // Update the wrapped entity.
    $handler->setEntity($entity);
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
      $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->entity->entity_type, $this->entity, $this->targetFieldLanguage);
      $fieldValue = $fieldProcessor->fetchDataFromXML($xpath);
      $fieldProcessor->setDrupalContentFromXML($fieldValue);
    }

    $this->saveDrupalEntity();
  }

  /**
   * Process given xml parsed object using translated_file.
   */
  public function updateEntityFromXML() {
    // @todo Move it into separate method.
    $file_path = drupal_realpath(smartling_clean_filename(smartling_get_dir($this->entity->translated_file_name), TRUE));

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
      $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->entity->entity_type, $this->entity, $this->targetFieldLanguage);
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
      $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $this->contentEntity, $this->entity->entity_type, $this->entity, $this->targetFieldLanguage);
      if ($fieldProcessor) {
        $data = $fieldProcessor->getSmartlingContent();
        $fieldProcessor->putDataToXML($xml, $localize, $data);
      }
    }

    return $localize;
  }

  /**
   * Build name for translations xml file.
   *
   * @todo move it to XML convector class.
   *
   * @return string
   */
  public function buildXmlFileName() {
    return strtolower(trim(preg_replace('#\W+#', '_', $this->contentEntityWrapper->label()), '_')) . '_' . $this->contentEntityWrapper->type() . '_' . $this->contentEntityWrapper->getIdentifier() . '.xml';
  }

  /**
   * Wrapper for Smartling settings storage.
   *
   * @todo avoid procedural code and inject storage to keep DI pattern.
   *
   * @return array()
   */
  public function getTranslatableFields() {
    // @todo Inject via DIC.
    return smartling_settings_get_handler()->getFieldsSettingsByBundle($this->entity->entity_type, $this->entity->bundle);
  }

  public function sendToUploadQueue() {
    global $user;
    $this->entity->translated_file_name = FALSE;
    $this->entity->submitter = $user->uid;
    $this->entity->submission_date = REQUEST_TIME;

    $this->setProgressStatus(SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
  }
}
<?php

/**
 * @file
 * Contains Drupal\smartling\Processors\GenericEntityProcessor.
 *
 * @todo rename namespace to EntityProcessor or something else.
 */

namespace Drupal\smartling\Processors;

use DOMXPath;
use Drupal\smartling\FieldProcessors\BaseFieldProcessor;
use Drupal\smartling\FieldProcessors\FieldProcessorFactory;

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
   * List of drupal content entity fields that could be translated.
   *
   * @var array
   */
  protected $fields;

  /**
   * Contain drupal content entity type.
   *
   * @var string
   */
  protected $originalEntityType;

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
   * Contains locale in drupal format, e.g. 'en', 'und'.
   *
   * @var string
   */
  protected $drupalLocale;

  /**
   * Contains locale in drupal format, e.g. 'en-US', 'ru-RU'.
   *
   * @var string
   */
  protected $originalLocale;

  /**
   * Contains if drupal content bundle has "Field translation" mode.
   *
   * @var bool
   */
  protected $ifFieldMethod;
  // @todo choose better name.
  /**
   * @var \Drupal\smartling\ApiWrapper\SmartlingApiWrapper
   */
  protected $smartlingAPI;

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
   * @param $entity object
   *   Smartling data entity.
   * @param $log \Drupal\smartling\Log\SmartlingLog
   *   Smartling log object.
   *
   * @todo avoid procedural code in construct to achieve full DI.
   */
  public function __construct($entity, $log) {
    $this->entity = $entity;
    $this->drupalLocale = $entity->target_language;
    $this->originalLocale = smartling_convert_locale_drupal_to_smartling($entity->target_language);
    $this->relatedId = $entity->rid;

    $this->contentEntity = entity_load_single($this->entity->entity_type, $this->entity->rid);
    $this->originalEntityType = $this->entity->entity_type;
    $this->ifFieldMethod = smartling_fields_method($this->contentEntity->type);

    $this->log = $log;
    $this->smartlingAPI = drupal_container()->get('smartling.api_wrapper');
  }

  /**
   * Fetch translation status from Smartling server.
   *
   * @return bool
   */
  public function getProgressStatus() {
    if (!empty($this->entity->file_name)) {
      $result = $this->smartlingAPI->getStatus($this->entity, $this->linkToContent());

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
   *
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
    entity_save($this->originalEntityType, $this->contentEntity);
  }

  /**
   * Get link to drupal content.
   *
   * @todo move this logic to original entity Proxy object.
   */
  public function linkToContent() {
    $uri_callback = $this->entity->entity_type . '_uri';
    return l(t('Related entity'), $uri_callback($this->contentEntity));
  }

  /**
   * Downloads translation data from Smartling server and push into drupal entity.
   */
  public function downloadTranslation() {
    $download_result = $this->smartlingAPI->downloadFile($this->entity, $this->linkToContent());
    // This is a download result.
    $xml = new \DOMDocument();
    $xml->loadXML($download_result);

    $file_name = substr($this->entity->file_name, 0, strlen($this->entity->file_name) - 4);
    $translated_file_name = $file_name . '_' . $this->entity->target_language . '.xml';

    // Save result.
    $isSuccessfulSave = smartling_save_xml($xml, $this->entity, $translated_file_name, TRUE);

    // If result is saved.
    // @todo finish converting.
    if ($isSuccessfulSave) {
      $this->setProgressStatus(SMARTLING_STATUS_EVENT_UPDATE_FIELDS);
      $this->updateDrupalTranslation();
    }
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
      $this->contentEntity = entity_load($this->entity->bundle, array($this->entity->rid));
      $this->isOriginalEntityPrepared = TRUE;
    }
  }

  /**
   * Implements entity_translation logic to update translation data in Drupal.
   *
   * Should be used after ::importSmartlingTranslationToOriginalEntity()
   * @todo remove procedural code and use entities from properties.
   */
  public function updateDrupalTranslation() {
    $entity = entity_load_single($this->originalEntityType, $this->entity->rid);
    $handler = smartling_entity_translation_get_handler($this->originalEntityType, $entity);
    $translations = $handler->getTranslations();

    // Initialize translations if they are empty.
    if (empty($translations->original)) {
      $handler->initTranslations();
      smartling_entity_translation_save($handler, $entity);
      $translations = $handler->getTranslations();
    }

    $entity_translation = array(
      'entity_type' => $this->originalEntityType,
      'entity_id' => $this->entity->rid,
      'translate' => '0',
      'status' => $entity->status,
      'language' => $this->drupalLocale,
      'uid' => $this->entity->submitter,
      'changed' => $this->entity->submission_date,
    );

    if (isset($translations->data[$this->drupalLocale])) {
      $handler->setTranslation($entity_translation);
    }
    else {
      // Add the new translation.
      $entity_translation += array(
        'source' => $translations->original,
        'created' => $entity->created,
      );
      $handler->setTranslation($entity_translation);
    }
    smartling_entity_translation_save($handler, $entity);
  }

  /**
   * Moves translations from smartling entity to drupal content entity.
   *
   * @see smartling_copy_translations_from_xml_to_fields().
   */
  public function importSmartlingTranslationToOriginalEntity() {
    $this->prepareDrupalEntity();

    foreach ($this->getConfiguredFields() as $field_name) {
      /* @var $fieldProcessor BaseFieldProcessor */
      $this->fields[$field_name] = $fieldProcessor = FieldProcessorFactory::getProcessor($field_name, $this->contentEntity, $this->originalEntityType, $this->entity)->setSmartlingEntity((array) $this->entity);

      $this->contentEntity->{$field_name} = $fieldProcessor->getDrupalContent();
    }

    $this->saveDrupalEntity();
  }

  /**
   * Updates smartling data entity from given xml parsed object.
   *
   * @param $xml \DomNode
   */
  public function importSmartlingXMLToSmartlingEntity(\DOMNode $xml) {
    $this->prepareDrupalEntity();
    $xpath = new DomXpath($xml);

    foreach ($this->getConfiguredFields() as $field_name) {
      // Get language key for field translatable type.
      // @todo handle entity/field translation types in field processors.
      if (smartling_field_is_translatable_by_field_name($field_name, $this->originalEntityType)) {
        $language_key = $this->drupalLocale;
      }
      else {
        $language_key = LANGUAGE_NONE;
      }

      // @TODO test if format could be set automatically.
      $fieldProcessor = FieldProcessorFactory::getProcessor($field_name, $this->contentEntity, $this->entity->entity_type, $this->entity);
      $this->contentEntity->{$field_name} = $fieldProcessor->fetchDataFromXML($xpath);
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

    // Update original entity from smartling.
//    $this->importSmartlingTranslationToOriginalEntity();

    // Update translations information.
    $this->updateDrupalTranslation();

  }

  public function exportContentToTranslation() {
    $this->prepareDrupalEntity();
    $entity_current_translatable_content = array();

    foreach ($this->getConfiguredFields() as $field_name) {
      /* @var $fieldProcessor \Drupal\smartling\FieldProcessors\BaseFieldProcessor */
      $this->fields[$field_name] = $fieldProcessor = FieldProcessorFactory::getProcessor($field_name, $this->contentEntity, $this->entity->entity_type, $this->entity);

      if ($fieldProcessor) {
        $entity_current_translatable_content[$field_name] = $fieldProcessor->getSmartlingContent();
      }
    }

    return $entity_current_translatable_content;
  }

  /**
   * Build name for translations xml file.
   *
   * @todo move it to XML convector class.
   *
   * @return string
   */
  public function buildXmlFileName() {
    return strtolower(trim(preg_replace('#\W+#', '_', $this->contentEntity->title), '_')) . '_' . $this->entity->entity_type . '_' . $this->entity->rid . '.xml';
  }

  /**
   * Wrapper for Smartling settings storage.
   *
   * @todo avoid procedural code and inject storage to keep DI pattern.
   *
   * @return array()
   */
  public function getConfiguredFields() {
    return smartling_settings_get_handler()->getFieldsSettings($this->entity->entity_type, $this->entity->bundle);
  }

  /**
   * Clone fields values to new entity during creating new translations
   */
  public function fillFieldFromOriginalLanguage() {
    $this->prepareDrupalEntity();

    if ($this->ifFieldMethod) {
      $field_info_instances = field_info_instances($this->originalEntityType, $this->contentEntity->bundle);
      $fields = $this->getConfiguredFields();
      $need_save = FALSE;
      // @todo finish converting.
      foreach ($field_info_instances as $field) {
        if (!in_array($field['field_name'], $fields) && smartling_field_is_translatable_by_field_name($field['field_name'], $this->originalEntityType) && isset($this->contentEntity->{$field['field_name']})) {
          $need_save = TRUE;
          $original_lang = entity_language($this->originalEntityType, $this->contentEntity);
          $this->contentEntity->{$field['field_name']}[$this->drupalLocale] = $this->contentEntity->{$field['field_name']}[$original_lang];
        }
      }
      if ($need_save) {
        $function_name = $this->originalEntityType . '_save';
        $function_name($this->contentEntity);
      }
    }
  }
}
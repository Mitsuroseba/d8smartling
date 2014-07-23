<?php

namespace Drupal\smartling\Processors;

abstract class BaseEntityProcessor {
  /**
   * @var SmartlingEntityData
   */
  public $entity;

  public $originalEntity;

  protected $fields;

  protected $originalEntityType;

  protected $log;

  protected $relatedId;

  protected $drupalLocale;

  protected $originalLocale;

  protected $ifFieldMethod;

  public function __construct($entity, $locale, $log, $relatedId = NULL) {
    $this->entity = $entity;
    $this->drupalLocale = $locale;
    $this->originalLocale = smartling_convert_locale_drupal_to_smartling($locale);
    $this->relatedId = $relatedId;
    $this->log = $log;
  }

  /**
   * Should be overriden for node and term.
   */
  public function prepareOriginalEntity() {
    $this->originalEntity = entity_load($this->entity->bundle, array($this->entity->rid));
  }

  public function updateTranslation() {
    if (($this->originalEntityType == 'node') && smartling_nodes_method($this->entity->bundle)) {
      return;
    }
    $entity_load = entity_load_single($this->originalEntityType, $rid);
    $handler = smartling_entity_translation_get_handler($this->originalEntityType, $entity_load);
    $translations = $handler->getTranslations();

    // Initialize translations if they are empty.
    if (empty($translations->original)) {
      $handler->initTranslations();
      smartling_entity_translation_save($handler, $entity_load);
      $translations = $handler->getTranslations();
    }

    $entity_translation = array(
      'entity_type' => $this->originalEntityType,
      'entity_id' => $rid,
      'translate' => '0',
      'status' => $entity_load->status,
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
        'created' => $entity_load->created,
      );
      $handler->setTranslation($entity_translation);
    }
    smartling_entity_translation_save($handler, $entity_load);
  }

  /**
   * @see smartling_copy_translations_from_xml_to_fields().
   */
  public function importXMLTranslation($xml) {

  }

  /**
   * Process all field types at beginning.
   */
  public function processFields() {

  }

  public function exportContentToXML() {

  }
}
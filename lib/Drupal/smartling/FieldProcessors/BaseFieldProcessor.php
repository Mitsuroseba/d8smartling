<?php

namespace Drupal\smartling\FieldProcessors;

abstract class BaseFieldProcessor {

  protected $entityType;
  protected $entity;
  protected $language;
  protected $fieldName;

  protected $smartlingData;

  public function __construct($entity, $entity_type, $language, $field_name, $smartling_data = NULL) {
    $this->entity = $entity;
    $this->entityType = $entity_type;
    $this->language = $language;
    $this->fieldName = $field_name;
    $this->smartlingData = $smartling_data;

    return $this;
  }

  public function setSmartlingData($smartling_data) {
    $this->smartlingData = $smartling_data;

    return $this;
  }

  abstract public function getSmartlingFormat();
  abstract public function getDrupalFormat();

}

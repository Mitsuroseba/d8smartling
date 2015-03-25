<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessor\FieldProcessorFactory.
 *
 * @todo move to Drupal\smartling namespace.
 */

namespace Drupal\smartling;

/**
 * Factory that creates field processor instances and contains mapping.
 *
 * @package Drupal\smartling\FieldProcessors
 */
class FieldProcessorFactory {

  protected $fieldMapping;
  protected $log;
  protected $field_api_wrapper;

  /**
   * @param array $field_mapping
   * @param SmartlingLog $logger
   * @param $field_api_wrapper
   */
  public function __construct($field_mapping, $logger, $field_api_wrapper) {
    $this->alter('smartling_field_processor_mapping_info', $field_mapping);
    $this->fieldMapping = $field_mapping;
    $this->log = $logger;
    $this->field_api_wrapper = $field_api_wrapper;
  }

  /*
   * A wrapper for Drupal drupal_alter function
   */
  protected function alter($hook_name, &$handlers) {
    drupal_alter($hook_name, $handlers);
  }

  /**
   * Factory method for FieldProcessor instances.
   *
   * @param string $field_name
   * @param \stdClass $entity
   * @param string $entity_type
   * @param \stdClass $smartling_entity
   * @param string $target_language
   * @param null|string $source_language
   *
   * @return BaseFieldProcessor
   */
  public function getProcessor($field_name, $entity, $entity_type, $smartling_entity, $target_language, $source_language = NULL) {
    $field_info = $this->field_api_wrapper->fieldInfoField($field_name);

    if ($field_info) {
      $type = $field_info['type'];
      // @todo we could get notice about invalid key here.
      $class_name = $this->fieldMapping['real'][$type];
    }
    elseif (isset($this->fieldMapping['fake'][$field_name])) {
      $type = $field_name;
      $class_name = $this->fieldMapping['fake'][$type];
    }
    else {
      $this->log->setMessage("Smartling found unexisted field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    if (!$class_name) {
      $this->log->setMessage("Smartling didn't process content of field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    $source_language = ($source_language ?: ((smartling_field_is_translatable($field_name, $entity_type)) ? entity_language($entity_type, $entity) : LANGUAGE_NONE));

    $field_class = new $class_name(
      $entity,
      $entity_type,
      $field_name,
      $smartling_entity,
      $source_language,
      $target_language
    );

    return $field_class;
  }

  public function isSupportedField($field_name) {
    $supported = FALSE;
    $field_info = $this->field_api_wrapper->fieldInfoField($field_name);

    if ($field_info) {
      $type = $field_info['type'];
      $supported = isset($this->fieldMapping['real'][$type]);
    }
    elseif (isset($this->fieldMapping['fake'][$field_name])) {
      $type = $field_name;
      $supported = isset($this->fieldMapping['fake'][$type]);
    }
    return (bool) $supported;
  }
}
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

  protected $field_mapping;
  protected $log;
  protected $field_api_wrapper;
  protected $entity_api_wrapper;
  protected $drupal_api_wrapper;
  protected $smartling_utils;

  /**
   * @param array $field_mapping
   * @param SmartlingLog $logger
   * @param $field_api_wrapper
   * @param $entity_api_wrapper
   * @param $drupal_api_wrapper
   * @param $smartling_utils
   */
  public function __construct($field_mapping, $logger, $field_api_wrapper, $entity_api_wrapper, $drupal_api_wrapper, $smartling_utils) {
    $this->log = $logger;
    $this->field_api_wrapper = $field_api_wrapper;
    $this->entity_api_wrapper = $entity_api_wrapper;
    $this->drupal_api_wrapper = $drupal_api_wrapper;
    $this->smartling_utils = $smartling_utils;

    $this->drupal_api_wrapper->alter('smartling_field_processor_mapping_info', $field_mapping);
    $this->field_mapping = $field_mapping;
  }

  public function getContainer() {
    return drupal_container();
  }

  /**
   * Factory method for FieldProcessor instances.
   *
   * @param string $field_name
   * @param \stdClass $entity
   * @param string $entity_type
   * @param \stdClass $smartling_submission
   *
   * @return BaseFieldProcessor
   */
  public function getProcessor($field_name, $entity, $entity_type, $smartling_submission) {
    $field_info = $this->field_api_wrapper->fieldInfoField($field_name);

    if ($field_info) {
      $type = $field_info['type'];
      // @todo we could get notice about invalid key here.
      $service_id = $this->field_mapping['real'][$type];
    }
    elseif (isset($this->field_mapping['fake'][$field_name])) {
      $type = $field_name;
      $service_id = $this->field_mapping['fake'][$type];
    }
    else {
      $this->log->setMessage("Smartling found unexisted field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    if (!$service_id) {
      $this->log->setMessage("Smartling didn't process content of field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    $container = $this->getContainer();
    $container->setParameter('field_name', $field_name);
    $container->setParameter('drupal_entity', $entity);
    $container->setParameter('entity_type', $entity_type);
    $container->setParameter('smartling_submission', $smartling_submission);
    //new $class_name($field_name, $entity, $entity_type, $smartling_submission);
    return $container->get($service_id);
  }

  public function isSupportedField($field_name) {
    $supported = FALSE;
    $field_info = $this->field_api_wrapper->fieldInfoField($field_name);

    if ($field_info) {
      $type = $field_info['type'];
      $supported = isset($this->field_mapping['real'][$type]);
    }
    elseif (isset($this->field_mapping['fake'][$field_name])) {
      $type = $field_name;
      $supported = isset($this->field_mapping['fake'][$type]);
    }
    return (bool) $supported;
  }
}
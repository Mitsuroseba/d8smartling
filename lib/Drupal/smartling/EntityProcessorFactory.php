<?php

/**
 * @file
 * Contains Drupal\smartling\EntityProcessorFactory.
 */

namespace Drupal\smartling;

use Drupal\smartling\ApiWrapper\SmartlingApiWrapper;
use Drupal\smartling\Log\SmartlingLog;
use Drupal\smartling\Processors\GenericEntityProcessor;

/**
 * Factory that creates entity processor instances and caches it statically.
 *
 * @package Drupal\smartling
 */
class EntityProcessorFactory {

  /**
   * @var array
   *   entity_type => ProcessorClass
   */
  protected $processorMapping;

  /**
   * @var SmartlingLog
   */
  protected $logger;

  /**
   * @var SmartlingApiWrapper
   */
  protected $smartlingAPI;

  /**
   * @var FieldProcessorFactory
   */
  protected $fieldProcessorFactory;

  /**
   * @param array $processor_mapping
   * @param FieldProcessorFactory $field_processor_factory
   * @param SmartlingLog $logger
   * @param SmartlingApiWrapper $smartling_api
   */
  public function __construct($processor_mapping, $field_processor_factory, $logger, $smartling_api) {
    $this->alter('smartling_entity_processor_mapping_info', $processor_mapping);
    $this->processorMapping = $processor_mapping;
    $this->logger = $logger;
    $this->smartlingAPI = $smartling_api;
    $this->fieldProcessorFactory = $field_processor_factory;
  }

  /*
   * A wrapper for Drupal drupal_alter function
   */
  protected function alter($hook_name, &$handlers) {
    drupal_alter($hook_name, $handlers);
  }

  /**
   * Creates GenericEntityProcessor instance based on entity type.
   *
   * Also caches instances statically to work with nested usages.
   *
   * @param $smartling_entity \stdClass|\SmartlingEntityData
   *
   * @return GenericEntityProcessor
   */
  public function getProcessor($smartling_entity) {
    $static_storage = &drupal_static(__CLASS__ . '_' . __METHOD__, array());

    if (!empty($static_storage[$smartling_entity->eid])) {
      return $static_storage[$smartling_entity->eid];
    }

    // @Todo avoid hardcoding 'generic' key.
    $processor_class = isset($this->processorMapping[$smartling_entity->entity_type]) ? $this->processorMapping[$smartling_entity->entity_type] : $this->processorMapping['generic'];

    $static_storage[$smartling_entity->eid] = new $processor_class($smartling_entity, $this->fieldProcessorFactory, $this->smartlingAPI, $this->logger);

    return $static_storage[$smartling_entity->eid];
  }

}
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

  protected $drupal_api_wrapper;

  /**
   * @param array $processor_mapping
   * @param $drupal_api_wrapper
   */
  public function __construct($processor_mapping, $drupal_api_wrapper) {
    $this->drupal_api_wrapper = $drupal_api_wrapper;

    $this->drupal_api_wrapper->alter('smartling_entity_processor_mapping_info', $processor_mapping);
    $this->processorMapping = $processor_mapping;
  }

  /**
   * Creates GenericEntityProcessor instance based on entity type.
   *
   * Also caches instances statically to work with nested usages.
   *
   * @param $smartling_submission \stdClass|\SmartlingEntityData
   *
   * @return GenericEntityProcessor
   */
  public function getProcessor($smartling_submission) {
    $container = drupal_container();
    $static_storage = &$this->drupal_api_wrapper->drupalStatic(__CLASS__ . '_' . __METHOD__, array());

    if (!empty($static_storage[$smartling_submission->eid])) {
      return $static_storage[$smartling_submission->eid];
    }

    // @Todo avoid hardcoding 'generic' key.
    $processor_yml_id = isset($this->processorMapping[$smartling_submission->entity_type]) ? $this->processorMapping[$smartling_submission->entity_type] : $this->processorMapping['generic'];

    $container->setParameter('smartling_submission', $smartling_submission);
    $static_storage[$smartling_submission->eid] = $container->get($processor_yml_id);//new $processor_class($smartling_entity, $this->fieldProcessorFactory, $this->smartlingAPI, $this->logger);

    return $static_storage[$smartling_submission->eid];
  }

}
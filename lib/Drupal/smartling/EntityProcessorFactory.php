<?php

/**
 * @file
 * Contains Drupal\smartling\EntityProcessorFactory.
 */

namespace Drupal\smartling;

use Drupal\smartling\Processors\NodeProcessor;
use Drupal\smartling\Processors\TaxonomyTermProcessor;
use Drupal\smartling\Processors\FieldCollectionProcessor;
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

  public function __construct($processor_mapping) {
    $this->processorMapping = $processor_mapping;
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
    $log = smartling_log_get_handler();
    $smartling_api = drupal_container()->get('smartling.api_wrapper');
    $field_processor_factory = drupal_container()->get('smartling.field_processor_factory');
    $static_storage = &drupal_static(__CLASS__ . '_' . __METHOD__, array());

    if (!empty($static_storage[$smartling_entity->eid])) {
      return $static_storage[$smartling_entity->eid];
    }

    // @Todo avoid hardcoding 'generic' key.
    $processor_class = isset($this->processorMapping[$smartling_entity->entity_type]) ? $this->processorMapping[$smartling_entity->entity_type] : $this->processorMapping['generic'];

    $static_storage[$smartling_entity->eid] = new $processor_class($smartling_entity, $field_processor_factory, $smartling_api, $log);

    return $static_storage[$smartling_entity->eid];
  }

}
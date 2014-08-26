<?php

/**
 * @file
 * Contains Drupal\smartling\EntityProcessorFactory.
 */

namespace Drupal\smartling;

use Drupal\smartling\Processors\NodeProcessor;
use Drupal\smartling\Processors\TaxonomyTermProcessor;
use Drupal\smartling\Processors\GenericEntityProcessor;

/**
 * Factory that creates entity processor instances and caches it statically.
 *
 * @package Drupal\smartling
 */
class EntityProcessorFactory {

  /**
   * Creates GenericEntityProcessor instance based on entity type.
   *
   * Also caches instances statically to work with nested usages.
   *
   * @param $smartling_entity \stdClass|\SmartlingEntityData
   *
   * @return GenericEntityProcessor
   */
  public static function getProcessor($smartling_entity) {
    $log = smartling_log_get_handler();
    $static_storage = &drupal_static(__CLASS__ . '_' . __METHOD__, array());

    if (!empty($static_storage[$smartling_entity->eid])) {
      return $static_storage[$smartling_entity->eid];
    }

    switch ($smartling_entity->entity_type) {
      case 'node':
        $static_storage[$smartling_entity->eid] = new NodeProcessor($smartling_entity, $log);
        return $static_storage[$smartling_entity->eid];
        break;

      case 'taxonomy_term':
        $static_storage[$smartling_entity->eid] = new TaxonomyTermProcessor($smartling_entity, $log);
        return $static_storage[$smartling_entity->eid];
        break;

      default:
        $static_storage[$smartling_entity->eid] = new GenericEntityProcessor($smartling_entity, $log);
        return $static_storage[$smartling_entity->eid];
        break;
    }
  }

}
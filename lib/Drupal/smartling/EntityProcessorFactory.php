<?php

namespace Drupal\smartling;

use Drupal\smartling\Processors\NodeProcessor;
use Drupal\smartling\Processors\TaxonomyTermProcessor;
use Drupal\smartling\Processors\BaseEntityProcessor;

class EntityProcessorFactory {

  /**
   * @param $smartling_entity
   *
   * @return BaseEntityProcessor
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
        $static_storage[$smartling_entity->eid] = new BaseEntityProcessor($smartling_entity, $log);
        return $static_storage[$smartling_entity->eid];
        break;
    }
  }

}
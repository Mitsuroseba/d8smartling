<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Wrappers;

/**
 * Class EntityAPIWrapper.
 */
class EntityAPIWrapper {
  public function entityMetadataWrapper($entity_type, $entity) {
    return entity_metadata_wrapper($entity_type, $entity);
  }

  public function getOriginalEntity($entity_type, $entity) {
    switch ($entity_type) {
      case 'node':
        $entity = smartling_get_original_node($entity);
        break;

      case 'taxonomy_term':
        $entity = smartling_get_original_taxonomy_term($entity);
        break;
    }
    return $entity;
  }

  public function getLink($entity_type, $entity) {
    return smartling_get_link_to_entity($entity_type, $entity);
  }

  public function entityLanguage($entity_type, $entity) {
    return entity_language($entity_type, $entity);
  }
}
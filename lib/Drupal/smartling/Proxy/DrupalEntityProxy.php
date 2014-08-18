<?php

/**
 * @file
 * Contains \Drupal\smartling\Proxy\DrupalEntityProxy.
 */

namespace Drupal\smartling\Proxy;

class DrupalEntityProxy {

  public $entity;
  public $entityType;
  public $entityWrapper;

  public function __construct($entity, $entity_type) {
    $this->entity = $entity;
    $this->entityWrapper = entity_metadata_wrapper($entity_type, $entity);
  }

  public function save() {
    entity_save($this->entityType, $this->entity);
  }

  /**
   * Check if current entity is original or just translation of another entity.
   */
  public function getOriginEntity() {

  }

  public function prepareEntity() {}

  public function getLinkToContent() {}

}
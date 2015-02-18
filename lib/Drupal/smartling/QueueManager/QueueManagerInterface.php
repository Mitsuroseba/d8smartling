<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

interface QueueManagerInterface {
  /**
   * Adds an element to the queue.
   *
   * @param string $entity_type
   *   type of the entity
   * @param object $entity
   *   entity itself.
   * @param array $langs
   *   Array of languages entity should be translated to.
   */
  public function add($entity_type, $entity, $langs);

  /**
   * Runs each queue iteration.
   *
   * @param int|array $eids
   *   A number or associative array containing ids of SmartlingEntities that should be uploaded.
   *   !!! Please be aware that if you pass an array of ids to the method - those should be ids of Smartling entities that
   *  refer to a single Drupal entity but with different target languages!!!
   */
  public function execute($eids);
}

<?php

/**
 * @file
 * Contains \Drupal\smartling\Entity\SmartlingEntityData.
 */

namespace Drupal\smartling\Entity;
use Drupal\Core\Entity\Entity;

/**
 * Defines the node entity class.
 *
 * @ContentEntityType(
 *   id = "smartling_entity_data",
 *   label = @Translation("Smartling Entity Data"),
 *   controllers = {
 *     "storage" => "Drupal\smartling\SmartlingStorageController"
 *     "view_builder" = "Drupal\Core\Entity\EntityViewBuilder"
 *   },
 *   base_table = "smartling_entity_data",
 *   fieldable = FALSE,
 *   entity_keys = {
 *     "id" = "eid",
 *   }
 *
 * )
 */
class SmartlingEntityData extends Entity {}

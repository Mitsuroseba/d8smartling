<?php

/**
 * @file
 * Provides info about the smartling_entity_data entity.
 */

/**
 * Implements hook_entity_property_info() on top of smartling module.
 *
 * @see entity_entity_property_info()
 */
function entity_metadata_smartling_entity_property_info() {
  $info = array();
  // Add meta-data about the basic smartling_entity_data properties.
  $properties = &$info['smartling_entity_data']['properties'];

  $properties['rid'] = array(
    'label' => t("Related ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the entity."),
    'schema field' => 'rid',
  );
  return $info;
}

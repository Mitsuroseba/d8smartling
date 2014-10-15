<?php
/**
 * @file
 * smartling_field_collection_content_demo.features.inc
 */

/**
 * Implements hook_node_info().
 */
function smartling_field_collection_content_demo_node_info() {
  $items = array(
    'collection_test' => array(
      'name' => t('Collection test'),
      'base' => 'node_content',
      'description' => t('Content type that is used to test integration of field_collections with Smartling module'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}

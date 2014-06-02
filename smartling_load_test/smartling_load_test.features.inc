<?php
/**
 * @file
 * smartling_load_test.features.inc
 */

/**
 * Implements hook_node_info().
 */
function smartling_load_test_node_info() {
  $items = array(
    'smartling_load_test' => array(
      'name' => t('Smartling load-test'),
      'base' => 'node_content',
      'description' => t('Content type for dev test'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}

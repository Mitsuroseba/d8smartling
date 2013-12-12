<?php
/**
 * @file
 * smartling_load_tests.features.inc
 */

/**
 * Implements hook_views_api().
 */
function smartling_load_tests_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function smartling_load_tests_node_info() {
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

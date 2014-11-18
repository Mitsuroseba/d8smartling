<?php
/**
 * @file
 * smartling_demo_content.features.inc
 */

/**
 * Implements hook_node_info().
 */
function smartling_demo_content_node_info() {
  $items = array(
    'article' => array(
      'name' => t('Article'),
      'base' => 'node_content',
      'description' => t('Use <em>articles</em> for time-sensitive content like news, press releases or blog posts.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'collection_test' => array(
      'name' => t('Collection test'),
      'base' => 'node_content',
      'description' => t('Content type that is used to test integration of field_collections with Smartling module'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'travel' => array(
      'name' => t('Travel'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}

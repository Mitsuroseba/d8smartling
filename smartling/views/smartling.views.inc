<?php

/**
 * @file
 * Smartling views integrations.
 */

/**
 * Implements hook_views_data_alter().
 */
function smartling_views_data_alter(&$data) {
  // Node relationship
  $data['node']['smartling_entity_data'] = array(
    'group' => t('Smartling'),
    'title' => t('Smartling Entity'),
    'help' => t('Relationship.'),
    'relationship' => array(
      'base' => 'smartling_entity_data',
      'base field' => 'rid',
      'relationship field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Smartling related ID'),
    )
  );

  // Add views handler
  $data['smartling_entity_data']['target_language'] = array(
    'title' => t('Target Language'),
    'help' => t('Smartling entity data "target_language" property.'),
  );
  $data['smartling_entity_data']['target_language']['field'] = array(
    'real field' => 'target_language',
    'handler' => 'smartling_handler_field_target_language',
    'click sortable' => TRUE,
  );
  $data['smartling_entity_data']['target_language']['filter'] = array(
    'handler' => 'views_handler_filter_string',
  );

  $data['smartling_entity_data']['smartling_title'] = array(
    'title' => t('Smartling entity Title'),
    'help' => t('Smartling entity Title link.'),
  );
  $data['smartling_entity_data']['smartling_title']['field'] = array(
    'real field' => 'eid',
    'handler' => 'smartling_handler_field_smartling_title',
    'click sortable' => TRUE,
  );
  $data['smartling_entity_data']['smartling_title']['sort'] = array(
    'handler' => 'views_handler_sort',
  );

  $data['smartling_entity_data']['smartling_operations'] = array(
    'title' => t('Smartling entity OPERATIONS'),
    'help' => t('Smartling entity OPERATIONS.'),
  );
  $data['smartling_entity_data']['smartling_operations']['field'] = array(
    'real field' => 'eid',
    'handler' => 'smartling_handler_field_smartling_operations',
    'click sortable' => TRUE,
  );

  $data['node']['locale_info'] = array(
    'group' => t('Smartling'),
    'title' => t('Locales for node'),
    'help' => t('Smartling locales property.'),
  );
  $data['node']['locale_info']['field'] = array(
    'real field' => 'nid',
    'handler' => 'smartling_handler_field_locale_info',
    'click sortable' => TRUE,
  );

  $data['taxonomy_term_data']['locale_info'] = array(
    'group' => t('Smartling'),
    'title' => t('Locales for taxonomy_term_data'),
    'help' => t('Smartling locales property.'),
  );
  $data['taxonomy_term_data']['locale_info']['field'] = array(
    'real field' => 'tid',
    'handler' => 'smartling_handler_field_locale_info',
    'click sortable' => TRUE,
  );

  $data['comment']['locale_info'] = array(
    'group' => t('Smartling'),
    'title' => t('Locales for comment'),
    'help' => t('Smartling locales property.'),
  );
  $data['comment']['locale_info']['field'] = array(
    'real field' => 'cid',
    'handler' => 'smartling_handler_field_locale_info',
    'click sortable' => TRUE,
  );

  $data['users']['locale_info'] = array(
    'group' => t('Smartling'),
    'title' => t('Locales for user'),
    'help' => t('Smartling locales property.'),
  );
  $data['users']['locale_info']['field'] = array(
    'real field' => 'uid',
    'handler' => 'smartling_handler_field_locale_info',
    'click sortable' => TRUE,
  );

  $data['smartling_entity_data']['submitter']['relationship'] = array(
    'title' => t('Submitter'),
    'help' => t('The user submit to translate.'),
    'base' => 'users',
    'field' => 'submitter',
    'label' => t('Smartling submitter'),
  );

  $data['smartling_entity_data']['submission_date'] = array(
    'title' => t('Date'),
  );

  $data['smartling_entity_data']['submission_date']['field'] = array(
    'handler' => 'views_handler_field_date',
    'click sortable' => TRUE,
  );

  $data['smartling_entity_data']['submission_date']['sort'] = array(
    'handler' => 'views_handler_sort_date',
  );

  $data['smartling_entity_data']['submission_date']['filter'] = array(
    'handler' => 'views_handler_filter_date',
  );

  unset($data['smartling_entity_data']['submission_date']['argument']);
}

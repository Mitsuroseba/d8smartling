<?php

/**
 * @file
 * Smartling views integrations.
 */

/**
 * Implements hook_views_data_alter().
 */
function smartling_views_data_alter(&$data) {

  $data['node']['smartling_entity_data'] = array(
    'group' => t('Smartling'),
    'title' => t('Smartling Entity'),
    'help' => t('Relate a user to the orders they have placed. This relationship will create one record for each order placed by the user.'),
    'relationship' => array(
      'base' => 'smartling_entity_data',
      'base field' => 'rid',
      'relationship field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Smartling related ID'),
    )
  );

  $data['smartling_entity_data']['node'] = array(
    'group' => t('Node'),
    'title' => t('Node Entity'),
    'help' => t('Relationship with entity node.'),
    'relationship' => array(
      'base' => 'node',
      'base field' => 'nid',
      'relationship field' => 'rid',
      'handler' => 'views_handler_relationship',
      'label' => t('Node entity ID'),
    )
  );

  $data['smartling_entity_data']['submitter']['relationship'] = array(
    'title' => t('Submitter'),
    'help' => t('The user submit to translate.'),
    'base' => 'users',
    'field' => 'submitter',
    'label' => t('Smartling submitter'),
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

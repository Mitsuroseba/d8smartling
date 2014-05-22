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

  $data['smartling_entity_data']['status'] = array(
    'title' => t('Smartling status'),
    'help' => t('The status type (for example, "in queue", "in translate", etc).'),
    'filter' => array(
      'handler' => 'smartling_handler_filter_entity_status',
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
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

  $data['node']['locale_info'] = array(
    'group' => t('Smartling'),
    'title' => t('Locales'),
    'help' => t('Smartling locales property.'),
  );
  $data['node']['locale_info']['field'] = array(
    'real field' => 'nid',
    'handler' => 'smartling_handler_field_locale_info',
    'click sortable' => TRUE,
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

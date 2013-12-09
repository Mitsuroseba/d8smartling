<?php
/**
 * @file
 * smartling_reports.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function smartling_reports_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'smartlig_report';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'smartling_entity_data';
  $view->human_name = 'Smartlig report';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Completed Submissions';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '100';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'nid' => 'nid',
    'title' => 'title',
    'original_language' => 'original_language',
    'target_language' => 'target_language',
    'file_name' => 'file_name',
    'translated_file_name' => 'translated_file_name',
    'name' => 'name',
    'type' => 'type',
    'submission_date' => 'submission_date',
    'progress' => 'progress',
    'download' => 'download',
    'nothing' => 'nothing',
  );
  $handler->display->display_options['style_options']['default'] = 'nid';
  $handler->display->display_options['style_options']['info'] = array(
    'nid' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'title' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'original_language' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'target_language' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'file_name' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'translated_file_name' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'name' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'type' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'submission_date' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'progress' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'download' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'nothing' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* Relationship: Node: Node Entity */
  $handler->display->display_options['relationships']['node']['id'] = 'node';
  $handler->display->display_options['relationships']['node']['table'] = 'smartling_entity_data';
  $handler->display->display_options['relationships']['node']['field'] = 'node';
  $handler->display->display_options['relationships']['node']['required'] = TRUE;
  /* Relationship: Smartling Entity Data: Submitter */
  $handler->display->display_options['relationships']['submitter']['id'] = 'submitter';
  $handler->display->display_options['relationships']['submitter']['table'] = 'smartling_entity_data';
  $handler->display->display_options['relationships']['submitter']['field'] = 'submitter';
  $handler->display->display_options['relationships']['submitter']['required'] = TRUE;
  /* Field: Content: Nid */
  $handler->display->display_options['fields']['nid']['id'] = 'nid';
  $handler->display->display_options['fields']['nid']['table'] = 'node';
  $handler->display->display_options['fields']['nid']['field'] = 'nid';
  $handler->display->display_options['fields']['nid']['relationship'] = 'node';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['relationship'] = 'node';
  /* Field: Smartling Entity Data: Original_language */
  $handler->display->display_options['fields']['original_language']['id'] = 'original_language';
  $handler->display->display_options['fields']['original_language']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['original_language']['field'] = 'original_language';
  $handler->display->display_options['fields']['original_language']['label'] = 'Original language';
  $handler->display->display_options['fields']['original_language']['exclude'] = TRUE;
  /* Field: Smartling Entity Data: Target Language */
  $handler->display->display_options['fields']['target_language']['id'] = 'target_language';
  $handler->display->display_options['fields']['target_language']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['target_language']['field'] = 'target_language';
  $handler->display->display_options['fields']['target_language']['label'] = 'Locale';
  /* Field: Smartling Entity Data: File_name */
  $handler->display->display_options['fields']['file_name']['id'] = 'file_name';
  $handler->display->display_options['fields']['file_name']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['file_name']['field'] = 'file_name';
  $handler->display->display_options['fields']['file_name']['label'] = 'Original filename';
  $handler->display->display_options['fields']['file_name']['exclude'] = TRUE;
  /* Field: Smartling Entity Data: Translated_file_name */
  $handler->display->display_options['fields']['translated_file_name']['id'] = 'translated_file_name';
  $handler->display->display_options['fields']['translated_file_name']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['translated_file_name']['field'] = 'translated_file_name';
  $handler->display->display_options['fields']['translated_file_name']['label'] = 'Translated filename';
  $handler->display->display_options['fields']['translated_file_name']['exclude'] = TRUE;
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['relationship'] = 'submitter';
  $handler->display->display_options['fields']['name']['label'] = 'Submitter name';
  /* Field: Content: Type */
  $handler->display->display_options['fields']['type']['id'] = 'type';
  $handler->display->display_options['fields']['type']['table'] = 'node';
  $handler->display->display_options['fields']['type']['field'] = 'type';
  $handler->display->display_options['fields']['type']['relationship'] = 'node';
  $handler->display->display_options['fields']['type']['label'] = 'Content type';
  /* Field: Smartling Entity Data: Submission_date */
  $handler->display->display_options['fields']['submission_date']['id'] = 'submission_date';
  $handler->display->display_options['fields']['submission_date']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['submission_date']['field'] = 'submission_date';
  $handler->display->display_options['fields']['submission_date']['label'] = 'Submission date';
  /* Field: Smartling Entity Data: Progress */
  $handler->display->display_options['fields']['progress']['id'] = 'progress';
  $handler->display->display_options['fields']['progress']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['progress']['field'] = 'progress';
  $handler->display->display_options['fields']['progress']['separator'] = '';
  $handler->display->display_options['fields']['progress']['suffix'] = ' %';
  /* Field: Smartling Entity Data: Download */
  $handler->display->display_options['fields']['download']['id'] = 'download';
  $handler->display->display_options['fields']['download']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['download']['field'] = 'download';
  $handler->display->display_options['fields']['download']['exclude'] = TRUE;
  /* Field: Smartling Entity Data: Smartling entity data ID */
  $handler->display->display_options['fields']['eid']['id'] = 'eid';
  $handler->display->display_options['fields']['eid']['table'] = 'smartling_entity_data';
  $handler->display->display_options['fields']['eid']['field'] = 'eid';
  $handler->display->display_options['fields']['eid']['label'] = '';
  $handler->display->display_options['fields']['eid']['exclude'] = TRUE;
  $handler->display->display_options['fields']['eid']['element_type'] = '0';
  $handler->display->display_options['fields']['eid']['element_label_type'] = '0';
  $handler->display->display_options['fields']['eid']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['eid']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['eid']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['eid']['separator'] = '';
  /* Field: Global: Custom text */
  $handler->display->display_options['fields']['nothing']['id'] = 'nothing';
  $handler->display->display_options['fields']['nothing']['table'] = 'views';
  $handler->display->display_options['fields']['nothing']['field'] = 'nothing';
  $handler->display->display_options['fields']['nothing']['label'] = 'Operations';
  $handler->display->display_options['fields']['nothing']['alter']['text'] = '<a href="/node/[nid]/edit">Edit</a> | <a href="/smartling/download/[nid]/[eid]?destination=admin/content/smartling/report-in-progress">Download</a>';
  /* Sort criterion: Smartling Entity Data: Submission_date */
  $handler->display->display_options['sorts']['submission_date']['id'] = 'submission_date';
  $handler->display->display_options['sorts']['submission_date']['table'] = 'smartling_entity_data';
  $handler->display->display_options['sorts']['submission_date']['field'] = 'submission_date';
  $handler->display->display_options['sorts']['submission_date']['order'] = 'DESC';
  /* Filter criterion: Content: Title */
  $handler->display->display_options['filters']['title']['id'] = 'title';
  $handler->display->display_options['filters']['title']['table'] = 'node';
  $handler->display->display_options['filters']['title']['field'] = 'title';
  $handler->display->display_options['filters']['title']['relationship'] = 'node';
  $handler->display->display_options['filters']['title']['operator'] = 'word';
  $handler->display->display_options['filters']['title']['exposed'] = TRUE;
  $handler->display->display_options['filters']['title']['expose']['operator_id'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['label'] = 'Title';
  $handler->display->display_options['filters']['title']['expose']['operator'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['identifier'] = 'title';
  $handler->display->display_options['filters']['title']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Smartling Entity Data: Progress */
  $handler->display->display_options['filters']['progress']['id'] = 'progress';
  $handler->display->display_options['filters']['progress']['table'] = 'smartling_entity_data';
  $handler->display->display_options['filters']['progress']['field'] = 'progress';
  $handler->display->display_options['filters']['progress']['value']['value'] = '100';
  $handler->display->display_options['filters']['progress']['expose']['operator_id'] = 'progress_op';
  $handler->display->display_options['filters']['progress']['expose']['label'] = 'Progress';
  $handler->display->display_options['filters']['progress']['expose']['operator'] = 'progress_op';
  $handler->display->display_options['filters']['progress']['expose']['identifier'] = 'progress';

  /* Display: Completed Submissions */
  $handler = $view->new_display('page', 'Completed Submissions', 'page');
  $handler->display->display_options['path'] = 'admin/content/smartling/report-completed';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Completed Submissions';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['context'] = 1;
  $handler->display->display_options['menu']['context_only_inline'] = 0;
  $handler->display->display_options['tab_options']['type'] = 'tab';
  $handler->display->display_options['tab_options']['title'] = 'Submissions Report';
  $handler->display->display_options['tab_options']['weight'] = '0';

  /* Display: Submissions in Progress */
  $handler = $view->new_display('page', 'Submissions in Progress', 'page_1');
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['title'] = 'Submissions in Progress';
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Content: Title */
  $handler->display->display_options['filters']['title']['id'] = 'title';
  $handler->display->display_options['filters']['title']['table'] = 'node';
  $handler->display->display_options['filters']['title']['field'] = 'title';
  $handler->display->display_options['filters']['title']['relationship'] = 'node';
  $handler->display->display_options['filters']['title']['operator'] = 'word';
  $handler->display->display_options['filters']['title']['exposed'] = TRUE;
  $handler->display->display_options['filters']['title']['expose']['operator_id'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['label'] = 'Title';
  $handler->display->display_options['filters']['title']['expose']['operator'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['identifier'] = 'title';
  $handler->display->display_options['filters']['title']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Smartling Entity Data: Progress */
  $handler->display->display_options['filters']['progress']['id'] = 'progress';
  $handler->display->display_options['filters']['progress']['table'] = 'smartling_entity_data';
  $handler->display->display_options['filters']['progress']['field'] = 'progress';
  $handler->display->display_options['filters']['progress']['operator'] = '<';
  $handler->display->display_options['filters']['progress']['value']['value'] = '100';
  $handler->display->display_options['filters']['progress']['expose']['operator_id'] = 'progress_op';
  $handler->display->display_options['filters']['progress']['expose']['label'] = 'Progress';
  $handler->display->display_options['filters']['progress']['expose']['operator'] = 'progress_op';
  $handler->display->display_options['filters']['progress']['expose']['identifier'] = 'progress';
  /* Filter criterion: Smartling Entity Data: Target Language */
  $handler->display->display_options['filters']['target_language']['id'] = 'target_language';
  $handler->display->display_options['filters']['target_language']['table'] = 'smartling_entity_data';
  $handler->display->display_options['filters']['target_language']['field'] = 'target_language';
  $handler->display->display_options['filters']['target_language']['operator'] = '!=';
  $handler->display->display_options['filters']['target_language']['value'] = 'original';
  $handler->display->display_options['path'] = 'admin/content/smartling/report-in-progress';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Submissions in Progress';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['context'] = 1;
  $handler->display->display_options['menu']['context_only_inline'] = 0;
  $handler->display->display_options['tab_options']['type'] = 'normal';
  $handler->display->display_options['tab_options']['title'] = 'Submissions Report';
  $handler->display->display_options['tab_options']['weight'] = '0';
  $translatables['smartlig_report'] = array(
    t('Master'),
    t('Completed Submissions'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('« first'),
    t('‹ previous'),
    t('next ›'),
    t('last »'),
    t('Node entity ID'),
    t('Smartling submitter'),
    t('Nid'),
    t('Title'),
    t('Original language'),
    t('Locale'),
    t('Original filename'),
    t('Translated filename'),
    t('Submitter name'),
    t('Content type'),
    t('Submission date'),
    t('Progress'),
    t('.'),
    t(' %'),
    t('Download'),
    t('Operations'),
    t('<a href="/node/[nid]/edit">Edit</a> | <a href="/smartling/download/[nid]/[eid]?destination=admin/content/smartling/report-in-progress">Download</a>'),
    t('Submissions in Progress'),
  );
  $export['smartlig_report'] = $view;

  return $export;
}

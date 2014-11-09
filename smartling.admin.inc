<?php

/**
 * @file
 * Admin forms and other for Smartling module administration section.
 */

use Drupal\smartling\Settings\SmartlingSettingsHandler;

/**
 * Smartling settings page callback.
 *
 * @return array
 *   Return render array.
 */
function smartling_admin_configuration_view() {
  // Add ajax library.
  drupal_add_library('system', 'drupal.ajax');

  global $_fix_field_settings;
  $_fix_field_settings = array();

  $output['smartling'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'smartling') . '/js/smartling_admin.js'),
      'css' => array(drupal_get_path('module', 'smartling') . '/css/smartling_admin.css'),
    ),
  );

  $settings_forms = module_invoke_all('smartling_settings_form_info');

  foreach ($settings_forms as $machine_name => $title) {
    $form = drupal_get_form($machine_name);
    $output['smartling'][] = smartling_wrap_in_fieldset($form, $title);
  }

  if (!empty($_fix_field_settings) && is_array($_fix_field_settings)) {
    $text = t('Following field(s) should be set as untranslatable for translation: !fields.', array('!fields' => implode(', ', $_fix_field_settings)));
    $suffix = '<div id="smartling-untranslatable-fix">' . $text . '</div>';
    $output['fix'] = array(
      '#type' => 'markup',
      '#title' => t('Fix field settings'),
      '#title_display' => 'invisible',
      '#suffix' => $suffix,
    );
  }

  return $output;
}

/**
 * Form for smartling account settings.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling account settings.
 */
function smartling_admin_account_info_settings_form(array $form, array &$form_state) {
  $smartling_settings = smartling_settings_get_handler();

  $form['account_info'] = array(
    'actions' => array(
      '#type' => 'actions',
    ),
  );

  $form['account_info']['title'] = array(
    '#type' => 'item',
    '#title' => t('Account info'),
  );

  $form['account_info']['api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('API URL'),
    '#default_value' => $smartling_settings->getApiUrl(),
    '#size' => 25,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#description' => t('Set api url. Default: @api_url', array('@api_url' => SMARTLING_DEFAULT_API_URL)),
  );

  $form['account_info']['project_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Project Id'),
    '#default_value' => $smartling_settings->getProjectId(),
    '#size' => 25,
    '#maxlength' => 25,
    '#required' => TRUE,
  );

  $form['account_info']['smartling_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#default_value' => '',
    '#description' => t('Current key: @key', array('@key' => smartling_hide_key($smartling_settings->getKey()))),
    '#size' => 40,
    '#maxlength' => 40,
    '#required' => FALSE,
  );

  $form['account_info']['production_retrieval_type'] = array(
    '#type' => 'radios',
    '#title' => t('Retrieval type'),
    '#default_value' => $smartling_settings->getRetrievalType(),
    '#options' => $smartling_settings->getRetrievalTypeOptions(),
    '#description' => t('Param for download translate.'),
  );

  $target_language_options_list = $smartling_settings->getTargetLanguageOptionsList();
  if (!empty($target_language_options_list)) {
    $form['account_info']['target_locales'] = array(
      '#type' => 'checkboxes',
      '#options' => $target_language_options_list,
      '#title' => t('Target Locales'),
      '#default_value' => $smartling_settings->getTargetLocales(),
      '#prefix' => '<div class="wrap-target-locales">',
    );

    $total = count($target_language_options_list);
    $counter = 0;
    $locales_convert_array = $smartling_settings->getLocalesConvertArray();
    foreach (array_keys($target_language_options_list) as $langcode) {
      $counter++;

      $form['account_info']['target_locales_text_key_' . $langcode] = array(
        '#type' => 'textfield',
        '#title' => '',
        '#title_display' => 'invisible',
        '#default_value' => (isset($locales_convert_array[$langcode]) && ($locales_convert_array[$langcode] != $langcode)) ? $locales_convert_array[$langcode] : '',
        '#size' => 6,
        '#maxlength' => 10,
        '#required' => FALSE,
        '#states' => array(
          'disabled' => array(
            ':input[name="target_locales[' . $langcode . ']"]' => array('checked' => FALSE),
          ),
        ),
      );

      if ($counter == 1) {
        $form['account_info']['target_locales_text_key_' . $langcode]['#prefix'] = '<div class="wrap-target-locales-text-key">';
      }

      if ($counter == $total) {
        $form['account_info']['target_locales_text_key_' . $langcode]['#suffix'] = '</div></div>';
      }
    }
  }
  else {
    $form['account_info']['target_locales'] = array(
      '#type' => 'checkboxes',
      '#options' => array(),
      '#title' => t('Target Locales'),
      '#default_value' => array(),
      '#description' => l(t('At least two languages must be enabled. Please change language settings.'), 'admin/config/regional/language'),
    );
  }

  $form['account_info']['default_language'] = array(
    '#type' => 'item',
    '#title' => t('Default language'),
  );

  $form['account_info']['default_language_markup'] = array(
    '#markup' => '<p>' . t('Site default language: @lang', array('@lang' => language_default()->name)) . '</p>',
    '#suffix' => '<p>' . l(t('Change default language'), 'admin/config/regional/language') . '</p>',
  );

  $form['account_info']['callback_info_title'] = array(
    '#type' => 'item',
    '#title' => t('Callback URL'),
  );

  $form['account_info']['callback_url_use'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Smartling callback: /smartling/callback/%cron_key'),
    '#default_value' => $smartling_settings->getCallbackUrlUse(),
    '#required' => FALSE,
  );

  $form['account_info']['auto_authorize_content_title'] = array(
    '#type' => 'item',
    '#title' => t('Auto authorize'),
  );

  $form['account_info']['auto_authorize_content'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto authorize content'),
    '#default_value' => $smartling_settings->getAutoAuthorizeContent(),
    '#required' => FALSE,
  );

  $form['account_info']['actions']['submit'][] = array(
    '#type' => 'submit',
    '#name' => 'save',
    '#value' => t('Save'),
  );
  $form['account_info']['actions']['submit'][] = array(
    '#type' => 'submit',
    '#name' => 'test_connection',
    '#value' => t('Test connection'),
  );

  $form['#validate'][] = 'smartling_admin_account_info_settings_form_validate';
  $form['#submit'][] = 'smartling_admin_account_info_settings_form_submit';

  return $form;
}

/**
 * Validate smartling account settings form.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_account_info_settings_form_validate(array $form, array &$form_state) {
  $project_id = '';
  if (isset($form_state['values']['api_url']) && !empty($form_state['values']['api_url'])) {
    $api_url = check_plain($form_state['values']['api_url']);
    $status = valid_url($api_url, TRUE);
    if (!$status) {
      drupal_set_message(t('API URL has wrong format'), 'error');
      form_set_error('api_url');
    }
  }

  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    $project_id = trim($form_state['values']['project_id']);
    if (!smartling_project_id_check($project_id)) {
      drupal_set_message(t('Please enter valid Smartling Project Id.'), 'error');
      form_set_error('project_id');
    }
  }

  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    $smartling_key = trim($form_state['values']['smartling_key']);

    if (!smartling_api_key_check($smartling_key)) {
      drupal_set_message(t('Please enter valid Smartling key.'), 'error');
      form_set_error('smartling_key');
    }
  }


  // Target locales validate.
  if (count(array_filter($form_state['values']['target_locales'])) == 0) {
    drupal_set_message(t('At least one locale must be selected'), 'error');
    form_set_error('target_locales');
  }
}

/**
 * Smartling account settings - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_account_info_settings_form_submit(array $form, array &$form_state) {
  // Add default private path.
  if (variable_get('file_private_path', '') == '') {
    $directory = 'sites/default/files/private';
    variable_set('file_private_path', $directory);

    if (!is_dir($directory) && !drupal_mkdir($directory, NULL, TRUE)) {
      // If the directory does not exists and cannot be created.
      drupal_set_message(t('The directory %directory does not exist and could not be created.', array('%directory' => $directory)));
      watchdog('file system', 'The directory %directory does not exist and could not be created.', array('%directory' => $directory), WATCHDOG_ERROR);
    }

    if (is_dir($directory) && !is_writable($directory) && !drupal_chmod($directory)) {
      // If the directory is not writable and cannot be made so.
      drupal_set_message(t('The directory %directory exists but is not writable and could not be made writable.', array('%directory' => $directory)));
      watchdog('file system', 'The directory %directory exists but is not writable and could not be made writable.', array('%directory' => $directory), WATCHDOG_ERROR);
    }
    elseif (is_dir($directory)) {
      // Create private .htaccess file.
      file_create_htaccess($directory);
    }
  }

  $smartling_settings = smartling_settings_get_handler();
  // Account settings.
  if (isset($form_state['values']['api_url'])) {
    $smartling_settings->setApiUrl(check_plain($form_state['values']['api_url']));
  }
  if (isset($form_state['values']['project_id'])) {
    $smartling_settings->setProjectId(check_plain($form_state['values']['project_id']));
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    $smartling_settings->setKey(check_plain(trim($form_state['values']['smartling_key'])));
  }
  // Retrieval type.
  if (isset($form_state['values']['production_retrieval_type']) && !empty($form_state['values']['production_retrieval_type'])) {
    $smartling_settings->setRetrievalType($form_state['values']['production_retrieval_type']);
  }

  // Target locales.
  $smartling_settings->makeTargetLocales($form_state['values']['target_locales']);
  $smartling_settings->makeLocalesConvertArray($form_state['values']);

  // Callback.
  if (isset($form_state['values']['callback_url_use'])) {
    $smartling_settings->setCallbackUrlUse($form_state['values']['callback_url_use']);
  }

  // AutoAuthorizeContent.
  if (isset($form_state['values']['auto_authorize_content'])) {
    $smartling_settings->setAutoAuthorizeContent($form_state['values']['auto_authorize_content']);
  }

  drupal_set_message(t('Account settings saved.'));

  if ($form_state['triggering_element']['#name'] == 'save') {
    drupal_goto(current_path());
  }

  // Test.
  if ($form_state['triggering_element']['#name'] == 'test_connection') {
    $api = drupal_container()->get('smartling.api_wrapper');

    $connection = $api->testConnection($form_state['values']['target_locales']);
    foreach ($connection as $locale => $val) {
      if ($val) {
        drupal_set_message(t('Test connection for locale @s_locale is success.', array('@s_locale' => $locale)));
      }
      else {
        drupal_set_message(t('Test connection for locale @s_locale is fail.', array('@s_locale' => $locale)), 'error');
      }
    }

    drupal_goto(current_path());
  }
}

/**
 * Form for smartling log settings.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling log settings.
 */
function smartling_admin_log_info_settings_form(array $form, array &$form_state) {
  $smartling_settings = smartling_settings_get_handler();

  $form['log_info']['log_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Smartling mode'),
    '#default_value' => $smartling_settings->getLogMode(),
    '#options' => $smartling_settings->getLogModeOptions(),
    '#description' => t('Log ON dy default.'),
  );

  $form['log_info']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'smartling_admin_log_info_settings_form_submit';

  return $form;
}

/**
 * Log Settings - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_log_info_settings_form_submit(array $form, array &$form_state) {
  if (isset($form_state['values']['log_mode'])) {
    $log = smartling_log_get_handler();
    if ($form_state['values']['log_mode'] == FALSE) {
      $log->setMessage('Smartling log OFF')
        ->setConsiderLog(FALSE)
        ->execute();
    }
    elseif ($form_state['values']['log_mode'] == TRUE) {
      $log->setMessage('Smartling log ON')
        ->setConsiderLog(FALSE)
        ->execute();
    }
    smartling_settings_get_handler()->setLogMode($form_state['values']['log_mode']);
  }
  drupal_goto(current_path(), array('fragment' => 'smartling-smartling-log'));
}

/**
 * Wrapper for most kind of entities.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling entity fields settings.
 */
function smartling_admin_entities_translation_settings_form(array $form, array &$form_state) {
  $entity_rows = smartling_admin_entities_translation_crawler();
  //$user_ = drupal_get_form('smartling_admin_user_translation_settings_form');

  $header = array(t('Entity Type'), t('Fields'));
  //$rows = array(array(t('User'), drupal_render($user_)));

  $variables = array(
    'header' => $header,
    'rows' => $entity_rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['entity_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  $form['entity_translation']['actions'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'smartling_admin_entities_translation_settings_form_submit';

  return $form;
}

/**
 * Custom submit handler for entities wrapper form.
 *
 * @see smartling_admin_entities_translation_settings_form()
 */
function smartling_admin_entities_translation_settings_form_submit(array $form, array &$form_state) {
  $input = $form_state['input'];
  $entities = entity_get_info();
  $translate = array();

  // Obtain what fields instances are marked for translation.
  foreach ($input as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $content_type = $parts[0];
        $content_field = $parts[1];

        $translate[$content_type][$content_field] = $content_field;

        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    }

    if (FALSE !== strpos($key, 'subject_swap_')) {
      // And only if set to swap.
      if ($value != 0) {
        $machine_name = substr($key, strlen('subject_swap_'));
        // Do the actual replacement.
        $entity_type = 'comment';
        $bundle = $machine_name;
        $legacy_field = 'subject';

        // Use the Title module to migrate the content.
        if (title_field_replacement_toggle($entity_type, $bundle, $legacy_field)) {
          $operations[] = array(
            'title_field_replacement_batch',
            array(
              $entity_type,
              $bundle,
              $legacy_field,
            ),
          );
          // Add in config.
          $translate[$machine_name]['subject_field'] = 'subject_field';

          $field = field_info_field('subject_field');
          $field['translatable'] = 1;
          $operations[] = array('field_update_field', array($field));
        }
      }
    }
  }

  // Define parent bundle for those fields.
  $_translate = array();
  $machine_names = array_keys($translate);
  foreach ($entities as $entity => $definition) {
    $bundles = array_keys($definition['bundles']);

    foreach ($machine_names as $name) {
      if (in_array($name, $bundles)) {
        $_translate[$entity][$name] = $translate[$name];
      }
    }
  }

  // Reset default state, since all disabled checkboxes will be ignored
  // and therefore, not updated.
  // @todo
  // This is to be refactored, since it's annoying to add a line for each
  // entity processor.
  smartling_settings_get_handler()->{'userSetFieldsSettings'}(array());
  smartling_settings_get_handler()->{'commentSetFieldsSettings'}(array());
  smartling_settings_get_handler()->{'fieldCollectionSetFieldsSettings'}(array());

  // Save the settings, considering entity bundle, to know which
  // update method to be called.
  foreach ($_translate as $k => $v) {
    $method = NULL;
    switch ($k) {
      case 'user':
        $method = 'user';
        break;
      case 'comment':
        $method = 'comment';
        break;
      case 'field_collection_item':
        $method = 'fieldCollection';
        break;
    }

    if ($method) {
      smartling_settings_get_handler()->{$method . 'SetFieldsSettings'}($v);
    }
  }

  drupal_set_message(t('Entities settings updated.'));

  $log = smartling_log_get_handler();
  $log->setMessage('Smartling entities and fields have been updated.')
    ->setConsiderLog(FALSE)
    ->execute();

  if (!empty($operations)) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations,
    );

    $redirect = url('admin/config/regional/smartling', array(
      'absolute' => TRUE,
      'fragment' => 'smartling-comment-fields',
    ));

    batch_set($batch);
    batch_process($redirect);
  }
}

/**
 * Aggregate various types of entities and their fields.
 *
 * @return array
 *   Set of rows ready to be placed into table form element.
 *   First element is the entity name, second - checkboxes for each
 *   translatable fields for this entity.
 */
function smartling_admin_entities_translation_crawler() {
  // Entities we do not want to parse.
  $exclude = array(
    'node',
    'taxonomy_term',
    'taxonomy_vocabulary',
    'i18n_translation',
    'smartling_interface_entity',
    'file',
    'smartling_entity_data',
  );

  $translatable_field_types = smartling_get_translatable_field_types();
  $entities = entity_get_info();
  $rows = array();

  // Loop thru each entity type.
  foreach ($entities as $name => $definition) {
    if (in_array($name, $exclude)) {
      continue;
    }

    $bundles = array_keys($definition['bundles']);
    // Loop thru each entity bundle.
    foreach ($bundles as $bundle) {
      $field_instances = field_info_instances($name, $bundle);

      // Loop thru each field instance of the bundle.
      foreach ($field_instances as $field) {
        $field_label = $field['label'];
        $field_machine_name = $field['field_name'];
        $field_type = $field['widget']['type'];

        if (in_array($field_type, $translatable_field_types)) {
          $key = $bundle . '_SEPARATOR_' . $field_machine_name;
          $fr_fields[$key] = array(
            '#type' => 'checkbox',
            '#title' => check_plain($field_label),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $bundle . '-separator-' . $field_machine_name),
              'name' => $bundle . '_SEPARATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $bundle . '-separator-' . $field_machine_name,
          );

          // Comments subject field need special treatment. i.e. some fields
          // replaced instead of standard ones.
          $comment_key = $bundle . '_SEPARATOR_' . 'subject_field';

          if ($name == 'comment' && !isset($fr_fields[$comment_key])) {
            $fr_fields[$comment_key] = array(
              '#type' => 'checkbox',
              '#title' => t('Subject (Note: field will be created.)'),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $bundle . '-separator-subject'),
                'name' => 'subject_swap_' . $bundle,
                'class' => array('field'),
              ),
            );
          }

          $method = NULL;
          switch ($name) {
            case 'user':
              $method = 'user';
              break;
            case 'comment':
              $method = 'comment';
              break;
            case 'field_collection_item':
              $method = 'fieldCollection';
              break;
          }

          if ($method) {
            $translate_fields = smartling_settings_get_handler()->{$method . 'GetFieldsSettings'}();

            $is_in_conf = !empty($translate_fields) && isset($translate_fields[$bundle][$field_machine_name]);

            if ($is_in_conf) {
              $fr_fields[$key]['#attributes']['checked'] = 'checked';
            }
          }
        }
      }

      $rows[] = array($bundle, drupal_render($fr_fields));
      $fr_fields = NULL;
    }
  }

  return $rows;
}

/**
 * Form for smartling node fields settings.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling node fields settings.
 */
function smartling_admin_node_translation_settings_form(array $form, array &$form_state) {
  $raw_types = node_type_get_types();
  $node_translate_fields = smartling_settings_get_handler()->nodeGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['node_translation'] = array(
    'actions' => array(
      '#type' => 'actions',
    ),
  );

  $form['node_translation']['title'] = array(
    '#type' => 'item',
    '#title' => t('Which content types do you want to translate?'),
  );

  $rows = array();
  global $_fix_field_settings;

  foreach ($raw_types as $value) {
    if (smartling_supported_type('node', $value->type)) {
      $fr_tt['from'] = array(
        '#type' => 'item',
        '#title' => t('Fields method'),
      );
      if (smartling_nodes_method($value->type)) {
        $fr_tt['from']['#title'] = t('Nodes method');
      }

      $fr_fields = array();

      foreach (field_info_instances('node', $value->type) as $field) {
        $field_label = $field['label'];
        $field_machine_name = $field['field_name'];
        $field_type = $field['widget']['type'];
        if (array_search($field_type, $translatable_field_types)) {
          $fr_fields[$field_machine_name] = array(
            '#type' => 'checkbox',
            '#title' => check_plain($field_label),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $value->type . '-separator-' . $field_machine_name),
              'name' => $value->type . '_SEPARATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $value->type . '-separator-' . $field_machine_name,
          );

          $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;

          if ($is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
            // Error in field settings.
            if (smartling_nodes_method($value->type) && smartling_field_is_translatable_by_field_name($field_machine_name, 'node') && !in_array($field_machine_name, $_fix_field_settings) && !smartling_is_title_module_field($field_machine_name)) {
              $_fix_field_settings[$field_machine_name] = '<b>' . $field_machine_name . '</b>';
            }
          }
        }
      }

      if (!isset($fr_fields['title_field']) && smartling_fields_method($value->type)) {
        $fr_fields['title_field'] = array(
          '#type' => 'checkbox',
          '#title' => t('Title (Note: field will be created.)'),
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-separator-title_field'),
            'name' => 'title_swap_' . $value->type,
            'class' => array('field'),
          ),
        );

        $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type]['title_field'])) ? TRUE : FALSE;
        if ($is_in_conf) {
          $fr_fields['title_field']['#attributes']['checked'] = 'checked';
        }
      }
      // Fix double title after change translate method.
      if (!isset($fr_fields['title_field'])) {
        // Fake field title ($node->title) for nodes method.
        if (smartling_nodes_method($value->type)) {
          $field_machine_name = 'title_property_field';
          $fr_fields[$field_machine_name] = array(
            '#type' => 'checkbox',
            '#title' => t('Title'),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $value->type . '-separator-title_property_field' . $field_machine_name),
              'name' => $value->type . '_SEPARATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $value->type . '-separator-' . $field_machine_name,
          );

          $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;
          if ($is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
          }
        }
      }
    }
    else {
      $options = array(
        0 => t('- Select Method -'),
        2 => t('Nodes method'),
        1 => t('Fields method'),
      );

      $fr_tt['method'][$value->type] = array(
        '#type' => 'select',
        '#title' => t('Translation Type'),
        '#title_display' => 'invisible',
        '#options' => $options,
        '#required' => FALSE,
        '#default_value' => 0,
        '#attributes' => array(
          'id' => array('edit-form-item-' . $value->type . '-TT-' . $value->type),
          'name' => $value->type . '_TT_' . $value->type,
          'class' => array('content-type'),
        ),
      );

      $fr_fields = array();
    }
    $rows[$value->type] = array(
      array(
        'data' => check_plain($value->name),
        'width' => '20%',
      ),
      array(
        'data' => drupal_render($fr_tt),
        'width' => '20%',
      ),
      array(
        'data' => drupal_render($fr_fields),
        'width' => '60%',
      ),
    );
    unset($fr_tt);
    unset($fr_fields);
  }

  $header = array(t('Entity Type'), t('Translation Type'), t('Fields'));

  $variables = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['node_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  foreach (array_keys($node_translate_fields) as $content_type) {
    $form['node_translation']['types']['#default_value'][$content_type] = 1;
  }

  $form['node_translation']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'smartling_admin_node_translation_settings_form_submit';

  return $form;
}

/**
 * Node Translation Settings - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_node_translation_settings_form_submit(array $form, array &$form_state) {
  $smartling_settings = smartling_settings_get_handler();
  $log = smartling_log_get_handler();
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $content_type = $parts[0];
        $content_field = $parts[1];

        $translate[$content_type][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update).
        if (smartling_fields_method($content_type)) {
          $field = field_info_field($content_field);
          $field['translatable'] = 1;
          field_update_field($field);
        }
      }
    }
    // END:  Selected Content Types and Fields.
    // Look for Selected Translation Type.
    if (FALSE !== strpos($key, '_TT_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_TT_', $key);
        $content_type = $parts[0];
        if ($value == 2) {
          variable_set('language_content_type_' . $content_type, SMARTLING_NODES_METHOD_KEY);
        }
        elseif ($value == 1) {
          variable_set('language_content_type_' . $content_type, SMARTLING_FIELDS_METHOD_KEY);
        }
      }
    }

    // Look for any nodes we need to do the Title swap for.
    if (FALSE !== strpos($key, 'title_swap_')) {
      // And only if set to swap.
      if ($value != 0) {
        $content_type = substr($key, strlen('title_swap_'));

        // Do the actual title replacement.
        $entity_type = 'node';
        $bundle = $content_type;
        $legacy_field = 'title';

        // Use the Title module to migrate the content.
        if (title_field_replacement_toggle($entity_type, $bundle, $legacy_field)) {
          $operations[] = array(
            'title_field_replacement_batch',
            array(
              $entity_type,
              $bundle,
              $legacy_field,
            ),
          );
          // Add in config.
          $translate[$content_type]['title_field'] = 'title_field';
          $field = field_info_field('title_field');
          $field['translatable'] = 1;
          $operations[] = array('field_update_field', array($field));
        }
      }
    }
  }

  $smartling_settings->nodeSetFieldsSettings($translate);
  drupal_set_message(t('Your content types have been updated.'));
  $log->setMessage('Smartling content types and fields have been updated.')
    ->setConsiderLog(FALSE)
    ->execute();

  $redirect = url('admin/config/regional/smartling', array(
    'absolute' => TRUE,
    'fragment' => 'smartling-content-types-and-fields',
  ));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations,
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }
}

/**
 * Form for smartling taxonomy fields settings.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling taxonomy fields settings.
 */
function smartling_admin_taxonomy_translation_settings_form(array $form, array &$form_state) {
  $raw_types = taxonomy_get_vocabularies();
  $term_translate_fields = smartling_settings_get_handler()->taxonomyTermGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['taxonomy_translation'] = array(
    'actions' => array(
      '#type' => 'actions',
    ),
  );

  $form['taxonomy_translation']['title'] = array(
    '#type' => 'item',
    '#title' => t('Which vocabularies do you want to translate?'),
  );

  $rows = array();
  global $_fix_field_settings;

  foreach ($raw_types as $vocabulary) {
    if (smartling_supported_type('taxonomy_term', $vocabulary->machine_name)) {
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
      switch ($vocabulary_mode) {
        case I18N_MODE_TRANSLATE:
          $voc_title = t('Translate method');
          break;

        case I18N_MODE_LOCALIZE:
          $voc_title = t('Localize method');
          break;

        case I18N_MODE_LANGUAGE:
        case I18N_MODE_NONE:
        default:
          $voc_title = '-';
          break;
      }

      $fr_fields = array();
      $fr_tt['from'] = array(
        '#type' => 'item',
        '#title' => t('@type', array('@type' => $voc_title)),
      );

      foreach (field_info_instances('taxonomy_term', $vocabulary->machine_name) as $field) {
        $field_label = $field['label'];
        $field_machine_name = $field['field_name'];
        $field_type = $field['widget']['type'];
        if (array_search($field_type, $translatable_field_types)) {
          $fr_fields[$field_machine_name] = array(
            '#type' => 'checkbox',
            '#title' => check_plain($field_label),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name),
              'name' => $vocabulary->machine_name . '_SEPARATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name,
          );

          $is_in_conf = (!empty($term_translate_fields) && isset($term_translate_fields[$vocabulary->machine_name][$field_machine_name])) ? TRUE : FALSE;

          if ($is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
            // Error in field settings.
            if (($vocabulary_mode == I18N_MODE_TRANSLATE) && smartling_field_is_translatable_by_field_name($field_machine_name, 'taxonomy_term') && isset($term_translate_fields[$vocabulary->machine_name][$field['field_name']]) && !in_array($field_machine_name, $_fix_field_settings) && !smartling_is_title_module_field($field_machine_name)) {
              $_fix_field_settings[$field_machine_name] = '<b>' . $field_machine_name . '</b>';
            }
          }
        }
      }
      // Fix double field after change translate method.
      $term_field_names = array(
        'description' => 'description',
        'name' => 'name',
      );
      foreach ($term_field_names as $property_name) {
        if (isset($fr_fields[$property_name . '_field'])) {
          unset($term_field_names[$property_name]);
        }
      }

      if ($vocabulary_mode == I18N_MODE_LOCALIZE) {
        $term_field_names = array('description', 'name');
        foreach ($term_field_names as $term_field_name) {
          if (!isset($fr_fields[$term_field_name . '_field'])) {
            $fr_fields[$term_field_name . '_field'] = array(
              '#type' => 'checkbox',
              '#title' => t('@name (Note: field will be created.)', array('@name' => ucfirst($term_field_name))),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $vocabulary->machine_name . '-separator-' . $term_field_name),
                'name' => $term_field_name . '_swap_' . $vocabulary->machine_name,
                'class' => array('field'),
              ),
            );

            $is_in_conf = (!empty($term_translate_fields) && isset($term_translate_fields[$vocabulary->machine_name][$term_field_name . '_field'])) ? TRUE : FALSE;
            if ($is_in_conf) {
              $fr_fields[$term_field_name . '_field']['#attributes']['checked'] = 'checked';
            }
          }
        }
      }
      // Fake field for taxonomy name and description ($term->name).
      if ($vocabulary_mode == I18N_MODE_TRANSLATE) {
        foreach ($term_field_names as $term_field) {
          $field_machine_name = $term_field . '_property_field';
          $fr_fields[$field_machine_name] = array(
            '#type' => 'checkbox',
            '#title' => t('@name', array('@name' => ucfirst($term_field))),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name),
              'name' => $vocabulary->machine_name . '_SEPARATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name,
          );

          $is_in_conf = (!empty($term_translate_fields) && isset($term_translate_fields[$vocabulary->machine_name][$field_machine_name])) ? TRUE : FALSE;

          if ($is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
          }
        }
      }
    }
    else {
      $options = array(
        0 => t('- Select Method -'),
        2 => t('Translate method'),
        1 => t('Localize method'),
      );

      $fr_tt['method'][$vocabulary->machine_name] = array(
        '#type' => 'select',
        '#title' => t('Translation Type'),
        '#title_display' => 'invisible',
        '#options' => $options,
        '#required' => FALSE,
        '#default_value' => 0,
        '#attributes' => array(
          'id' => array('edit-form-item-' . $vocabulary->machine_name . '-TT-' . $vocabulary->machine_name),
          'name' => $vocabulary->machine_name . '_TT_' . $vocabulary->machine_name,
          'class' => array('content-type'),
        ),
      );

      $fr_fields = array();
    }
    $rows[$vocabulary->machine_name] = array(
      array(
        'data' => $vocabulary->name,
        'width' => '20%',
      ),
      array(
        'data' => drupal_render($fr_tt),
        'width' => '20%',
      ),
      array(
        'data' => drupal_render($fr_fields),
        'width' => '60%',
      ),
    );
    unset($fr_tt);
    unset($fr_fields);
  }

  $header = array(t('Vocabularies'), t('Translation Type'), t('Fields'));

  $variables = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['taxonomy_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  foreach (array_keys($term_translate_fields) as $content_type) {
    $form['taxonomy_translation']['types']['#default_value'][$content_type] = 1;
  }

  $form['taxonomy_translation']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'smartling_admin_taxonomy_translation_settings_form_submit';

  return $form;
}

/**
 * Taxonomy Translation Settings - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_taxonomy_translation_settings_form_submit(array $form, array &$form_state) {
  $log = smartling_log_get_handler();
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $machine_name = $parts[0];
        $content_field = $parts[1];

        $translate[$machine_name][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update).
        $vocabulary = taxonomy_vocabulary_machine_name_load($machine_name);
        $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
        if ($vocabulary_mode == I18N_MODE_LOCALIZE) {
          $field = field_info_field($content_field);
          $field['translatable'] = 1;
          field_update_field($field);
        }
      }
    }
    // END:  Selected Content Types and Fields.
    // Look for Selected Translation Type.
    if (FALSE !== strpos($key, '_TT_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_TT_', $key);
        $machine_name = $parts[0];
        $vocabulary = taxonomy_vocabulary_machine_name_load($machine_name);
        if ($value == 2) {
          $vocabulary->i18n_mode = I18N_MODE_TRANSLATE;
        }
        elseif ($value == 1) {
          $vocabulary->i18n_mode = I18N_MODE_LOCALIZE;
        }
        taxonomy_vocabulary_save($vocabulary);
      }
    }

    // Look for any terms we need to do the swap for.
    $term_field_names = array('description', 'name');
    foreach ($term_field_names as $term_field_name) {
      if (FALSE !== strpos($key, $term_field_name . '_swap_')) {
        // And only if set to swap.
        if ($value != 0) {
          $machine_name = substr($key, strlen($term_field_name . '_swap_'));

          // Do the actual replacement.
          $entity_type = 'taxonomy_term';
          $legacy_field = $term_field_name;

          // Use the Title module to migrate the content.
          if (title_field_replacement_toggle($entity_type, $machine_name, $legacy_field)) {
            $operations[] = array(
              'title_field_replacement_batch',
              array(
                $entity_type,
                $machine_name,
                $legacy_field,
              ),
            );
            // Add in config.
            $translate[$machine_name][$term_field_name . '_field'] = $term_field_name . '_field';
            $field = field_info_field($term_field_name . '_field');
            $field['translatable'] = 1;
            $operations[] = array('field_update_field', array($field));
          }
        }
      }
    }
  }

  smartling_settings_get_handler()->taxonomyTermSetFieldsSettings($translate);
  drupal_set_message(t('Your taxonomy vocabularies have been updated.'));

  $log->setMessage('Smartling taxonomy vocabularies and fields have been updated.')
    ->setConsiderLog(FALSE)
    ->execute();

  $redirect = url('admin/config/regional/smartling', array(
    'absolute' => TRUE,
    'fragment' => 'smartling-taxonomy-vocabularies-and-fields',
  ));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations,
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }
}

/**
 * Form for smartling comments fields settings.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling comments fields settings.
 */
function smartling_admin_comments_translation_settings_form(array $form, array &$form_state) {
  /*$raw_types = node_type_get_types();
  $comment_translate_fields = smartling_settings_get_handler()->commentGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['comments_translation'] = array(
    'actions' => array(
      '#type' => 'actions',
    ),
  );

  $form['comments_translation']['title'] = array(
    '#type' => 'item',
    '#title' => t('Which comments do you want to translate?'),
  );

  $rows = array();

  foreach ($raw_types as $node_type) {
    $bundle = comment_node_type_load($node_type->type);
    if (smartling_supported_type('comment', $bundle)) {

      $fr_fields = array();
      $fr_tt['from'] = array(
        '#type' => 'item',
        '#title' => t('Fields method'),
      );

      foreach (field_info_instances('comment', $bundle) as $field) {
        $field_label = $field['label'];
        $field_machine_name = $field['field_name'];
        $field_type = $field['widget']['type'];
        if (array_search($field_type, $translatable_field_types)) {
          $fr_fields[$field_machine_name] = array(
            '#type' => 'checkbox',
            '#title' => check_plain($field_label),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $bundle . '-separator-' . $field_machine_name),
              'name' => $bundle . '_SEPARATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $bundle . '-separator-' . $field_machine_name,
          );

          $is_in_conf = (!empty($comment_translate_fields) && isset($comment_translate_fields[$bundle][$field_machine_name])) ? TRUE : FALSE;

          if ($is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
          }
        }
      }

      if (!isset($fr_fields['subject_field'])) {
        $fr_fields['subject_field'] = array(
          '#type' => 'checkbox',
          '#title' => t('Subject (Note: field will be created.)'),
          '#attributes' => array(
            'id' => array('edit-form-item-' . $bundle . '-separator-subject'),
            'name' => 'subject_swap_' . $bundle,
            'class' => array('field'),
          ),
        );

        $is_in_conf = (!empty($comment_translate_fields) && isset($comment_translate_fields[$bundle]['subject_field'])) ? TRUE : FALSE;
        if ($is_in_conf) {
          $fr_fields['subject_field']['#attributes']['checked'] = 'checked';
        }
      }
    }
    else {
      $fr_tt['from'] = array(
        '#type' => 'item',
        '#title' => t('Not support'),
      );
      $fr_fields = array();
    }
    $rows[$bundle] = array(
      array('data' => $bundle, 'width' => '20%'),
      array('data' => drupal_render($fr_tt), 'width' => '20%'),
      array('data' => drupal_render($fr_fields), 'width' => '60%'),
    );
    unset($fr_tt);
    unset($fr_fields);
  }

  $header = array(t('Content Type'), t('Translation Type'), t('Fields'));

  $variables = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['comments_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  foreach (array_keys($comment_translate_fields) as $content_type) {
    $form['comments_translation']['types']['#default_value'][$content_type] = 1;
  }

  $form['comments_translation']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'smartling_admin_comments_translation_settings_form_submit';

  return $form;*/
}

/**
 * Comments Translation Settings - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_comments_translation_settings_form_submit(array $form, array &$form_state) {
  /*$log = smartling_log_get_handler();
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $machine_name = $parts[0];
        $content_field = $parts[1];

        $translate[$machine_name][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update).
        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    }
    // END:  Selected Content Types and Fields.
    // Look for any comments we need to do the swap for.
    if (FALSE !== strpos($key, 'subject_swap_')) {
      // And only if set to swap.
      if ($value != 0) {
        $machine_name = substr($key, strlen('subject_swap_'));

        // Do the actual replacement.
        $entity_type = 'comment';
        $bundle = $machine_name;
        $legacy_field = 'subject';

        // Use the Title module to migrate the content.
        if (title_field_replacement_toggle($entity_type, $bundle, $legacy_field)) {
          $operations[] = array(
            'title_field_replacement_batch',
            array(
              $entity_type,
              $bundle,
              $legacy_field,
            ),
          );
          // Add in config.
          $translate[$machine_name]['subject_field'] = 'subject_field';

          $field = field_info_field('subject_field');
          $field['translatable'] = 1;
          $operations[] = array('field_update_field', array($field));
        }
      }
    }
  }

  smartling_settings_get_handler()->commentSetFieldsSettings($translate);
  drupal_set_message(t('Your comments have been updated.'));

  $log->setMessage('Smartling comments and fields have been updated.')
    ->setConsiderLog(FALSE)
    ->execute();

  $redirect = url('admin/config/regional/smartling', array(
    'absolute' => TRUE,
    'fragment' => 'smartling-comment-fields',
  ));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations,
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }*/
}

/**
 * Form for smartling user fields settings.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling user fields settings.
 */
function smartling_admin_user_translation_settings_form(array $form, array &$form_state) {
  /*$user_translate_fields = smartling_settings_get_handler()->userGetFieldsSettings();
  $translatable_field_types = smartling_get_translatable_field_types();
  $bundle = 'user';

  foreach (field_info_instances('user', $bundle) as $field) {
    $field_label = $field['label'];
    $field_machine_name = $field['field_name'];
    $field_type = $field['widget']['type'];

    if (array_search($field_type, $translatable_field_types)) {
      $form[$field_machine_name] = array(
        '#type' => 'checkbox',
        '#title' => check_plain($field_label),
        '#attributes' => array(
          'id' => array('edit-form-item-' . $bundle . '-separator-' . $field_machine_name),
          'name' => $bundle . '_SEPARATOR_' . $field_machine_name,
          'class' => array('field'),
        ),
        '#id' => 'edit-form-item-' . $bundle . '-separator-' . $field_machine_name,
      );

      $is_in_conf = (!empty($user_translate_fields) && isset($user_translate_fields[$bundle][$field_machine_name])) ? TRUE : FALSE;

      if ($is_in_conf) {
        $form[$field_machine_name]['#attributes']['checked'] = 'checked';
      }
    }
  }

  return $form;*/
}

/**
 * Users Translation Settings - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_admin_users_translation_settings_form_submit(array $form, array &$form_state) {
  /*$log = smartling_log_get_handler();
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $machine_name = $parts[0];
        $content_field = $parts[1];

        $translate[$machine_name][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update).
        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    }
    // END: Fields.
  }

  smartling_settings_get_handler()->userSetFieldsSettings($translate);
  drupal_set_message(t('Your users have been updated.'));

  $log->setMessage('Smartling users and fields have been updated.')
    ->setConsiderLog(FALSE)
    ->execute();

  $redirect = url('admin/config/regional/smartling', array(
    'absolute' => TRUE,
    'fragment' => 'smartling-user-fields',
  ));
  $form_state['redirect'] = $redirect;*/
}

/*function smartling_admin_field_collection_settings_form(array $form, array &$form_state) {
  $instances = field_info_instances();

  $raw_types = node_type_get_types();
  $node_translate_fields = smartling_settings_get_handler()->fieldCollectionGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['node_translation'] = array(
    'actions' => array(
      '#type' => 'actions',
    ),
  );

  $form['node_translation']['title'] = array(
    '#type' => 'item',
    '#title' => t('Which content types do you want to translate?'),
  );

  $rows = array();
  global $_fix_field_settings;


  foreach ($instances as $entity_type => $type_bundles) {
    foreach ($type_bundles as $bundle => $bundle_instances) {
      foreach ($bundle_instances as $field_name => $instance) {
        if ($entity_type != 'field_collection_item') {
          continue;
        }

        $fr_tt['from'] = array(
          '#type' => 'item',
          '#title' => t('Fields method'),
        );
        if (smartling_nodes_method($bundle)) {
          $fr_tt['from']['#title'] = t('Nodes method');
        }

        $fr_fields = array();

        foreach (field_info_instances($entity_type, $bundle) as $field) {
          $field_label = $field['label'];
          $field_machine_name = $field['field_name'];
          $field_type = $field['widget']['type'];
          if (array_search($field_type, $translatable_field_types) || TRUE) {
            $fr_fields[$field_machine_name] = array(
              '#type' => 'checkbox',
              '#title' => check_plain($field_label),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $bundle . '-separator-' . $field_machine_name),
                'name' => $bundle . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $bundle . '-separator-' . $field_machine_name,
            );

            $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$bundle][$field_machine_name])) ? TRUE : FALSE;

            if ($is_in_conf) {
              $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
              // Error in field settings.
              if (smartling_nodes_method($bundle) && smartling_field_is_translatable_by_field_name($field_machine_name, 'node') && !in_array($field_machine_name, $_fix_field_settings) && !smartling_is_title_module_field($field_machine_name)) {
                $_fix_field_settings[$field_machine_name] = '<b>' . $field_machine_name . '</b>';
              }
            }
          }
        }

        if (!isset($fr_fields['title_field']) && smartling_fields_method($bundle)) {
          $fr_fields['title_field'] = array(
            '#type' => 'checkbox',
            '#title' => t('Title (Note: field will be created.)'),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $bundle . '-separator-title_field'),
              'name' => 'title_swap_' . $bundle,
              'class' => array('field'),
            ),
          );

          $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$bundle]['title_field'])) ? TRUE : FALSE;
          if ($is_in_conf) {
            $fr_fields['title_field']['#attributes']['checked'] = 'checked';
          }
        }
        // Fix double title after change translate method.
        if (!isset($fr_fields['title_field'])) {
          // Fake field title ($node->title) for nodes method.
          if (smartling_nodes_method($bundle)) {
            $field_machine_name = 'title_property_field';
            $fr_fields[$field_machine_name] = array(
              '#type' => 'checkbox',
              '#title' => t('Title'),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $bundle . '-separator-title_property_field' . $field_machine_name),
                'name' => $bundle . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $bundle . '-separator-' . $field_machine_name,
            );

            $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$bundle][$field_machine_name])) ? TRUE : FALSE;
            if ($is_in_conf) {
              $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
            }
          }
        }

        $rows[$bundle] = array(
          array(
            'data' => check_plain($bundle),
            'width' => '20%',
          ),
          array(
            'data' => drupal_render($fr_tt),
            'width' => '20%',
          ),
          array(
            'data' => drupal_render($fr_fields),
            'width' => '60%',
          ),
        );
        unset($fr_tt);
        unset($fr_fields);
      }
    }
  }


  $header = array(t('Content Type'), t('Translation Type'), t('Fields'));

  $variables = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['node_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  foreach (array_keys($node_translate_fields) as $content_type) {
    $form['node_translation']['types']['#default_value'][$content_type] = 1;
  }

  $form['node_translation']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'smartling_admin_field_collection_settings_form_submit';

  return $form;

}*/

function smartling_admin_field_collection_settings_form_submit(array $form, array &$form_state) {
  $smartling_settings = smartling_settings_get_handler();
  $log = smartling_log_get_handler();
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate.
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $content_type = $parts[0];
        $content_field = $parts[1];

        $translate[$content_type][$content_field] = $content_field;

      }
    }
  }

  $smartling_settings->fieldCollectionSetFieldsSettings($translate);
  drupal_set_message(t('Your content types have been updated.'));
  $log->setMessage('Smartling content types and fields have been updated.')
    ->setConsiderLog(FALSE)
    ->execute();

  $redirect = url('admin/config/regional/smartling', array(
    'absolute' => TRUE,
    'fragment' => 'smartling-content-types-and-fields',
  ));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations,
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }
}

/**
 * Wrap form in fieldset.
 *
 * @param array $form
 *   FAPI array.
 * @param string $title
 *   Title for fieldset.
 *
 * @return array
 *   Return FAPI array.
 */
function smartling_wrap_in_fieldset(array $form, $title) {
  return array(
    '#type' => 'fieldset',
    '#group' => 'smartling',
    '#title' => $title,
    '#attributes' => array(
      'class' => array('smartling-' . strtolower(str_replace(' ', '-', $title))),
      'id' => array('smartling-' . strtolower(str_replace(' ', '-', $title))),
    ),
    'children' => $form,
  );
}

/**
 * Hide last 10 characters in string.
 *
 * @param string $key
 *   Smartling key.
 *
 * @return string
 *   Return smartling key without 10 last characters.
 */
function smartling_hide_key($key = '') {
  return substr($key, 0, -10) . str_repeat("*", 10);
}

/**
 * Check api key.
 *
 * @param string $key
 *   Api key.
 *
 * @return string
 *   Return checked api key.
 */
function smartling_api_key_check($key) {
  return preg_match("/^[a-z0-9]{8}(?:-[a-z0-9]{4}){3}-[a-z0-9]{12}$/", $key);
}

/**
 * Check project id.
 *
 * @param string $project_id
 *   Project id.
 *
 * @return string
 *   Return checked project id.
 */
function smartling_project_id_check($project_id) {
  return preg_match("/^[a-z0-9]{9}$/", $project_id);
}

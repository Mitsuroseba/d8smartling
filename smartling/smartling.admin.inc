<?php

/**
 * @file
 * Admin forms and other for Smartling module administration section.
 */
function smartling_admin_configuration_view() {
  // add ajax library
  drupal_add_library('system', 'drupal.ajax');

  $output['smartling'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'smartling') . '/js/smartling_admin.js'),
      'css' => array(drupal_get_path('module', 'smartling') . '/css/smartling_admin.css'),
    ),
  );

  $accaunt_info = drupal_get_form('smartling_admin_account_info_settings_form');
  $node_translation_settings = drupal_get_form('smartling_admin_node_translation_settings_form');
  $taxonomy_translation_settings = drupal_get_form('smartling_admin_taxonomy_translation_settings_form');
  $comments_translation_settings = drupal_get_form('smartling_admin_comments_translation_settings_form');
  $users_translation_settings = drupal_get_form('smartling_admin_users_translation_settings_form');
  $log_info = drupal_get_form('smartling_admin_log_info_settings_form');

  $output['smartling'][] = smartling_wrap_in_fieldset($accaunt_info, 'Account info');
  $output['smartling'][] = smartling_wrap_in_fieldset($node_translation_settings, 'Content types and fields');
  $output['smartling'][] = smartling_wrap_in_fieldset($taxonomy_translation_settings, 'Taxonomy vocabularies and fields');
  $output['smartling'][] = smartling_wrap_in_fieldset($comments_translation_settings, 'Comment fields');
  $output['smartling'][] = smartling_wrap_in_fieldset($users_translation_settings, 'User fields');
  $output['smartling'][] = smartling_wrap_in_fieldset($log_info, 'Smartling log');
  if (module_exists('smartling_load_test')) {
    $dev_settings = drupal_get_form('smartling_admin_dev_settings_form');
    $output['smartling'][] = smartling_wrap_in_fieldset($dev_settings, 'Dev settings');
  }
  return $output;
}

/**
 * Account info form
 */
function smartling_admin_account_info_settings_form($form, &$form_state) {
  $smartling_settings = smartling_settings_get_handler();

  $form['accaunt_info'] = array(
    '#type' => 'item',
    '#title' => t('Account info'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'accaunt_info',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_account_info_settings_form_submit'),
    '#validate' => array('smartling_admin_account_info_settings_form_validate'),
  );

  $form['accaunt_info']['api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('API URL'),
    '#default_value' => $smartling_settings->getApiUrl(),
    '#size' => 25,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#description' => t('Set api url. Default: ' . SMARTLING_DEFAULT_API_URL),
  );

  $form['accaunt_info']['project_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Project Id'),
    '#default_value' => $smartling_settings->getProjectId(),
    '#size' => 25,
    '#maxlength' => 25,
    '#required' => TRUE,
  );

  $form['accaunt_info']['smartling_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#default_value' => '',
    '#description' => t('Current key:') . ' ' . check_plain(smartling_hide_key($smartling_settings->getKey())),
    '#size' => 40,
    '#maxlength' => 40,
    '#required' => FALSE,
  );

  $form['accaunt_info']['production_retrieval_type'] = array(
    '#type' => 'radios',
    '#title' => t('Retrieval type'),
    '#default_value' => $smartling_settings->getRetrievalType(),
    '#options' => $smartling_settings->getRetrievalTypeOptions(),
    '#description' => t('Param for download translate.'),
  );

  $target_language_options_list = $smartling_settings->getTargetLanguageOptionsList();
  if (!empty($target_language_options_list)) {
    $form['accaunt_info']['target_locales'] = array(
      '#type' => 'checkboxes',
      '#options' => $target_language_options_list,
      '#title' => t('Target Locales'),
      '#default_value' => $smartling_settings->getTargetLocales(),
      '#prefix' => '<div class="wrap-target-locales">',
    );

    $total = count($target_language_options_list);
    $counter = 0;
    $locales_convert_array = $smartling_settings->getLocalesConvertArray();
    foreach ($target_language_options_list as $langcode => $lang_name) {
      $counter++;
      if ($counter == 1) {
        $form['accaunt_info']['target_locales_text_key_' . $langcode] = array(
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
          '#prefix' => '<div class="wrap-target-locales-text-key">',
        );
      }
      else {
        $form['accaunt_info']['target_locales_text_key_' . $langcode] = array(
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
      }

      if ($counter == $total) {
        $form['accaunt_info']['target_locales_text_key_' . $langcode] = array(
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
          '#suffix' => '</div></div>',
        );
      }
      // For one language
      if ($total == 1) {
        $form['accaunt_info']['target_locales_text_key_' . $langcode]['#prefix'] = '<div class="wrap-target-locales-text-key">';
      }
    }
  }
  else {
    $form['accaunt_info']['target_locales'] = array(
      '#type' => 'checkboxes',
      '#options' => array(),
      '#title' => t('Target Locales'),
      '#default_value' => array(),
      '#description' => l(t('At least two languages must be enabled. Please change language settings.'), url('admin/config/regional/language')),
    );
  }

  $form['accaunt_info']['default_language'] = array(
    '#type' => 'item',
    '#title' => t('Default language'),
  );

  $form['accaunt_info']['default_language_markup'] = array(
    '#markup' => '<p>' . t('Site default language: @lang', array('@lang' => check_plain(language_default()->name))) . '</p>',
    '#suffix' => '<p>' . l(t('Change default language'), 'admin/config/regional/language') . '</p>',
  );

  $form['accaunt_info']['callback_info_title'] = array(
    '#type' => 'item',
    '#title' => t('Callback URL'),
  );

  $form['accaunt_info']['callback_url_use'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Smartling callback: /smartling/callback/%cron_key'),
    '#default_value' => $smartling_settings->getCallbackUrlUse(),
    //'#suffix' => '<div>' . t('Example: smartling-callback (do not use "/" as the first character)') . '</div>',
    '#required' => FALSE,
  );

  $form['accaunt_info']['actions']['submit'][] = array(
    '#type' => 'submit',
    '#name' => 'save',
    '#value' => t('Save'),
  );
  $form['accaunt_info']['actions']['submit'][] = array(
    '#type' => 'submit',
    '#name' => 'test_connection',
    '#value' => t('Test connection'),
  );

  return $form;
}

function smartling_admin_account_info_settings_form_validate($form, &$form_state) {
  $project_id = '';
  $smartling_key = '';
  if (isset($form_state['values']['api_url']) && !empty($form_state['values']['api_url'])) {
    $api_url = check_plain($form_state['values']['api_url']);
    $status = valid_url($api_url, TRUE);
    if (!$status) {
      drupal_set_message(t('API URL has wrong format'), 'error');
      form_set_error('api_url');
    }
  }
  else {
    $api_url = SMARTLING_DEFAULT_API_URL;
  }

  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    $project_id = check_plain(trim($form_state['values']['project_id']));
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    $smartling_key = trim($form_state['values']['smartling_key']);
  }
  else {
    $smartling_key = smartling_settings_get_handler()->getKey();
  }
  if (smartling_api_key_check($smartling_key)) {
    if (smartling_project_id_check($project_id)) {
      // Init api object
      $api = new SmartlingAPI($api_url, $smartling_key, $project_id, SMARTLING_PRODUCTION_MODE);
      $api->getList('en', array('limit' => 1));

      if ($api->getCodeStatus() !== 'SUCCESS') {
        drupal_set_message(t('Please enter correct Smartling settings.'), 'error');
        form_set_error('api_url');
        form_set_error('smartling_key');
        form_set_error('project_id');
      }
    }
    else {
      drupal_set_message(t('Please enter valid Smartling Project Id.'), 'error');
      form_set_error('project_id');
    }
  }
  else {
    drupal_set_message(t('Please enter valid Smartling key.'), 'error');
    form_set_error('smartling_key');
  }

  // Target locales validate
  if (count(array_filter($form_state['values']['target_locales'])) == 0) {
    drupal_set_message(t('At least one locale must be selected'), 'error');
    form_set_error('target_locales');
  }
}

function smartling_admin_account_info_settings_form_submit($form, &$form_state) {
  // Save
  $smartling_settings = smartling_settings_get_handler();
  // Account settings
  if (isset($form_state['values']['api_url'])) {
    $smartling_settings->setApiUrl(check_plain($form_state['values']['api_url']));
  }
  if (isset($form_state['values']['project_id'])) {
    $smartling_settings->setProjectId(check_plain($form_state['values']['project_id']));
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    $smartling_settings->setKey(check_plain(trim($form_state['values']['smartling_key'])));
  }
  // Retrieval type
  if (isset($form_state['values']['production_retrieval_type']) && !empty($form_state['values']['production_retrieval_type'])) {
    $smartling_settings->setRetrievalType($form_state['values']['production_retrieval_type']);
  }

  // Target locales
  $smartling_settings->makeTargetLocales($form_state['values']['target_locales']);
  $smartling_settings->makeLocalesConvertArray($form_state['values']);

  // Callback
  if (isset($form_state['values']['callback_url_use']) && !empty($form_state['values']['callback_url_use'])) {
    $smartling_settings->setCallbackUrlUse($form_state['values']['callback_url_use']);
  }

  menu_rebuild();
  menu_router_build();

  drupal_set_message(t('Account settings saved.'));

  if ($form_state['clicked_button']['#name'] == 'save') {
    drupal_goto(current_path());
  }

  // Test
  if ($form_state['clicked_button']['#name'] == 'test_connection') {
    foreach ($form_state['values']['target_locales'] as $key => $d_locale) {
      if ($d_locale !== 0 && $d_locale == $key) {
        $s_locale = smartling_convert_locale_drupal_to_smartling($d_locale);
        // Init api object
        $api = new SmartlingAPI($smartling_settings->getApiUrl(), $smartling_settings->getKey(), $smartling_settings->getProjectId(), SMARTLING_PRODUCTION_MODE);
        $api->getList($s_locale);

        if ($api->getCodeStatus() == 'SUCCESS') {
          drupal_set_message(t('Test connection for locale @d_locale is success.', array('@d_locale' => $d_locale)));
        }
        else {
          drupal_set_message(t('Test connection for locale @d_locale is fail.', array('@d_locale' => $d_locale)), 'error');
        }
      }
    }
    drupal_goto(current_path());
  }
}

/**
 * Smartling log info form
 */
function smartling_admin_log_info_settings_form($form, &$form_state) {
  $smartling_settings = smartling_settings_get_handler();
  $form['log_info']['log_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Smartling mode'),
    '#default_value' => $smartling_settings->getLogMode(),
    '#options' => $smartling_settings->getLogModeOptions(),
    '#description' => t('Log ON dy default.'),
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_log_info_settings_form_submit'),
  );

  $form['log_info']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function smartling_admin_log_info_settings_form_submit($form, &$form_state) {
  if (isset($form_state['values']['log_mode'])) {
    if ($form_state['values']['log_mode'] == FALSE) {
      smartling_watchdog('Smartling log OFF', FALSE);
    }
    elseif ($form_state['values']['log_mode'] == TRUE) {
      smartling_watchdog('Smartling log ON', FALSE);
    }
    smartling_settings_get_handler()->setLogMode($form_state['values']['log_mode']);
  }
  drupal_goto(current_path(), array('fragment' => 'smartling-smartling-log'));
}

/**
 * Content translation form
 */
function smartling_admin_node_translation_settings_form($form, &$form_state) {
  $raw_types = node_type_get_types();
  $node_translate_fields = smartling_settings_get_handler()->nodeGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['node_translation'] = array(
    '#type' => 'item',
    '#title' => t('Which content types do you want to translate?'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'administrative_settings',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_node_translation_settings_form_submit'),
  );

  $rows = array();
  $fix_field_settings = array();

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
            // Error in field settings
            if (smartling_nodes_method($value->type) && smartling_field_is_translatable_by_field_name($field_machine_name) && !in_array($field_machine_name, $fix_field_settings)) {
              $fix_field_settings[$field_machine_name] = l($field_label, 'smartling/translatable/' . $field_machine_name . '/nojs', array(
                'attributes' => array(
                  'class' => array('use-ajax', 'smartling-translatable-' . $field_machine_name),
                  'title' => t('Fix field settings'),
              )));
            }
          }
        }
      }

      if (!isset($fr_fields['title_field'])) {
        $fr_fields['title_field'] = array(
          '#type' => 'checkbox',
          '#title' => t('Title (Note: field will be created.)'),
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-separator-' . $field_machine_name),
            'name' => 'title_swap_' . $value->type,
            'class' => array('field'),
          ),
        );

        $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type]['title_field'])) ? TRUE : FALSE;
        if ($is_in_conf) {
          $fr_fields['title_field']['#attributes']['checked'] = 'checked';
        }
      }
    }
    else {
      $options = array(0 => t('- Select Method -'), 2 => t('Nodes method'), 1 => t('Fields method'));

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
      array('data' => check_plain($value->name), 'width' => '20%'),
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

  $form['node_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  if (!empty($fix_field_settings) && is_array($fix_field_settings)) {
    $suffix = '<div id="smartling-untranslatable-fix">' . t('Following field(s) should be set as untranslatable for node translation: !fields. Click on field name to fix.', array('!fields' => implode(', ', $fix_field_settings))) . '</div>';
    $form['node_translation']['fix'] = array(
      '#type' => 'markup',
      '#title' => t('Fix field settings'),
      '#title_display' => 'invisible',
      '#suffix' => $suffix,
    );
  }

  foreach (array_keys($node_translate_fields) as $content_type) {
    $form['node_translation']['types']['#default_value'][$content_type] = 1;
  }

  $form['node_translation']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Node Translation Settings - Form Submit
 */
function smartling_admin_node_translation_settings_form_submit($form, &$form_state) {
  $smartling_settings = smartling_settings_get_handler();
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $content_type = $parts[0];
        $content_field = $parts[1];

        $translate[$content_type][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update)
        if (smartling_fields_method($content_type)) {
          $field = field_info_field($content_field);
          $field['translatable'] = 1;
          field_update_field($field);
        }
      }
    } // END:  Selected Content Types and Fields
    // Look for Selected Translation Type.
    if (FALSE !== strpos($key, '_TT_')) {
      // And only if set to translate
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
      // And only if set to swap
      if ($value != 0) {
        $content_type = substr($key, strlen('title_swap_'));

        // Do the actual title replacement
        $entity_type = 'node';
        $bundle = $content_type;
        $legacy_field = 'title';

        // Use the Title module to migrate the content.
        if (title_field_replacement_toggle($entity_type, $bundle, $legacy_field)) {
          $operations[] = array('title_field_replacement_batch', array($entity_type, $bundle, $legacy_field));
          // Add in config
          $translate[$content_type]['title_field'] = 'title_field';
          $field = field_info_field('title_field');
          (smartling_fields_method($content_type)) ? $field['translatable'] = 1 : $field['translatable'] = 0;
          $operations[] = array('field_update_field', array($field));
        }
      }
    }
  }

  $smartling_settings->nodeSetFieldsSettings($translate);
  drupal_set_message(t('Your content types have been updated.'));

  smartling_watchdog('Smartling content types and fields have been updated.', FALSE);

  $redirect = url('admin/config/regional/smartling', array('absolute' => TRUE, 'fragment' => 'smartling-content-types-and-fields'));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }
}

function smartling_admin_taxonomy_translation_settings_form($form, &$form_state) {
  $raw_types = taxonomy_get_vocabularies();
  $term_translate_fields = smartling_settings_get_handler()->taxonomy_termGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['taxonomy_translation'] = array(
    '#type' => 'item',
    '#title' => t('Which vocabularies do you want to translate?'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'administrative_settings',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_taxonomy_translation_settings_form_submit'),
  );

  $rows = array();

  foreach ($raw_types as $vocabulary) {
    if (smartling_supported_type('taxonomy_term', $vocabulary->machine_name)) {
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
      switch ($vocabulary_mode) {
        case I18N_MODE_TRANSLATE:
          $voc_title = 'Translate method';
          break;

        case I18N_MODE_LOCALIZE:
          $voc_title = 'Localize method';
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
          }
        }
      }

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
    else {
      $options = array(0 => t('- Select Method -'), 2 => t('Translate method'), 1 => t('Localize method'));

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
      array('data' => $vocabulary->name, 'width' => '20%'),
      array('data' => drupal_render($fr_tt), 'width' => '20%'),
      array('data' => drupal_render($fr_fields), 'width' => '60%'),
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

  return $form;
}

/**
 * Taxonomy Translation Settings - Form Submit
 */
function smartling_admin_taxonomy_translation_settings_form_submit($form, &$form_state) {
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $machine_name = $parts[0];
        $content_field = $parts[1];

        $translate[$machine_name][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update)
        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    } // END:  Selected Content Types and Fields
    // Look for Selected Translation Type.
    if (FALSE !== strpos($key, '_TT_')) {
      // And only if set to translate
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
        // And only if set to swap
        if ($value != 0) {
          $machine_name = substr($key, strlen($term_field_name . '_swap_'));

          // Do the actual replacement
          $entity_type = 'taxonomy_term';
          $legacy_field = $term_field_name;

          // Use the Title module to migrate the content.
          if (title_field_replacement_toggle($entity_type, $machine_name, $legacy_field)) {
            $operations[] = array('title_field_replacement_batch', array($entity_type, $machine_name, $legacy_field));
            // Add in config
            $translate[$machine_name][$term_field_name . '_field'] = $term_field_name . '_field';
            $vocabulary = taxonomy_vocabulary_machine_name_load($machine_name);
            $field = field_info_field($term_field_name . '_field');
            ($vocabulary->i18n_mode == I18N_MODE_LOCALIZE) ? $field['translatable'] = 1 : $field['translatable'] = 0;
            $operations[] = array('field_update_field', array($field));
          }
        }
      }
    }
  }

  smartling_settings_get_handler()->taxonomy_termSetFieldsSettings($translate);
  drupal_set_message(t('Your taxonomy vocabularies have been updated.'));

  smartling_watchdog('Smartling taxonomy vocabularies and fields have been updated.', FALSE);

  $redirect = url('admin/config/regional/smartling', array('absolute' => TRUE, 'fragment' => 'smartling-taxonomy-vocabularies-and-fields'));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }
}

function smartling_admin_comments_translation_settings_form($form, &$form_state) {
  $raw_types = node_type_get_types();
  $comment_translate_fields = smartling_settings_get_handler()->commentGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['comments_translation'] = array(
    '#type' => 'item',
    '#title' => t('Which comments do you want to translate?'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'administrative_settings',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_comments_translation_settings_form_submit'),
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

  return $form;
}

/**
 * Comments Translation Settings - Form Submit
 */
function smartling_admin_comments_translation_settings_form_submit($form, &$form_state) {
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $machine_name = $parts[0];
        $content_field = $parts[1];

        $translate[$machine_name][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update)
        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    } // END:  Selected Content Types and Fields
    // Look for any comments we need to do the swap for.
    if (FALSE !== strpos($key, 'subject_swap_')) {
      // And only if set to swap
      if ($value != 0) {
        $machine_name = substr($key, strlen('subject_swap_'));

        // Do the actual replacement
        $entity_type = 'comment';
        $bundle = $machine_name;
        $legacy_field = 'subject';

        // Use the Title module to migrate the content.
        if (title_field_replacement_toggle($entity_type, $bundle, $legacy_field)) {
          $operations[] = array('title_field_replacement_batch', array($entity_type, $bundle, $legacy_field));
          // Add in config
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

  smartling_watchdog('Smartling comments and fields have been updated.', FALSE);

  $redirect = url('admin/config/regional/smartling', array('absolute' => TRUE, 'fragment' => 'smartling-comment-fields'));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations
    );

    batch_set($batch);
    batch_process($redirect);
  }
  else {
    $form_state['redirect'] = $redirect;
  }
}

function smartling_admin_users_translation_settings_form($form, &$form_state) {
  $user_translate_fields = smartling_settings_get_handler()->userGetFieldsSettings();

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['users_translation'] = array(
    '#type' => 'item',
    '#title' => t('Which user do you want to translate?'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'administrative_settings',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_users_translation_settings_form_submit'),
  );

  $rows = array();
  $bundle = 'user';
  $fr_fields = array();
  $fr_tt['from'] = array(
    '#type' => 'item',
    '#title' => t('Fields method'),
  );

  foreach (field_info_instances('user', $bundle) as $field) {
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

      $is_in_conf = (!empty($user_translate_fields) && isset($user_translate_fields[$bundle][$field_machine_name])) ? TRUE : FALSE;

      if ($is_in_conf) {
        $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
      }
    }
  }

  $rows[$bundle] = array(
    array('data' => drupal_render($fr_tt), 'width' => '20%'),
    array('data' => drupal_render($fr_fields), 'width' => '80%'),
  );
  unset($fr_tt);
  unset($fr_fields);


  $header = array(t('Translation Type'), t('Fields'));

  $variables = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['users_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
  );

  foreach (array_keys($user_translate_fields) as $content_type) {
    $form['users_translation']['types']['#default_value'][$content_type] = 1;
  }

  $form['users_translation']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Users Translation Settings - Form Submit
 */
function smartling_admin_users_translation_settings_form_submit($form, &$form_state) {
  // This is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();

  foreach ($form_state['input'] as $key => $value) {
    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPARATOR_')) {
      // And only if set to translate
      if ($value != 0) {
        $parts = explode('_SEPARATOR_', $key);
        $machine_name = $parts[0];
        $content_field = $parts[1];

        $translate[$machine_name][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update)
        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    } // END: Fields
  }

  smartling_settings_get_handler()->userSetFieldsSettings($translate);
  drupal_set_message(t('Your users have been updated.'));

  smartling_watchdog('Smartling users and fields have been updated.', FALSE);

  $redirect = url('admin/config/regional/smartling', array('absolute' => TRUE, 'fragment' => 'smartling-user-fields'));
  $form_state['redirect'] = $redirect;
}

function smartling_wrap_in_fieldset($form, $title) {
  return array(
    '#type' => 'fieldset',
    '#group' => 'smartling',
    '#title' => t($title),
    '#attributes' => array('class' => array('smartling-' . strtolower(str_replace(' ', '-', $title))), 'id' => array('smartling-' . strtolower(str_replace(' ', '-', $title)))),
    'children' => $form,
  );
}

function smartling_hide_key($key = '') {
  return substr($key, 0, -10) . str_repeat("*", 10);
}

function smartling_api_key_check($key) {
  return preg_match("/[a-z0-9]{8}(?:-[a-z0-9]{4}){3}-[a-z0-9]{12}/", $key);
}

function smartling_project_id_check($project_id) {
  return preg_match("/[a-z0-9]{9}/", $project_id);
}

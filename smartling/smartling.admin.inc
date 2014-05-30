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
  $log_info = drupal_get_form('smartling_admin_log_info_settings_form');

  $output['smartling'][] = smartling_wrap_in_fieldset($accaunt_info, 'Account info');
  $output['smartling'][] = smartling_wrap_in_fieldset($node_translation_settings, 'Content types and fields');
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
  global $base_url;

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
    '#default_value' => smartling_get_api_url(),
    '#size' => 25,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#description' => t('Set api url. Default: https://capi.smartling.com/v1'),
  );

  $form['accaunt_info']['project_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Project Id'),
    '#default_value' => variable_get('smartling_project_id', ''),
    '#size' => 25,
    '#maxlength' => 25,
    '#required' => TRUE,
  );

  $form['accaunt_info']['smartling_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#default_value' => '',
    '#description' => t('Current key:') . ' ' . check_plain(smartling_hide_key(variable_get('smartling_key', ''))),
    '#size' => 40,
    '#maxlength' => 40,
    '#required' => FALSE,
  );

  $retrieval_type = array('pseudo' => 'pseudo', 'published' => 'published', 'pending' => 'pending');
  $form['accaunt_info']['production_retrieval_type'] = array(
    '#type' => 'radios',
    '#title' => t('Retrieval type'),
    '#default_value' => variable_get('smartling_retrieval_type', 'published'),
    '#options' => $retrieval_type,
    '#description' => t('Param for download translate.'),
  );

  $language_options_list = array();
  $languages = language_list();
  $default_language = language_default();
  unset($languages[$default_language->language]);

  foreach ($languages as $langcode => $language) {
    if ($language->enabled != '0') {
      $language_options_list[$langcode] = check_plain($language->name);
    }
  }
  if (!empty($language_options_list)) {
    $form['accaunt_info']['target_locales'] = array(
      '#type' => 'checkboxes',
      '#options' => $language_options_list,
      '#title' => t('Target Locales'),
      '#default_value' => variable_get('smartling_target_locales', array()),
      '#prefix' => '<div class="wrap-target-locales">',
    );

    $total = count($language_options_list);
    $counter = 0;
    $locales_convert_array = variable_get('smartling_locales_convert_array', array());
    foreach ($language_options_list as $langcode => $lang_name) {
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
    '#markup' => '<p>' . t('Site default language: @lang', array('@lang' => check_plain($default_language->name))) . '</p>',
  );
  $form['accaunt_info']['default_language_format'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($default_language->language),
    '#default_value' => (isset($locales_convert_array[$default_language->language])) ? $locales_convert_array[$default_language->language] : '',
    '#size' => 6,
    '#maxlength' => 10,
    '#required' => TRUE,
    '#description' => t('Set Smartling format for Default language. Example: en-US for en.'),
    '#suffix' => '<p>' . l(t('Change default language'), url('admin/config/regional/language')) . '</p>',
  );
  $form_state['smartling_default_language'] = $default_language->language;

  $form['accaunt_info']['callback_info_title'] = array(
    '#type' => 'item',
    '#title' => t('Callback URL'),
  );

  $form['accaunt_info']['callback_url_use'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Smartling callback: /smartling/callback/%cron_key'),
    '#default_value' => variable_get('smartling_callback_url_use', TRUE),
    // '#suffix' => '<div>' . t('Example: smartling-callback (do not use "/" as the first character)') . '</div>',
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
    $api_url = 'https://capi.smartling.com/v1';
  }

  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    $project_id = check_plain(trim($form_state['values']['project_id']));
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    $smartling_key = trim($form_state['values']['smartling_key']);
  }
  else {
    $smartling_key = variable_get('smartling_key', '');
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
  // Account settings
  if (isset($form_state['values']['api_url']) && !empty($form_state['values']['api_url'])) {
    variable_set('smartling_api_url', check_plain($form_state['values']['api_url']));
  }
  elseif (empty($form_state['values']['api_url'])) {
    variable_set('smartling_api_url', 'https://capi.smartling.com/v1');
  }
  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    variable_set('smartling_project_id', check_plain($form_state['values']['project_id']));
  }
  elseif (empty($form_state['values']['project_id'])) {
    variable_del('smartling_project_id');
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    variable_set('smartling_key', check_plain(trim($form_state['values']['smartling_key'])));
  }
//  elseif (empty($form_state['values']['smartling_key'])) {
//    variable_del('smartling_key');
//  }
  // Retrieval type
  if (isset($form_state['values']['production_retrieval_type']) && !empty($form_state['values']['production_retrieval_type'])) {
    variable_set('smartling_retrieval_type', $form_state['values']['production_retrieval_type']);
  }

  // Target locales
  $target_locales = $form_state['values']['target_locales'];
  $locales_convert_array = $form_state['values']['target_locales'];
  foreach ($form_state['values']['target_locales'] as $key => $lang) {
    if ($lang === 0) { // Must be ===
      unset($target_locales[$key]);
      unset($locales_convert_array[$key]);
    }
    else {
      if (!empty($form_state['values']['target_locales_text_key_' . $key])) {
        $locales_convert_array[$key] = check_plain($form_state['values']['target_locales_text_key_' . $key]);
      }
    }
  }
  if (!empty($target_locales)) {
    variable_set('smartling_target_locales', $target_locales);
  }
  variable_set('smartling_locales_convert_array', $locales_convert_array);

  // Default language format
  $locales = variable_get('smartling_locales_convert_array', array());
  $locales = array_merge($locales, array($form_state['smartling_default_language'] => check_plain($form_state['values']['default_language_format'])));
  variable_set('smartling_locales_convert_array', $locales);

  // Update once for 7000
  if (variable_get('smartling_update_7000_locales_convert_array') === FALSE) {
    _smartling_update_7000_locales_convert_array();
  }

  // Callback
  if (isset($form_state['values']['callback_url_use']) && !empty($form_state['values']['callback_url_use'])) {
    variable_set('smartling_callback_url_use', (int) $form_state['values']['callback_url_use']);
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
        $api_url = variable_get('smartling_api_url', '');
        $project_id = variable_get('smartling_project_id', '');
        $key = variable_get('smartling_key', '');
        $api = new SmartlingAPI($api_url, $key, $project_id, SMARTLING_PRODUCTION_MODE);
        $api->getList($s_locale);

        if ($api->getCodeStatus() == 'SUCCESS') {
          drupal_set_message(t('Test connection for locale @s_locale is success.', array('@s_locale' => $s_locale)));
        }
        else {
          drupal_set_message(t('Test connection for locale @s_locale is fail.', array('@s_locale' => $s_locale)), 'error');
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
  $active = array(0 => 'OFF', 1 => 'ON');
  $form['log_info']['log_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Smartling mode'),
    '#default_value' => (smartling_get_log_mode()) ? 1 : 0,
    '#options' => $active,
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
    if ($form_state['values']['log_mode'] == 0) {
      smartling_watchdog('Smartling log OFF', FALSE);
      variable_set('smartling_log', FALSE);
    }
    elseif ($form_state['values']['log_mode'] == 1) {
      smartling_watchdog('Smartling log ON', FALSE);
      variable_set('smartling_log', TRUE);
    }
  }
  drupal_goto(current_path(), array('fragment' => 'smartling-smartling-log'));
}

/**
 * Content translation form
 */
function smartling_admin_node_translation_settings_form($form, &$form_state) {
  $setup_complete = smartling_is_configured();

  $raw_types = node_type_get_types();
  $translate = variable_get('smartling_translate_fields', array());

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

    if (smartling_supported_type($value->type)) {
      $fr_tt['from'] = array(
        '#type' => 'item',
        '#title' => t('Fields method'),
      );

      $fr_fields = array();
      $smartling_translate_fields = variable_get('smartling_translate_fields', FALSE);

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

          $is_enabled = !empty($translate[$value->type]) && array_search($field_machine_name, $translate[$value->type]) !== FALSE;

          $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;

          if ($is_enabled && $is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
            // Error in field settings
            if (smartling_nodes_method($value->type) && smartling_field_is_translatable_by_field_name($field_machine_name) && !in_array($field_machine_name, $fix_field_settings)) {
              $fix_field_settings[$field_machine_name] = l($field_label, 'smartling/translatable/' . $field_machine_name . '/nojs', array(
                  'attributes' => array(
                    'class' => array('use-ajax', 'smartling-translatable-' . $field_machine_name),
                    'title' => t('Fix field settings'),
                  )
                ));
            }
          }
        }
      }

      if (!isset($fr_fields['title_field']) && smartling_fields_method($value->type)) {
        $fr_fields['title_field'] = array(
          '#type' => 'checkbox',
          '#title' => 'Title (Note: field will be created.)',
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-separator-' . $field_machine_name),
            'name' => 'title_swap_' . $value->type,
            'class' => array('field'),
          ),
        );

        $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type]['title_field'])) ? TRUE : FALSE;
        if ($setup_complete && $is_in_conf) {
          $fr_fields['title_field']['#attributes']['checked'] = 'checked';
        }
      }

      if (smartling_nodes_method($value->type)) {
        $fr_tt['from']['#title'] = t('Nodes method');
        // If !isset(title_field)
        if (!isset($fr_fields['title_field'])) {
          $field_label = t('Title');
          $node_title_name = 'node_title';
          $fr_fields[$node_title_name] = array(
            '#type' => 'checkbox',
            '#title' => check_plain($field_label),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $value->type . '-separator-' . $node_title_name),
              'name' => $value->type . '_SEPARATOR_' . $node_title_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $value->type . '-separator-' . $node_title_name,
          );

          $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type][$node_title_name])) ? TRUE : FALSE;

          if ($is_in_conf) {
            $fr_fields[$node_title_name]['#attributes']['checked'] = 'checked';
          }
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
    '#suffix' => '* Note: Title field is not used if the content type specified method of translating nodes.',
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

  foreach (array_keys($translate) as $content_type) {
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
          if ($content_field != 'node_title') {
            $field = field_info_field($content_field);
            $field['translatable'] = 1;
            field_update_field($field);
          }
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
          variable_set('language_content_type_' . $content_type, $value);
        }
        elseif ($value == 1) {
          variable_set('language_content_type_' . $content_type, SMARTLING_KEY);
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
          $field['translatable'] = 1;
          $operations[] = array('field_update_field', array($field));
        }
      }
    }
  }

  if (!empty($translate)) {
    variable_set('smartling_translate_fields', $translate);
  }
  else {
    variable_delete('smartling_translate_fields');
  }
  drupal_set_message(t('Your content types have been updated.'));

  smartling_watchdog('Smartling content types and fields have been updated.', FALSE);

  $redirect = url('admin/config/regional/smartling', array(
    'absolute' => TRUE,
    'fragment' => 'smartling-content-types-and-fields'
  ));

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

function smartling_wrap_in_fieldset($form, $title) {
  return array(
    '#type' => 'fieldset',
    '#group' => 'smartling',
    '#title' => t($title),
    '#attributes' => array(
      'class' => array('smartling-' . strtolower(str_replace(' ', '-', $title))),
      'id' => array('smartling-' . strtolower(str_replace(' ', '-', $title)))
    ),
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
<?php

/**
 * @file
 * Admin forms and other for Smartling module administration section.
 */
function smartling_admin_configuration_view($form_short_id = NULL, $show_fieldset = TRUE) {
  ctools_include('modal');
  ctools_modal_add_js();

  $show_fieldset = FALSE;

  $output = array();

  $output['smartling'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'smartling') . '/js/smartling_admin.js'),
      'css' => array(drupal_get_path('module', 'smartling') . '/css/smartling_admin.css'),
    ),
  );

  $accaunt_info = drupal_get_form('smartling_admin_account_info_settings_form', $show_fieldset);
  $node_translation_settings = drupal_get_form('smartling_admin_node_translation_settings_form', $show_fieldset);
  $default_language_info = drupal_get_form('smartling_admin_default_language_settings_form', $show_fieldset);
  $log_info = drupal_get_form('smartling_admin_log_info_settings_form', $show_fieldset);

  $output['smartling'][] = smartling_wrap_in_fieldset($accaunt_info, 'Account info');
  $output['smartling'][] = smartling_wrap_in_fieldset($node_translation_settings, 'Content types and fields');
  $output['smartling'][] = smartling_wrap_in_fieldset($default_language_info, 'Default language info');
  $output['smartling'][] = smartling_wrap_in_fieldset($log_info, 'Smartling log');
  if (module_exists('smartling_load_test')) {
    $dev_settings = drupal_get_form('smartling_admin_dev_settings_form', $show_fieldset);
    $output['smartling'][] = smartling_wrap_in_fieldset($dev_settings, 'Dev settings');
  }
  return $output;
}

/**
 * Account info form
 */
function smartling_admin_account_info_settings_form($form, &$form_state, $show_fieldset = FALSE) {
  $form['accaunt_info'] = array(
    '#type' => $show_fieldset ? 'fieldset' : 'item',
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

  $smartling_project_id = variable_get('smartling_project_id', FALSE);
  $form['accaunt_info']['project_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Project Id'),
    '#default_value' => $smartling_project_id ? $smartling_project_id : '',
    '#size' => 25,
    '#maxlength' => 25,
    '#required' => FALSE,
  );

  $smartling_key = variable_get('smartling_key', FALSE);
  $form['accaunt_info']['smartling_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#default_value' => $smartling_key ? $smartling_key : '',
    '#size' => 40,
    '#maxlength' => 40,
    '#required' => FALSE,
  );

  $mode = variable_get('smartling_mode', NULL);
  $active = array(0 => 'PRODUCTION', 1 => 'SANDBOX');
  $form['accaunt_info']['smartling_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Smartling mode'),
    '#default_value' => (!is_null($mode) && ($mode == 'PRODUCTION')) ? 0 : 1,
    '#options' => $active,
    '#description' => t('Configure Smartling mode.'),
  );

  $retrieval_type_default = variable_get('smartling_retrieval_type', NULL);
  $retrieval_type = array('pseudo' => 'pseudo', 'published' => 'published', 'pending' => 'pending');
  $form['accaunt_info']['production_retrieval_type'] = array(
    '#type' => 'radios',
    '#title' => t('Retrieval type'),
    '#default_value' => (!is_null($retrieval_type_default)) ? $retrieval_type_default : 'pseudo',
    '#options' => $retrieval_type,
    '#description' => t('Param for upload translate.'),
    '#states' => array(
      'visible' => array(
        ':input[name="smartling_mode"]' => array('value' => 0),
      ),
    ),
  );

  $form['accaunt_info']['callback_info_title'] = array(
    '#type' => 'item',
    '#title' => t('Callback URL'),
  );

  global $base_url;
  $smartling_callback_url = variable_get('smartling_callback_url', 'smartling-callback');
  $form['accaunt_info']['callback_url'] = array(
    '#type' => 'textfield',
    '#title' => $base_url . '/',
    '#default_value' => $smartling_callback_url,
    '#size' => 50,
    '#maxlength' => 255,
    '#suffix' => '<div>' . t('Example: smartling-callback (do not use / as the first character)') . '</div>',
    '#required' => FALSE,
  );

  $form['accaunt_info']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function smartling_admin_account_info_settings_form_validate($form, &$form_state) {
  $project_id = '';
  $smartling_key = '';
  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    $project_id = $form_state['values']['project_id'];
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    $smartling_key = $form_state['values']['smartling_key'];
  }

  $data = array(
    'apiKey' => trim($smartling_key),
    'projectId' => trim($project_id),
    'limit' => 1
  );

  $options = array(
    'method' => 'POST',
    'data' => drupal_http_build_query($data),
    'timeout' => 5,
    'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
  );

  $url = 'https://api.smartling.com/v1/file/list';
  $response = drupal_http_request($url, $options);
  if ($response->code == 200) {
    $response = json_decode($response->data);
    if ($response->response->code == 'SUCCESS') {
      $form_state['values']['smartling_key'] = trim($smartling_key);
      $form_state['values']['project_id'] = trim($project_id);
    }
    else {
      drupal_set_message(t('Please input correct Smartling settings.'), 'error');
      form_set_error('smartling_key', '');
      form_set_error('project_id');
    }
  }
  else {
    if (isset($response->error)) {
      drupal_set_message($response->error, 'error');
      smartling_watchdog('Smartling account settings error - @error', TRUE, array('@error' => $response->error), WATCHDOG_INFO);
    }
    drupal_set_message(t('Please input correct Smartling settings.'), 'error');
    form_set_error('smartling_key', '');
    form_set_error('project_id');
  }

  // Callback validate
  $callback_url = '';
  if (isset($form_state['values']['callback_url']) && !empty($form_state['values']['callback_url'])) {
    $callback_url = $form_state['values']['callback_url'];
    $status = valid_url($callback_url);
    if (!$status) {
      drupal_set_message('Callback URL has wrong format', 'error');
      form_set_error('callback_url');
    }
  }
}

function smartling_admin_account_info_settings_form_submit($form, &$form_state) {
  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    variable_set('smartling_project_id', $form_state['values']['project_id']);
  }
  elseif (empty($form_state['values']['project_id'])) {
    variable_del('smartling_project_id');
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    variable_set('smartling_key', $form_state['values']['smartling_key']);
  }
  elseif (empty($form_state['values']['smartling_key'])) {
    variable_del('smartling_key');
  }
  if (isset($form_state['values']['smartling_mode'])) {
    if ($form_state['values']['smartling_mode'] == 0) {
      variable_set('smartling_mode', 'PRODUCTION');
      smartling_watchdog('Smartling set in PRODUCTION mode.', FALSE);
      variable_set('smartling_retrieval_type', $form_state['values']['production_retrieval_type']);
    }
    elseif ($form_state['values']['smartling_mode'] == 1) {
      variable_set('smartling_mode', 'SANDBOX');
      smartling_watchdog('Smartling set in SANDBOX mode.', FALSE);
    }
  }

  if (isset($form_state['values']['callback_url']) && !empty($form_state['values']['callback_url'])) {
    variable_set('smartling_callback_url', $form_state['values']['callback_url']);
  }
  elseif (empty($form_state['values']['callback_url'])) {
    variable_set('smartling_callback_url', 'smartling-callback');
  }

  $destination = 'admin/config/regional/smartling';
  $token = drupal_get_token('admin_menu/flush-cache/menu');
  $path = 'admin_menu/flush-cache/menu';

  $data = array(
    'token' => $token,
    'destination' => $destination,
  );

  $options = array(
    'query' => $data,
  );

  drupal_set_message(t('Account settings saved.'));

  drupal_goto($path, $options);
}

/**
 * Default language info form
 */
function smartling_admin_default_language_settings_form($form, &$form_state) {
  $form['default_language'] = array(
    '#type' => 'item',
    '#title' => t('Default language'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $default_language = language_default();
  $form['default_language']['language'] = array(
    '#markup' => '<p>Site default language: ' . $default_language->name . '</p>',
    '#suffix' => l(t('Change default language'), 'admin/config/regional/language'),
  );

  return $form;
}

/**
 * Smartling log info form
 */
function smartling_admin_log_info_settings_form($form, &$form_state) {
  $mode = variable_get('smartling_log', 1);
  $active = array(0 => 'OFF', 1 => 'ON');
  $form['log_info']['log_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Smartling mode'),
    '#default_value' => ($mode) ? 1 : 0,
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
      smartling_watchdog('smartling', 'Smartling log OFF', FALSE);
      variable_set('smartling_log', 0);
    }
    elseif ($form_state['values']['log_mode'] == 1) {
      smartling_watchdog('smartling', 'Smartling log ON', FALSE);
      variable_set('smartling_log', 1);
    }
  }
}

/**
 * Smartling dev settings form
 */
function smartling_admin_dev_settings_form($form, &$form_state) {

}

/**
 * Content translation form
 */
function smartling_admin_node_translation_settings_form($form, &$form_state, $show_fieldset = FALSE) {
  $setup_complete = smartling_is_configured();

  $raw_types = node_type_get_types();
  $translate = variable_get('smartling_translate_fields', array());

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['node_translation'] = array(
    '#type' => $show_fieldset ? 'fieldset' : 'item',
    '#title' => t('Which content types do you want translated?'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'administrative_settings',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_node_translation_settings_form_submit'),
  );

  $rows = array();

  foreach ($raw_types as $value) {
    if (smartling_supported_type($value->type)) {
      $fr_tt['from'] = array(
        '#type' => 'item',
        '#title' => t('Fields method'),
      );

      $fr_fields = array();
      $smartling_translate_fields = variable_get('smartling_translate_fields', FALSE);

      if (smartling_nodes_method($value->type)) {
        $fr_tt['from']['#title'] = t('Nodes method');
        $field_label = t('Title');
        $node_title_name = 'node_title';
        $fr_fields[$node_title_name] = array(
          '#type' => 'checkbox',
          '#title' => check_plain($field_label),
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-seperator-' . $node_title_name),
            'name' => $value->type . '_SEPERATOR_' . $node_title_name,
            'class' => array('field'),
          ),
          '#id' => 'edit-form-item-' . $value->type . '-seperator-' . $node_title_name,
        );

        $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type][$node_title_name])) ? TRUE : FALSE;

        if ($is_in_conf) {
          $fr_fields[$node_title_name]['#attributes']['checked'] = 'checked';
        }
      }

      foreach (field_info_instances('node', $value->type) as $field) {
        $field_label = $field['label'];
        $field_machine_name = $field['field_name'];
        $field_type = $field['widget']['type'];
        if (array_search($field_type, $translatable_field_types)) {
          $fr_fields[$field_machine_name] = array(
            '#type' => 'checkbox',
            '#title' => check_plain($field_label),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $value->type . '-seperator-' . $field_machine_name),
              'name' => $value->type . '_SEPERATOR_' . $field_machine_name,
              'class' => array('field'),
            ),
            '#id' => 'edit-form-item-' . $value->type . '-seperator-' . $field_machine_name,
          );

          $is_enabled = !empty($translate[$value->type]) && array_search($field_machine_name, $translate[$value->type]) !== FALSE;

          $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;

          if ($is_enabled && $is_in_conf) {
            $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
          }
        }
      }
      if (!isset($fr_fields['title_field']) && smartling_fields_method($value->type)) {
        $fr_fields['title_field'] = array(
          '#type' => 'checkbox',
          '#title' => 'Title (Note: field will be created.)',
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-seperator-' . $field_machine_name),
            'name' => 'title_swap_' . $value->type,
            'class' => array('field'),
          ),
        );

        $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type]['title_field'])) ? TRUE : FALSE;
        if ($setup_complete && $is_in_conf) {
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
      array('data' => $value->name, 'width' => '20%'),
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
    if (FALSE !== strpos($key, '_SEPERATOR_')) {
      // And only if set to translate
      if ($value != 0) {
        $parts = explode('_SEPERATOR_', $key);
        $content_type = $parts[0];
        $content_field = $parts[1];

        $translate[$content_type][$content_field] = $content_field;

        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update)
        if ($content_field != 'node_title') {
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

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations
    );

    batch_set($batch);
  }
}

function smartling_wrap_in_fieldset($form, $title) {
  return array(
    '#type' => 'fieldset',
    '#group' => 'smartling',
    '#title' => t($title),
    '#attributes' => array('class' => array('smartling-' . strtolower(str_replace(' ', '-', $title)))),
    'children' => $form,
  );
}

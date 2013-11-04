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
    ),
  );

  $accaunt_info = drupal_get_form('smartling_admin_accaunt_info_settings_form', $show_fieldset);
  $node_translation_settings = drupal_get_form('smartling_admin_node_translation_settings_form', $show_fieldset);
  $default_language_info = drupal_get_form('smartling_admin_default_language_settings_form', $show_fieldset);

  $output['smartling'][] = smartling_wrap_in_fieldset($accaunt_info, 'Account info');
  $output['smartling'][] = smartling_wrap_in_fieldset($node_translation_settings, 'Content types and fields');
  $output['smartling'][] = smartling_wrap_in_fieldset($default_language_info, 'Default language info');

  return $output;
}

/**
 * Account info form
 */
function smartling_admin_accaunt_info_settings_form($form, &$form_state, $show_fieldset = FALSE) {
  $form['accaunt_info'] = array(
    '#type' => $show_fieldset ? 'fieldset' : 'item',
    '#title' => t('Account info'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'accaunt_info',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_accaunt_info_settings_form_submit'),
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

  $form['accaunt_info']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function smartling_admin_accaunt_info_settings_form_submit($form, &$form_state) {
  if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
    variable_set('smartling_project_id', $form_state['values']['project_id']);
  }
  if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
    variable_set('smartling_key', $form_state['values']['smartling_key']);
  }
  if (isset($form_state['values']['smartling_mode'])) {
    if ($form_state['values']['smartling_mode'] == 0) {
      variable_set('smartling_mode', 'PRODUCTION');
      variable_set('smartling_retrieval_type', $form_state['values']['production_retrieval_type']);
    }
    elseif ($form_state['values']['smartling_mode'] == 1) {
      variable_set('smartling_mode', 'SANDBOX');
    }
  }
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
    '#group' => 'default_language',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_default_language_settings_form_submit'),
  );

  $default_language = language_default();
  $form['default_language']['language'] = array(
    '#markup' => '<p>Site default language: ' . $default_language->name . '</p>',
    '#suffix' => l('Change default language', 'admin/config/regional/language'),
  );

  return $form;
}

/**
 * Content translation form
 */
function smartling_admin_node_translation_settings_form($form, &$form_state, $show_fieldset = FALSE) {
  $setup_complete = (is_string(smartling_is_configured())) ? FALSE : TRUE;

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
    $fr = array();
    foreach (field_info_instances('node', $value->type) as $field) {
      $field_label = $field['label'];
      $field_machine_name = $field['field_name'];
      $field_type = $field['widget']['type'];
      if (array_search($field_type, $translatable_field_types)) {
        $fr[$field_machine_name] = array(
          '#type' => 'checkbox',
          '#title' => $field_label,
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-seperator-' . $field_machine_name),
            'name' => $value->type . '_SEPERATOR_' . $field_machine_name,
            'class' => array('field'),
          ),
          '#id' => 'edit-form-item-' . $value->type . '-seperator-' . $field_machine_name,
          '#states' => array(
            'invisible' => array(
              ':input[name="profile_' . $value->type . '"]' => array('value' => 'DISABLED'),
            ),
          ),
        );

        $is_enabled = !empty($translate[$value->type]) && array_search($field_machine_name, $translate[$value->type]) !== FALSE;

        $smartling_translate_fields = variable_get('smartling_translate_fields', FALSE);
        $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;

        if ($is_enabled && $is_in_conf) {
          $fr[$field_machine_name]['#attributes']['checked'] = 'checked';
        }
      }
    }
    if (!isset($fr['title_field'])) {
      $fr['title_field'] = array(
        '#type' => 'checkbox',
        '#title' => 'Title (Note: field will be created.)',
        '#attributes' => array(
          'id' => array('edit-form-item-' . $value->type . '-seperator-' . $field_machine_name),
          'name' => 'title_swap_' . $value->type,
          'class' => array('field'),
        ),
      );

      $smartling_translate_fields = variable_get('smartling_translate_fields', FALSE);
      $is_in_conf = (!empty($smartling_translate_fields) && $smartling_translate_fields && isset($smartling_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;
      if ($setup_complete && $is_in_conf) {
        $fr['title_field']['#attributes']['checked'] = 'checked';
      }
    }

    $rows[$value->type] = array(
      array('data' => $value->name, 'width' => '20%'),
      array('data' => drupal_render($fr), 'width' => '65%'),
    );
  }

  $header = array(t('Content Type'), t('Fields'));

  $variables = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('smartling-content-settings-table')),
  );

  $form['node_translation']['types'] = array(
    '#type' => 'markup',
    '#markup' => theme('table', $variables),
    '#suffix' => '* Note: For content type, which is selected at least one field is set to: Multilingual support -> Enabled Smartling translation',
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

  //this is needed for the setup because of the field/node method selector.
  system_settings_form_submit($form, $form_state);

  $translate = array();
  $operations = array();

  $entity_profiles = array();

  foreach ($form_state['input'] as $key => $value) {

    // Look for Selected Content Types and Fields.
    if (FALSE !== strpos($key, '_SEPERATOR_')) {
      // And only if set to translate
      if ($value != 0) {
        $parts = explode('_SEPERATOR_', $key);
        $content_type = $parts[0];
        $content_field = $parts[1];

        $translate[$content_type][$content_field] = $content_field;

        // Set this content type to be Smartling translated.
        variable_set('language_content_type_' . $content_type, SMARTLING_KEY);
        // Set this field to 'translatable'.
        // Update the field via the Field API (Instead of the direct db_update)
        $field = field_info_field($content_field);
        $field['translatable'] = 1;
        field_update_field($field);
      }
    } // END:  Selected Content Types and Fields
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
          //add in config
          $translate[$content_type]['title_field'] = 'title_field';
          // Set this content type to be Smartling translated.
          variable_set('language_content_type_' . $content_type, SMARTLING_KEY);

          $field = field_info_field('title_field');
          $field['translatable'] = 1;
          $operations[] = array('field_update_field', array($field));
        }
      }
    }
    // Look for any profiles
    if (FALSE !== strpos($key, 'profile_')) {
      $content_type = substr($key, strlen('profile_'));
      $entity_profiles['node'][$content_type] = $value;
    }
  }

  if (!empty($translate)) {
    variable_set('smartling_translate_fields', $translate);
  }
  else {
    variable_delete('smartling_translate_fields');
  }
  variable_set('smartling_entity_profiles', $entity_profiles);
  drupal_set_message(t('Your content types have been updated.'));

  if (count($operations) >= 1) {
    $batch = array(
      'title' => t('Preparing content'),
      'operations' => $operations
    );

    batch_set($batch);
    //batch_process('admin/config/regional/smartling');
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

<?php

/**
 * @file
 * Admin forms and other for Smartling module administration section.
 */

/**
 * Content translation form
 */
function smartling_admin_node_translation_settings_form($form, &$form_state, $show_fieldset = FALSE) {
  //$node_types = smartling_translatable_node_types(); // These are the node types marked for translation.  I need 1 field from each to count off of.
  //$node_fields = smartling_translatable_node_field_details(); // These are the fields that are translated.
  //dsm($node_types, '$node_types');
  //dsm($node_fields, '$node_fields');
  //smartling_node_content_to_translate(5);

  $setup_complete = smartling_is_configured();
  $raw_types = node_type_get_types();
  $translate = variable_get('smartling_translate_fields', array());

  // What types of fields DO we translate?
  $translatable_field_types = smartling_get_translatable_field_types();

  $form['node_translation'] = array(
    '#type' => $show_fieldset ? 'fieldset' : 'item',
    '#title' => (!$setup_complete ? t('Translate Content') : t('Which content types do you want translated?')),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'administrative_settings',
    'actions' => array(
      '#type' => 'actions',
    ),
    '#submit' => array('smartling_admin_node_translation_settings_form_submit'),
  );

  if (!$setup_complete) {
    $form['node_translation'][] = array(
      '#type' => 'item',
      '#description' => t('Choose the content types to be translated:'),
    );
  }

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
        if (!$setup_complete || $is_enabled) {
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
      if (!$setup_complete) {
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
    '#suffix' => '* Note: text text text',
  );

  foreach (array_keys($translate) as $content_type) {
    $form['node_translation']['types']['#default_value'][$content_type] = 1;
  }

  if ($setup_complete) {
    $defaults = array();
    foreach ($raw_types as $content_type) {
      $defaults[$content_type->type] = 1;
    }
    $form['node_translation']['types']['#default_value'] = $defaults;
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

        $translate[$content_type][] = $content_field;

        // Set this content type to be Lingotek translated.
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
          $translate[$content_type][] = 'title_field';
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
  variable_set('smartling_translate_fields', $translate);
  variable_set('smartling_entity_profiles', $entity_profiles);
  drupal_set_message(t('Your content types have been updated.'));

  if (count($operations)) {
    $batch = array(
      'title' => t('Preparing content for translation'),
      'operations' => $operations
    );

    batch_set($batch);
    batch_process('admin/config/regional/smartling');
  }
}

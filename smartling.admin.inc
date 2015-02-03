<?php

/**
 * @file
 * Admin forms and other for Smartling module administration section.
 */

/**
 * Smartling settings page callback.
 *
 * @return array
 *   Return render array.
 */
function smartling_admin_configuration_page() {
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
  return drupal_container()->get('smartling.forms.admin_account_info_settings')->buildForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_account_info_settings')->validateForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_account_info_settings')->submitForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_log_info_settings')->buildForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_log_info_settings')->submitForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_entities_translation_settings')->buildForm($form, $form_state);
}

/**
 * Custom submit handler for entities wrapper form.
 *
 * @see smartling_admin_entities_translation_settings_form()
 */
function smartling_admin_entities_translation_settings_form_submit(array $form, array &$form_state) {
  return drupal_container()->get('smartling.forms.admin_entities_translation_settings')->submitForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_node_translation_settings')->buildForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_node_translation_settings')->submitForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_taxonomy_translation_settings')->buildForm($form, $form_state);
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
  return drupal_container()->get('smartling.forms.admin_taxonomy_translation_settings')->submitForm($form, $form_state);
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

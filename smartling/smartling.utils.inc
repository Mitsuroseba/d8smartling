<?php

/**
 * @file
 * Utils functions for Smartling module.
 */

/**
 * Returns whether the given node type has support for Smartling translations.
 *
 * @return
 *   Boolean value.
 */
function smartling_supported_type($type) {
  $smartling_supported_explicitly = variable_get('language_content_type_' . $type, NULL) == SMARTLING_KEY;
  return $smartling_supported_explicitly;
}

/**
 * Gets the Smartling Source Language ( Drupal variable: smartling_source_language).
 * Returns a language code (ie:  en / es / de)
 * Uses the Drupal default language as a fallback.
 *
 * @return
 *   String value.
 */
function smartling_get_source_language() {
  $source_language = variable_get('smartling_source_language', NULL);

  if (empty($source_language)) {
    $drupal_default_language = language_default();
    $source_language = $drupal_default_language->language;
  }

  return $source_language;
}

/*
 * Create and return an empty default node
 */

function smartling_empty_node() {
  $node = new stdClass();
  $node->nid = -2;
  $node->language = LANGUAGE_NONE;
  return $node;
}

/**
 * Smartling custom wrapper function for node_load
 *
 * This is needed because the Smartling module should translate only the most current revision of any node
 *
 * @param $nid entity_id of node to load
 *
 * @return $node most current revision of the node at $nid
 */
function smartling_node_load_default($nid, $vid = NULL, $reset = FALSE) {
  if ($vid == NULL) {
    $query = db_select('node_revision', 'nr')
        ->condition('nid', $nid, '=');
    $query->addExpression('MAX(vid)', 'max_vid');
    $vid = $query->execute()->fetchField();
  }

  $node = node_load($nid, $vid, $reset);
  return $node;
}

/**
 * Menu access callback.
 *
 * Only display Smartling tab for node types, which have translation enabled
 * and where the current node is not language neutral (which should span
 * all languages).
 */
function smartling_access($node, $permission) {
  return TRUE;
}

function smartling_multilingual_support_node_type_submit(&$form, &$form_state) {
  //dsm($form_state, 'smartling_multilingual_support_node_type_submit');
}

/**
 * Checks to make sure the Smartling module setup completed successfully.  If its not, the user is directed to the setup wizard.
 */
function smartling_is_module_setup() {
  $redirect = smartling_is_configured();
  if (!is_bool($redirect)) {
    drupal_goto($redirect); // If something is missing - Go to the Setup Process
  }
}

/**
 * Checks any required configuration parameters are missing.
 *
 * @return string|boolean
 */
function smartling_is_configured() {
  $required_variables = array(
    'smartling_translate_fields',
  );
  foreach ($required_variables as $required_variable) {
    $val = variable_get($required_variable, NULL);
    if (empty($val)) {
      if ($required_variable == 'smartling_translate_fields') {
        return 'admin/config/regional/smartling';
      }
      else {
        return FALSE;
      }
    }
  }
  return TRUE; // all required configuration variables are set
}

function smartling_get_translatable_field_types() {
  // What types of fields DO we translate?
  $included_fields = array('text', 'text_long', 'text_textfield', 'text_textarea', 'text_textarea_with_summary', 'field_collection_embed');
  if (module_exists('link')) {
    $included_fields[] = 'link_field';
  }
  return $included_fields;
}

/**
 * Content node types linked to 'translatable' fields.
 */
function smartling_translatable_node_types() {

  $types = array();

  $fields = smartling_translatable_node_field_details();
  foreach ($fields as $field) {
    foreach ($field['bundles'] as $bundle) {
      $types[$bundle] = $bundle;
    }
  }

  if (count($types) > 0) {
    $types = array_keys($types);
  }

  return $types;
}

/**
 * Goes though ALL the fields in the system and gets the details about the ones that are marked 'translatable'.
 */
function smartling_translatable_node_field_details() {

  $fields = field_info_fields();
  $translatable_fields = array();

  foreach ($fields as $field_id => $field) {
    foreach ($field['bundles'] as $type => $instance) {
      /*
        echo '<br>FieldID: ' . $field_id;
        echo '<br>Field: ' . $field;
        echo '<br>Type: ' . $type;
        echo '<br>Instance: ' . $instance;
       */
      //dsm($field_id, '$field_id');
      //dsm($field, '$field');
      //dsm($type, '$type');
      //dsm($instance, '$instance');

      if (field_is_translatable($type, $field)) {
        //echo '<br>Translatable: YES!' ;
        $field_db_table = array_keys($field['storage']['details']['sql']['FIELD_LOAD_CURRENT']);
        $field_db_table = array_shift($field_db_table);
        $translatable_fields[] = array(
          'entity_type' => $type,
          'machine_name' => $field['field_name'],
          'db_table' => $field_db_table,
          'bundles' => $field['bundles'][$type],
        );
      }
      //echo '<br>';
    }
  }

  /* Return data format
    array (
    0 =>
    array (
    'entity_type' => 'node',
    'machine_name' => 'body',
    'db_table' => 'field_data_body',
    'bundles' =>
    array (
    0 => 'page',
    1 => 'article',
    ),
    ),
    1 =>
    array (
    'entity_type' => 'node',
    'machine_name' => 'title_field',
    'db_table' => 'field_data_title_field',
    'bundles' =>
    array (
    0 => 'article',
    1 => 'page',
    ),
    ),
    )
   */
  return $translatable_fields;
}

/**
 *
 * @param type $nid
 * @return type array
 */
function smartling_node_content_to_translate($nid) {
  $node = smartling_node_load_default($nid);
  $translate = variable_get('smartling_translate_fields', array());
  $node_current_translatable_content = array();
  if (!empty($translate)) {
    foreach ($translate as $type => $field_names) {
      if ($node->type === $type) {
        foreach ($field_names as $field_name) {
          $node_current_translatable_content[$field_name] = $node->{$field_name}[$node->language][0]['value'];
        }
      }
    }
  }
  $smartling_data = smartling_entity_data_load($nid);
  $lang_info = is_null($smartling_data) ? array() : unserialize($smartling_data->locale_name);
  $node_current_translatable_content += $lang_info;
  dsm($node_current_translatable_content, '$node_current_translatable_content');
  return $node_current_translatable_content;
}

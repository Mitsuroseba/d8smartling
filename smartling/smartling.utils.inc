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
    }
  }

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
          $node_current_translatable_content['fields'][$field_name] = $node->{$field_name}[$node->language][0]['value'];
        }
      }
    }
  }

  $smartling_data = smartling_entity_data_load($nid);
  $lang_info = is_null($smartling_data) ? array() : unserialize($smartling_data->locale_name);
  $node_current_translatable_content += $lang_info;

  return $node_current_translatable_content;
}

/**
 * Get basic xml file/
 *
 * @return string
 */
function smartling_get_basic_xml() {
  $xml_body = <<<XML
<?xml version="1.0" encoding="utf-8" ?>
<data>
</data>
XML;
  $xml_doc = simplexml_load_string($xml_body);
  return $xml_doc;
}

/**
 *
 * @param type $file_name
 * @param type $nid
 */
function smartling_entity_data_filename_update($file_name, $nid) {
  $entity_data = smartling_entity_data_load($nid);
  $entity_data->file_name = $file_name;
  smartling_entity_data_save($entity_data);
}

/**
 *
 * @param type $xml_doc
 */
function smartling_save_xml($xml_doc, $nid = NULL) {
  $directory = SMARTLING_DIRECTORY;
  $file_name = 'smartling_translate_' . $nid . '_' . time() . '.xml';
  $path = $directory . '/' . $file_name;

  if (is_dir($directory) && is_writable($directory)) {
    $xml_doc->saveXML($path);
    if (!is_null($nid)) {
      smartling_entity_data_filename_update($file_name, $nid);
    }
    return $xml_doc;
  }
  elseif (drupal_mkdir($directory) && drupal_chmod($directory)) {
    $xml_doc->saveXML($path);
    if (!is_null($nid)) {
      smartling_entity_data_filename_update($file_name, $nid);
    }
    return $xml_doc;
  }
  else {
    drupal_set_message(t('File is not be saved because of the some errors.'), 'error');
    return NULL;
  }
}

/**
 *
 * @param type $nid
 * @return boolean
 */
function smartling_build_xml_by_node_id($nid = NULL) {
  if (!is_null($nid)) {
    $node_content_to_translate = smartling_node_content_to_translate($nid);

    $xml = smartling_get_basic_xml();
    $localize = $xml->addChild('localize');
    $localize->addAttribute('lang', $node_content_to_translate['original_language']);
    foreach ($node_content_to_translate['fields'] as $field_mane => $field_value) {
      $field_value = isset($field_value) ? $field_value : 'error';
      $string = $localize->addChild('string', $field_value);
      $string->addAttribute('name', $field_mane);
    }
    $xml = smartling_save_xml($xml, $nid);
    if (!is_null($xml)) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

/**
 *
 * @param type $nid
 */
function smartling_do_translate($nid) {
  $entity_data = smartling_entity_data_load($nid);
  $file_name = $entity_data->file_name;

  $file_uri = SMARTLING_DIRECTORY . '/' . $file_name;
  $file_type = 'xml';
  $new_file_uri = SMARTLING_DIRECTORY . '/new_' . $file_name;
  $translation_state = 'PUBLISHED';
  $key = '';
  $project_id = '';
  $locale = 'ru-RU';

  //init api object
  $api = new SmartlingAPI($key, $project_id);

  $params = array(
    'approved' => TRUE,
  );
  /*
    //try to upload file
    $result = $api->uploadFile('./test.json', $file_type, $file_uri, $params);
    dsm($result, 'This is a upload file');

    //try to download file
    $result = $api->downloadFile($file_uri, $locale);
    dsm($result, 'This is a download file');

    //try to retrieve file status
    $result = $api->getStatus($file_uri, $locale);
    dsm($result, 'This is a get status');

    //try get files list
    $result = $api->getList($locale);
    dsm($result, 'This is a get list');

    //try to rename file
    $result = $api->renameFile($file_uri, $new_file_uri);
    dsm($result, 'This is a rename file');

    //try to delete file
    $result = $api->deleteFile($new_file_uri);
    dsm($result, 'This is delete file');

   */
}

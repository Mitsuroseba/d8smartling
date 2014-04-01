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
  $smartling_supported_explicitly = variable_get('language_content_type_' . $type, NULL);

  if ($smartling_supported_explicitly == 2 || $smartling_supported_explicitly == SMARTLING_KEY) {
    $smartling_supported_explicitly = TRUE;
  }
  else {
    $smartling_supported_explicitly = FALSE;
  }

  return $smartling_supported_explicitly;
}

function smartling_nodes_method($type) {
  $nodes_method = variable_get('language_content_type_' . $type, NULL) == 2;
  return $nodes_method;
}

function smartling_fields_method($type) {
  $fields_method = variable_get('language_content_type_' . $type, NULL) == SMARTLING_KEY;
  return $fields_method;
}

/**
 * Returns node status for nodes method by node id.
 */
function smartling_nodes_method_node_status($nid) {
  if (is_int($nid) || is_numeric($nid)) {
    $node = smartling_node_load_default($nid);
  }
  elseif (is_object($nid)) {
    $node = $nid;
  }

  if (isset($node->tnid) && $node->tnid != '0' && ($node->tnid != $node->nid)) {
    return SMARTLING_TRANSLATE_NODE;
  }
  elseif (isset($node->tnid) && $node->tnid != '0' && ($node->tnid == $node->nid)) {
    return SMARTLING_ORIGINAL_NODE;
  }
  elseif (isset($node->tnid) && $node->tnid == '0') {
    return SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE;
  }
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
 * Checks any required configuration parameters are missing.
 *
 * @return string|boolean
 */
function smartling_is_configured() {
  $required_variables = array(
    'smartling_api_url',
    'smartling_key',
    'smartling_project_id',
    'smartling_target_locales',
  );
  foreach ($required_variables as $required_variable) {
    $val = variable_get($required_variable, NULL);
    if (empty($val)) {
      if ($required_variable == 'smartling_api_url') {
        return FALSE;
      }
      elseif ($required_variable == 'smartling_key') {
        return FALSE;
      }
      elseif ($required_variable == 'smartling_project_id') {
        return FALSE;
      }
      elseif ($required_variable == 'smartling_target_locales') {
        return FALSE;
      }
    }
  }
  // All required configuration variables are set
  return TRUE;
}

/**
 * Checks smartling_translate_fields variable and check settings for content type.
 */
function smartling_translate_fields_is_set($type = FALSE) {
  $val = variable_get('smartling_translate_fields', NULL);

  if (is_null($val) || empty($val)) {
    $result = FALSE;
  }
  else {
    if ($type !== FALSE) {
      if (isset($val[$type])) {
        $result = TRUE;
      }
      else {
        $result = FALSE;
      }
    }
    else {
      $result = TRUE;
    }
  }
  if (!$result) {
    drupal_set_message(t('Please follow the page <a href="/admin/config/regional/smartling">"Smartling settings"</a> to setup Smartling configuration.'), 'warning');
  }

  return $result;
}

function smartling_get_translatable_field_types() {
  // What types of fields DO we translate?
  $included_fields = array(
    'text',
    'text_long',
    'text_textfield',
    'text_textarea',
    'text_textarea_with_summary',
    'image_image',
      //'options_select',
      //'options_buttons',
      //'taxonomy_autocomplete',
  );
  /* uncomment after more tests
    $included_fields = array('text', 'text_long', 'text_textfield', 'text_textarea', 'text_textarea_with_summary', 'field_collection_embed');
    if (module_exists('link')) {
    $included_fields[] = 'link_field';
    }
   */
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
 * @param type $file_name
 * @param type $nid
 */
function smartling_entity_data_filename_update($file_name, $nid, $s_locale, $translated_file = FALSE) {
  if (is_array($s_locale)) {
    if (!$translated_file) {
      foreach ($s_locale as $value) {
        $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'target_language' => $value));
        $entity_data->file_name = $file_name;
        smartling_entity_data_save($entity_data);
      }
    }
    else {
      foreach ($s_locale as $value) {
        $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'target_language' => $value));
        $entity_data->translated_file_name = $file_name;
        smartling_entity_data_save($entity_data);
      }
    }
  }
  else {
    if (!$translated_file) {
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'target_language' => $s_locale));
      $entity_data->file_name = $file_name;
      smartling_entity_data_save($entity_data);
    }
    else {
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'target_language' => $s_locale));
      $entity_data->translated_file_name = $file_name;
      smartling_entity_data_save($entity_data);
    }
  }
}

function smartling_get_full_language_name($s_locale) {
  $d_locale = smartling_convert_locale_drupal_to_smartling($s_locale, TRUE);

  /*
    $locales = array(
    'en' => t('English'),
    'es' => t('Spanish (International)'),
    'nl-NL' => t('Dutch (Netherlands)'),
    'ru-RU' => t('Russian (Russia)'),
    'zh-CN' => t('Chinese (Simplified)'),
    );
   */
  $languages = language_list();
  if (isset($languages[$d_locale])) {
    return $languages[$d_locale]->name;
  }
  else {
    return NULL;
  }
}

/**
 * This functions convert locale format.
 * Example: 'en' => 'en-US'.
 *
 * @param string $locale
 * Locale string in some foormat: 'en' or 'en-US'.
 *
 * @param bool $reverse
 * If TRUE, convert format: 'en-US' => 'en'.
 * FALSE by default.
 *
 * @return string|null
 */
function smartling_convert_locale_drupal_to_smartling($locale, $reverse = FALSE) {
  $locales = variable_get('smartling_locales_convert_array', array());
  if (!$reverse) {
    if (isset($locales[$locale])) {
      return $locales[$locale];
    }
    else {
      return NULL;
    }
  }
  else {
    foreach ($locales as $key => $loc) {
      if ($locale == $loc) {
        return $key;
      }
    }
  }
}

/**
 *
 * @param type $xml_doc
 */
function smartling_save_xml($xml_doc, $nid = NULL, $s_locale = FALSE, $file_name = NULL, $translated_file = FALSE) {
  $directory = SMARTLING_DIRECTORY;
  if (is_null($file_name)) {
    $file_name = 'smartling_translate_' . $nid . '.xml';
  }
  $file_name = file_munge_filename(preg_replace('@^.*/@', '', $file_name), '', TRUE);

  if ($translated_file !== FALSE) {
    $translated_file = file_munge_filename(preg_replace('@^.*/@', '', $translated_file), '', TRUE);
  }

  $path = $directory . '/' . $file_name;

  if (is_dir($directory) && is_writable($directory)) {
    $xml_doc->save($path);

    if ($translated_file == FALSE) {
      if (is_array($s_locale)) {
        smartling_watchdog('Smartling save original xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => implode('; ', $s_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save original xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }
    if ($translated_file == TRUE) {
      if (is_array($s_locale)) {
        smartling_watchdog('Smartling save translated xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => implode('; ', $s_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save translated xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }

    if (!is_null($nid)) {
      smartling_entity_data_filename_update($file_name, $nid, $s_locale, $translated_file);
    }
    return $xml_doc;
  }
  elseif (drupal_mkdir($directory) && drupal_chmod($directory)) {
    $xml_doc->save($path);

    if ($translated_file == FALSE) {
      if (is_array($s_locale)) {
        smartling_watchdog('Smartling save original xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => implode('; ', $s_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save original xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }
    if ($translated_file == TRUE) {
      if (is_array($s_locale)) {
        smartling_watchdog('Smartling save translated xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => implode('; ', $s_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save translated xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }

    if (!is_null($nid)) {
      smartling_entity_data_filename_update($file_name, $nid, $s_locale, $translated_file);
    }
    return $xml_doc;
  }
  else {
    smartling_watchdog('Smartling file is not be saved because of the some errors. Filename: @file_name, related entity - @nid', FALSE, array('@file_name' => $file_name, '@nid' => $nid), WATCHDOG_ERROR);
    drupal_set_message(t('File is not be saved because of the some errors.'), 'error');
    return NULL;
  }
}

/**
 *
 * @param type $nid
 * @return boolean
 */
function smartling_build_xml_by_node_id($rid = NULL, $s_locale, $title = '', $nodes_method = FALSE) {
  if (!is_null($rid)) {
    $node_content_to_translate = smartling_node_content_to_translate($rid, $nodes_method);
    $xml = new DOMDocument('1.0', 'UTF-8');

    $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string '));
    $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string '));

    $data = $xml->createElement('data');

    $data->appendChild(smartling_build_localize($xml, $rid, $node_content_to_translate));
    $xml->appendChild($data);

    $file_name = strtolower(trim(preg_replace('#\W+#', '_', $title), '_')) . '_' . $rid . '.xml';

    $xml = smartling_save_xml($xml, $rid, $s_locale, $file_name, FALSE);
    if (!is_null($xml)) {
      return $file_name;
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
 * @param object $dom
 * @param string $title
 * This param set value of attribute title
 * @param array $strings
 * This is array of strings to be create
 * @param string $index
 * This is index of localize or nolocalize tag. Such as localize2 or nolocalize3.
 * Is empty string by default.
 * @param bool $no
 * If TRUE tag -> nolocalize, if FALSE tag -> localize. Tag localize use by default.
 * @return type
 */
function smartling_build_localize($dom, $title = '', $strings = array(), $index = '', $no = FALSE) {
  $element = ($no !== FALSE) ? 'nolocalize' . $index : 'localize' . $index;

  $localize = $dom->createElement($element);
  $localize_attr = $dom->createAttribute('title');
  $localize_attr->value = $title;
  $localize->appendChild($localize_attr);

  foreach ($strings as $field_name => $field_value) {
    if (is_string($field_value)) {
      // Simple field
      $string = $dom->createElement('string');
      $string_val = $dom->createTextNode($field_value);
      $string_attr = $dom->createAttribute('id');
      $string_attr->value = $field_name;
      $string->appendChild($string_attr);
      $string->appendChild($string_val);
      $localize->appendChild($string);
    }
    elseif (is_array($field_value)) {
      if (count($field_value) >= 1) {
        // Fields body-summary or image or multifields
        if (isset($field_value['body'])) {
          // Field body-summary
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($field_value['body']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-body';
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          $localize->appendChild($string);

          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($field_value['summary']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-summary';
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          $localize->appendChild($string);
        }
        elseif (isset($field_value['alt-img'])) {
          // Field image
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($field_value['alt-img']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-alt-img';
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set image fid
          $string_attr = $dom->createAttribute('fid');
          $string_attr->value = $field_value['fid-img'];
          $string->appendChild($string_attr);
          $localize->appendChild($string);

          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($field_value['title-img']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-title-img';
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set image fid
          $string_attr = $dom->createAttribute('fid');
          $string_attr->value = $field_value['fid-img'];
          $string->appendChild($string_attr);
          $localize->appendChild($string);
        }
        else {
          if (!is_string($field_value[0]) && isset($field_value[0]['body'])) {
            // Multifields body-summary
            $quantity = count($field_value);
            foreach ($field_value as $key => $value) {
              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value['body']);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_name . '-body' . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // Set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              $localize->appendChild($string);

              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value['summary']);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_name . '-summary' . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // Set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              $localize->appendChild($string);
            }
          }
          elseif (!is_string($field_value[0]) && isset($field_value[0]['alt-img'])) {
            // Multifields image
            $quantity = count($field_value);
            foreach ($field_value as $key => $value) {
              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value['alt-img']);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_name . '-alt-img' . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // Set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              // Set image fid
              $string_attr = $dom->createAttribute('fid');
              $string_attr->value = $value['fid-img'];
              $string->appendChild($string_attr);
              $localize->appendChild($string);

              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value['title-img']);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_name . '-title-img' . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // Set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              // Set image fid
              $string_attr = $dom->createAttribute('fid');
              $string_attr->value = $value['fid-img'];
              $string->appendChild($string_attr);
              $localize->appendChild($string);
            }
          }
          elseif (is_string($field_value[0])) {
            // Multifields
            $quantity = count($field_value);
            foreach ($field_value as $key => $value) {
              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_name . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // Set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              $localize->appendChild($string);
            }
          }
        }
      }
    }
  }

  return $localize;
}

/**
 *
 * @param type $rid
 * @param type $s_locale
 * @param type $entity_data
 * @return int
 */
function smartling_check_translate($rid, $s_locale = '', $entity_data = NULL) {
  smartling_is_configured();

  if (is_null($entity_data)) {
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
  }
  $file_name = $entity_data->file_name;
  $file_name_unic = $entity_data->file_name;
  $file_uri = file_create_url('public://smartling/' . $file_name);

  $key = variable_get('smartling_key');
  $project_id = variable_get('smartling_project_id');
  // Init api object
  $api = new SmartlingAPI(SMARTLING_API_URL, $key, $project_id, SMARTLING_PRODUCTION_MODE);

  // Try to retrieve file status
  $status_result = $api->getStatus($file_name_unic, $s_locale);
  $status_result = json_decode($status_result);

  // This is a get status
  if ($api->getCodeStatus() == 'SUCCESS') {

    if ($status_result->response->data->approvedStringCount !== 0) {
      $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);
    }
    else {
      $progress = 100;
    }
    smartling_watchdog('Smartling progress status for node id - @nid (@s_locale). approvedString = @as, completedString = @cs', TRUE, array(
      '@nid' => $rid,
      '@s_locale' => $s_locale,
      '@as' => $status_result->response->data->approvedStringCount,
      '@cs' => $status_result->response->data->completedStringCount
        ), WATCHDOG_INFO, l(t('View original file'), $file_uri));

    return $progress;
  }
  else {
    smartling_watchdog('Smartling try to check progress file: <br/>
      Project Id: @project_id <br/>
      Action: status <br/>
      URI: @file_uri <br/>
      Locale: @s_locale <br/>
      Error: response code -> @code and message -> @message', FALSE, array('@project_id' => $project_id, '@file_uri' => $file_uri, '@s_locale' => $s_locale, '@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $rid));
    drupal_set_message(t('Smartling try to check progress file: response code - @code, message - @message', array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0])), 'error');
    drupal_goto(current_path());
  }
}

/**
 *
 * @param type $rid
 * @param type $s_locale
 * @param type $entity_data
 * @return int
 */
function smartling_download_translate($rid, $s_locale = '', $entity_data = NULL, $nodes_method = FALSE) {
  smartling_is_configured();
  $report_page = FALSE;

  // Load entity
  if (is_null($entity_data)) {
    if (is_numeric($s_locale)) {
      // Use in views report (Download)
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'eid' => $s_locale));
      $s_locale = $entity_data->target_language;
      $report_page = TRUE;
    }
    else {
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
    }
  }

  $nodes_method = smartling_nodes_method($entity_data->bundle);

  if (is_string($entity_data->file_name) && (strlen($entity_data->file_name) > 0 )) {

    $file_name = $entity_data->file_name;
    $file_name_unic = $entity_data->file_name;
    $file_uri = file_create_url('public://smartling/' . $file_name);
    $file_name = substr($file_name, 0, strlen($file_name) - 4);

    $translated_filename = $file_name . '_' . $s_locale . '.xml';

    $key = variable_get('smartling_key');
    $project_id = variable_get('smartling_project_id');
    // Init api object
    $api = new SmartlingAPI(SMARTLING_API_URL, $key, $project_id, SMARTLING_PRODUCTION_MODE);

    $retrieval_type = variable_get('smartling_retrieval_type');
    $download_param = array(
      'retrievalType' => $retrieval_type,
    );

    // Try to retrieve file status
    $status_result = $api->getStatus($file_name_unic, $s_locale);
    $status_result = json_decode($status_result);

    // This is a get status
    if ($api->getCodeStatus() == 'SUCCESS') {
      // Try to download file
      $download_result = $api->downloadFile($file_name_unic, $s_locale, $download_param);
      // This is a $download_result
      $xml = new DOMDocument();
      $xml->loadXML($download_result);
      // Save result
      $save = smartling_save_xml($xml, $rid, $s_locale, $translated_filename, TRUE);
      // Calc progress
      if ($status_result->response->data->approvedStringCount !== 0) {
        $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);
      }
      else {
        $progress = 100;
      }

      // If result is saved
      if (is_object($save)) {
        smartling_watchdog('Smartling progress status for node id - @nid (@s_locale). approvedString = @as, completedString = @cs', TRUE, array(
          '@nid' => $rid,
          '@s_locale' => $s_locale,
          '@as' => $status_result->response->data->approvedStringCount,
          '@cs' => $status_result->response->data->completedStringCount
            ), WATCHDOG_INFO, l(t('View original file'), $file_uri));

        smartling_update_translated_fields($rid, $s_locale, $nodes_method);

        $entity_data->progress = $progress;
        smartling_entity_data_save($entity_data);

        if (isset($_GET['destination']) && $_GET['destination'] != current_path() && !$report_page) {
          // Redirect user back to node page
          $variant = 'back to node page';
        }
        elseif (isset($_GET['destination']) && $_GET['destination'] != current_path() && $report_page) {
          // Redirect user back to report page
          $variant = 'back to report page';
        }
        else {
          // If use for download end get progress
          $variant = 'download end get progress';
        }
      }
    }
    else {
      $variant = 'status fail';
    }
  }
  else {
    $variant = 'exception';
  }

  switch ($variant) {
    case 'back to node page':
      unset($_GET['destination']);
      drupal_set_message(t('Downloaded for language translation @language', array('@language' => $s_locale)));
      drupal_goto(current_path());
      exit();
      break;

    case 'back to report page':
      drupal_set_message(t('Downloaded for language translation @language', array('@language' => $s_locale)));
      drupal_goto($_GET['destination']);
      exit();
      break;

    case 'download end get progress':
      return $progress;
      break;

    case 'status fail':
      smartling_watchdog('Smartling try to download file: <br/>
      Project Id: @project_id <br/>
      Action: download <br/>
      URI: @file_uri <br/>
      Locale: @s_locale <br/>
      Error: response code -> @code and message -> @message', FALSE, array('@project_id' => $project_id, '@file_uri' => $file_uri, '@s_locale' => $s_locale, '@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $rid));
      drupal_set_message(t('Smartling try to download file: response code - @code, message - @message', array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0])), 'error');
      drupal_goto(current_path());
      break;

    case 'exception':
      smartling_watchdog('Node can not be download at a moment since it is still in a queue for upload. Id: @nid. Locale: @locale', TRUE, array('@nid' => $rid, '@locale' => $s_locale), WATCHDOG_INFO);
      $args = arg();
      if (($args[0] == 'smartling') && ($args[1] == 'download')) {
        drupal_set_message(t('Node can not be download at a moment since it is still in a queue for upload. Id: @nid. Locale: @locale', array('@nid' => $rid, '@locale' => $s_locale)), 'warning');
        drupal_goto('admin/content/smartling/report-in-progress');
      }
      return 'exception';
      break;

    default:
      break;
  }
}

function smartling_language_options_list() {
  $language_options_list = array();
  $languages = smartling_language_list();

  foreach ($languages as $langcode => $language) {
    if ($language->enabled != '0') {
      $language_options_list[$langcode] = $language->name;
    }
  }
  return $language_options_list;
}

function smartling_language_list() {
  $languages_list = array();
  $languages = language_list('language');

  $smartling_target_locales = variable_get('smartling_target_locales', array());

  foreach ($languages as $langcode => $language) {
    if ($language->enabled != '0' && (in_array($langcode, $smartling_target_locales))) {
      $languages_list[$langcode] = $language;
    }
  }
  return $languages_list;
}

/**
 * Smartling watchdog function.
 *
 * @param string $message
 * @param bool $consider_log
 * If FALSE, smartling not take into account the setting of logging. TRUE by default.
 * @param array $variables
 * @param int $severity
 * WATCHDOG_INFO by default.
 * @param string $link
 */
function smartling_watchdog($message, $consider_log = TRUE, $variables = array(), $severity = WATCHDOG_INFO, $link = NULL) {
  if ($consider_log) {
    if (SMARTLING_LOG) {
      watchdog('smartling', $message, $variables, $severity, $link);
    }
  }
  else {
    watchdog('smartling', $message, $variables, $severity, $link);
  }
}

function smartling_set_translation_status($entity_data = NULL, $event = NULL) {
  if (!is_null($entity_data) && !is_null($event)) {
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $entity_data->rid, 'target_language' => $entity_data->target_language));
    switch ($event) {
      case SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE:
        if (empty($entity_data->status) || ($entity_data->status == SMARTLING_STATUS_CHANGE)) {
          $entity_data->status = SMARTLING_STATUS_IN_QUEUE;
          smartling_entity_data_save($entity_data);
        }
        break;

      case SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE:
        if ($entity_data->status != SMARTLING_STATUS_CHANGE) {
          $entity_data->status = SMARTLING_STATUS_IN_TRANSLATE;
          smartling_entity_data_save($entity_data);
        }
        break;

      case SMARTLING_STATUS_EVENT_DOWNLOAD_FROM_SERVICE:
      case SMARTLING_STATUS_EVENT_UPDATE_FIELDS:
        if ($entity_data->status != SMARTLING_STATUS_CHANGE) {
          if ($entity_data->progress == 100) {
            $entity_data->status = SMARTLING_STATUS_TRANSLATED;
          }
          smartling_entity_data_save($entity_data);
        }
        break;

      case SMARTLING_STATUS_EVENT_NODE_ENTITY_UPDATE:
        $entity_data->status = SMARTLING_STATUS_CHANGE;
        smartling_entity_data_save($entity_data);
        break;

      default:
        break;
    }
  }
}

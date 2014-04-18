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
function smartling_supported_type($entity_type, $bundle) {
  $smartling_supported_explicitly = FALSE;

  switch ($entity_type) {
    case 'node':
      $smartling_supported_explicitly = variable_get('language_content_type_' . $bundle, NULL);

      if ($smartling_supported_explicitly == SMARTLING_NODES_METHOD_KEY || $smartling_supported_explicitly == SMARTLING_FIELDS_METHOD_KEY) {
        $smartling_supported_explicitly = TRUE;
      }
      else {
        $smartling_supported_explicitly = FALSE;
      }
      break;

    case 'taxonomy_term':
      // If term not exist, need create new node
      $vocabulary = taxonomy_vocabulary_machine_name_load($bundle);
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary->vid);
      switch ($vocabulary_mode) {
        case I18N_MODE_TRANSLATE:
          $smartling_supported_explicitly = TRUE;
          break;

        default:
          $smartling_supported_explicitly = FALSE;
          break;
      }
      break;

    case 'comment':
      $node_type = str_ireplace('comment_node_', '', $bundle);
      if (smartling_fields_method($node_type)) {
        $smartling_supported_explicitly = smartling_entity_has_translatable_field($entity_type, $bundle);
      }
      break;

    case 'user':
      $smartling_supported_explicitly = smartling_entity_has_translatable_field($entity_type, $bundle);
      break;

    default:
      $smartling_supported_explicitly = FALSE;
      break;
  }

  return $smartling_supported_explicitly;
}

function smartling_entity_has_translatable_field($entity_type, $bundle) {
  $translatable = FALSE;
  foreach (field_info_instances($entity_type, $bundle) as $instance) {
    $field_name = $instance['field_name'];
    $field = field_info_field($field_name);
    if ($field['translatable']) {
      $translatable = TRUE;
      break;
    }
  }
  return $translatable;
}

function smartling_nodes_method($type) {
  $nodes_method = variable_get('language_content_type_' . $type, NULL) == SMARTLING_NODES_METHOD_KEY;
  return $nodes_method;
}

function smartling_fields_method($type) {
  $fields_method = variable_get('language_content_type_' . $type, NULL) == SMARTLING_FIELDS_METHOD_KEY;
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
function smartling_entity_data_filename_update($file_name, $rid, $d_locale, $translated_file = FALSE, $entity_type = FALSE) {
  if (is_array($d_locale)) {
    if (!$translated_file) {
      foreach ($d_locale as $value) {
        $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'entity_type' => $entity_type, 'target_language' => $value));
        $entity_data->file_name = $file_name;
        smartling_entity_data_save($entity_data);
      }
    }
    else {
      foreach ($d_locale as $value) {
        $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'entity_type' => $entity_type, 'target_language' => $value));
        $entity_data->translated_file_name = $file_name;
        smartling_entity_data_save($entity_data);
      }
    }
  }
  else {
    if (!$translated_file) {
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'entity_type' => $entity_type, 'target_language' => $d_locale));
      $entity_data->file_name = $file_name;
      smartling_entity_data_save($entity_data);
    }
    else {
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'entity_type' => $entity_type, 'target_language' => $d_locale));
      $entity_data->translated_file_name = $file_name;
      smartling_entity_data_save($entity_data);
    }
  }
}

function smartling_get_full_language_name($d_locale) {
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
function smartling_save_xml($xml_doc, $rid = NULL, $d_locale = FALSE, $file_name = NULL, $translated_file = FALSE, $entity_type = NULL) {
  $directory = SMARTLING_DIRECTORY;
  if (is_null($file_name)) {
    $file_name = 'smartling_translate_' . $entity_type . '_' . $rid . '.xml';
  }
  $file_name = file_munge_filename(preg_replace('@^.*/@', '', $file_name), '', TRUE);

  if ($translated_file !== FALSE) {
    $translated_file = file_munge_filename(preg_replace('@^.*/@', '', $translated_file), '', TRUE);
  }

  $path = $directory . '/' . $file_name;

  if (is_dir($directory) && is_writable($directory)) {
    $xml_doc->save($path);

    if ($translated_file == FALSE) {
      if (is_array($d_locale)) {
        smartling_watchdog('Smartling save original xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => implode('; ', $d_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save original xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => $d_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }
    if ($translated_file == TRUE) {
      if (is_array($d_locale)) {
        smartling_watchdog('Smartling save translated xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => implode('; ', $d_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save translated xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => $d_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }

    if (!is_null($rid)) {
      smartling_entity_data_filename_update($file_name, $rid, $d_locale, $translated_file, $entity_type);
    }
    return $xml_doc;
  }
  elseif (drupal_mkdir($directory) && drupal_chmod($directory)) {
    $xml_doc->save($path);

    if ($translated_file == FALSE) {
      if (is_array($d_locale)) {
        smartling_watchdog('Smartling save original xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => implode('; ', $d_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save original xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => $d_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }
    if ($translated_file == TRUE) {
      if (is_array($d_locale)) {
        smartling_watchdog('Smartling save translated xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => implode('; ', $d_locale)), WATCHDOG_INFO, l(t('View file'), $path));
      }
      else {
        smartling_watchdog('Smartling save translated xml file for entity_type - @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => $d_locale), WATCHDOG_INFO, l(t('View file'), $path));
      }
    }

    if (!is_null($rid)) {
      smartling_entity_data_filename_update($file_name, $rid, $d_locale, $translated_file, $entity_type);
    }
    return $xml_doc;
  }
  else {
    smartling_watchdog('Smartling file was not saved because of some errors. Filename: @file_name, related entity - @rid', FALSE, array('@file_name' => $file_name, '@rid' => $rid), WATCHDOG_ERROR);
    drupal_set_message(t('File was not saved because of some errors.'), 'error');
    return NULL;
  }
}

/**
 *
 * @param type $rid
 * @param type $d_locale
 * @param type $title
 * @param type $entity_type
 * @return boolean|string
 */
function smartling_build_xml($rid = NULL, $d_locale, $title = '', $entity_type = NULL) {
  if (!is_null($rid)) {

    switch ($entity_type) {
      case 'node':
        $content_to_translate = smartling_node_content_to_translate($rid);
        break;

      case 'taxonomy_term':
        $content_to_translate = smartling_term_content_to_translate($rid);
        break;

      case 'comment':
        $content_to_translate = smartling_comment_content_to_translate($rid);
        break;

      case 'user':
        $content_to_translate = smartling_user_content_to_translate($rid);
        break;

      default:
        return FALSE;
        break;
    }

    $xml = new DOMDocument('1.0', 'UTF-8');

    $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string '));
    $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string '));

    $data = $xml->createElement('data');

    $data->appendChild(smartling_build_localize($xml, $rid, $content_to_translate));
    $xml->appendChild($data);

    $file_name = strtolower(trim(preg_replace('#\W+#', '_', $title), '_')) . '_' . $entity_type . '_' . $rid . '.xml';

    $xml = smartling_save_xml($xml, $rid, $d_locale, $file_name, FALSE, $entity_type);
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
    if (is_array($field_value)) {
      // Fields body-summary or image or text
      if (!is_string($field_value[0]) && isset($field_value[0]['body'])) {
        // Field body-summary
        $quantity = count($field_value);
        foreach ($field_value as $key => $value) {
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value['body']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-body' . '-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity
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
          // Set quantity
          $string_attr = $dom->createAttribute('quantity');
          $string_attr->value = $quantity;
          $string->appendChild($string_attr);
          $localize->appendChild($string);
        }
      }
      elseif (!is_string($field_value[0]) && isset($field_value[0]['alt-img'])) {
        // Field image
        $quantity = count($field_value);
        foreach ($field_value as $key => $value) {
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value['alt-img']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-alt-img' . '-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity
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
          // Set quantity
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
        // Field text
        $quantity = count($field_value);
        foreach ($field_value as $key => $value) {
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity
          $string_attr = $dom->createAttribute('quantity');
          $string_attr->value = $quantity;
          $string->appendChild($string_attr);
          $localize->appendChild($string);
        }
      }
    }
  }

  return $localize;
}

/**
 *
 * @param type $rid
 * @param type $d_locale
 * @param type $entity_data
 */
function smartling_download_translate($rid, $d_locale = '', $entity_data = NULL) {
  if (smartling_is_configured()) {
    $report_page = FALSE;

    // Load entity
    if (is_null($entity_data) && is_numeric($d_locale)) {
      // Use in views report (Download)
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'eid' => $d_locale));
      $d_locale = $entity_data->target_language;
      $report_page = TRUE;
    }

    if (($entity_data !== FALSE) && (smartling_supported_type($entity_data->entity_type, $entity_data->bundle))) {
      $s_locale = smartling_convert_locale_drupal_to_smartling($entity_data->target_language);
      $progress = smartling_get_progress_status($entity_data);

      if ($progress !== FALSE) {
        $xml = smartling_download_file($entity_data);

        $file_name = substr($entity_data->file_name, 0, strlen($entity_data->file_name) - 4);
        $translated_filename = $file_name . '_' . $s_locale . '.xml';

        // Save result
        $save = smartling_save_xml($xml, $rid, $d_locale, $translated_filename, TRUE, $entity_data->entity_type);

        // If result is saved
        if (is_object($save)) {
          smartling_update_translated_fields($rid, $d_locale, $entity_data->entity_type, $entity_data->bundle);
          $entity_data->progress = $progress;
          smartling_entity_data_save($entity_data);
          $result = t('Downloaded');
        }
      }
      else {
        $result = t('Progress status Failed');
      }
    }
    else {
      $result = t('Progress status Failed');
    }
  }
  else {
    $result = t('Smartling not configured');
  }

  drupal_set_message(t('@result for language translation @language', array('@result' => $result, '@language' => $d_locale)));
}

function smartling_download_file($entity_data) {
  $file_name_unic = $entity_data->file_name;
  $s_locale = smartling_convert_locale_drupal_to_smartling($entity_data->target_language);

  $key = variable_get('smartling_key');
  $project_id = variable_get('smartling_project_id');
  // Init api object
  $api = new SmartlingAPI(SMARTLING_API_URL, $key, $project_id, SMARTLING_PRODUCTION_MODE);

  $retrieval_type = variable_get('smartling_retrieval_type');
  $download_param = array(
    'retrievalType' => $retrieval_type,
  );

  // Try to download file
  $download_result = $api->downloadFile($file_name_unic, $s_locale, $download_param);
  // This is a $download_result
  $xml = new DOMDocument();
  $xml->loadXML($download_result);

  return $xml;
}

function smartling_get_progress_status($entity_data) {
  if (is_string($entity_data->file_name) && (strlen($entity_data->file_name) > 0 )) {

    $link_to_entity = smartling_get_link_to_entity($entity_data->rid, $entity_data->entity_type);

    $d_locale = $entity_data->target_language;
    $s_locale = smartling_convert_locale_drupal_to_smartling($d_locale);
    $file_name_unic = $entity_data->file_name;
    $file_uri = file_create_url('public://smartling/' . $entity_data->file_name);

    $key = variable_get('smartling_key');
    $project_id = variable_get('smartling_project_id');
    // Init api object
    $api = new SmartlingAPI(SMARTLING_API_URL, $key, $project_id, SMARTLING_PRODUCTION_MODE);

    // Try to retrieve file status
    $status_result = $api->getStatus($file_name_unic, $s_locale);
    $status_result = json_decode($status_result);

    // This is a get status
    if ($api->getCodeStatus() == 'SUCCESS') {
      // Calc progress
      if ($status_result->response->data->approvedStringCount !== 0) {
        $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);
      }
      else {
        $progress = 100;
      }

      smartling_watchdog('Smartling progress status for entity id - @rid (@d_locale). approvedString = @as, completedString = @cs, progress = @progress', TRUE, array(
        '@rid' => $entity_data->rid,
        '@d_locale' => $d_locale,
        '@as' => $status_result->response->data->approvedStringCount,
        '@cs' => $status_result->response->data->completedStringCount,
        '@progress' => $progress,
          ), WATCHDOG_INFO, l(t('View original file'), $file_uri));
      return $progress;
    }
    else {
      smartling_watchdog('Smartling tried to check file progress: <br/>
      Project Id: @project_id <br/>
      Action: get progress <br/>
      URI: @file_uri <br/>
      Locale: @d_locale <br/>
      Error: response code -> @code and message -> @message', FALSE, array('@project_id' => $project_id, '@file_uri' => $file_uri, '@d_locale' => $d_locale, '@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, $link_to_entity);
      drupal_set_message(t('Smartling tried to check file progress: response code - @code, message - @message', array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0])), 'error');
      return FALSE;
    }
  }
  else {
    return FALSE;
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
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $entity_data->rid, 'entity_type' => $entity_data->entity_type, 'target_language' => $entity_data->target_language));
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

function smartling_get_link_to_entity($rid, $entity_type) {
  switch ($entity_type) {
    case 'node':
      $link = l(t('Related entity'), 'node/' . $rid);
      break;

    case 'taxonomy_term':
      $link = l(t('Related entity'), 'taxonomy/term/' . $rid);
      break;

    case 'comment':
      $link = l(t('Related entity'), 'comment/' . $rid);
      break;

    case 'user':
      $link = l(t('Related entity'), 'user/' . $rid);
      break;

    default:
      $link = '-';
      break;
  }

  return $link;
}

function smartling_get_original_entity($rid, $entity_type) {
  switch ($entity_type) {
    case 'node':
      $entity = FALSE;
      break;

    case 'taxonomy_term':
      $term = taxonomy_term_load($rid);
      if ($term->language == language_default()->language) {
        $entity = $term;
      }
      else {
        if (!isset($term->i18n_tsid)) {
          $entity = FALSE;
        }
        elseif (isset($term->i18n_tsid) && ($term->i18n_tsid == FALSE)) {
          $entity = FALSE;
        }
        elseif (isset($term->i18n_tsid) && is_numeric($term->i18n_tsid)) {
          $entity = i18n_taxonomy_term_get_translation($term, language_default()->language);
        }
      }
      break;

    case 'comment':
      $entity = FALSE;
      break;

    case 'user':
      $entity = FALSE;
      break;

    default:
      $entity = FALSE;
      break;
  }

  return $entity;
}

function smartling_get_supported_field_names_by_type_and_bundle($entity_type, $bundle) {
  $field_names = array();
  $entity_fields = field_info_instances($entity_type, $bundle);
  $translatable_field_types = smartling_get_translatable_field_types();

  foreach ($entity_fields as $field) {
    $field_info = field_info_field($field['field_name']);
    $field_type = $field['widget']['type'];
    if ((array_search($field_type, $translatable_field_types)) && ($field_info['translatable'])) {
      $field_names[] = $field['field_name'];
    }
  }

  return $field_names;
}

function smartling_get_field_format($entity_wrapper, $field_name, $original_language, $target_locale, $i = 0) {
  if (isset($entity_wrapper->{$field_name}[$original_language][$i]['format'])) {
    $format = $entity_wrapper->{$field_name}[$original_language][$i]['format'];
  }
  elseif (isset($entity_wrapper->{$field_name}[$target_locale][$i]['format'])) {
    $format = $entity_wrapper->{$field_name}[$target_locale][$i]['format'];
  }
  else {
    $format = NULL;
  }

  return $format;
}

/**
 * Translation handler factory.
 *
 * @param type $entity_type
 * @param type $entity
 * @return \class
 */
function smartling_entity_translation_get_handler($entity_type, $entity) {
  $entity_info = entity_get_info($entity_type);
  $class = 'SmartlingEntityTranslationDefaultHandler';
  // @todo remove fourth parameter once 3rd-party translation handlers have
  // been fixed and no longer require the deprecated entity_id parameter.
  $handler = new $class($entity_type, $entity_info, $entity, NULL);
  return $handler;
}

function smartling_entity_translation_save($handler, $entity) {
  $handler->saveTranslations();
  // Update the wrapped entity
  $handler->setEntity($entity);
  $handler->smartlingEntityTranslationFieldAttach();
}

function smartling_check_filename_length($title, $entity_type, $rid) {

  $file_name = strtolower(trim(preg_replace('#\W+#', '_', $title), '_')) . '_' . $entity_type . '_' . $rid . '.xml';

  if (strlen($file_name) > 255) {
    $count = (strlen($file_name) - 240);
    $link_to_entity = smartling_get_link_to_entity($rid, $entity_type);

    smartling_watchdog('Generated file name is too long. Remove approximately @count characters. Filename: @file_name, related entity - @rid, entity type - @entity_type', FALSE, array('@count' => $count, '@file_name' => $file_name, '@rid' => $rid, '@entity_type' => $entity_type), WATCHDOG_ERROR, $link_to_entity);
    return FALSE;
  }
  return TRUE;
}

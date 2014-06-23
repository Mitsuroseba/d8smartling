<?php

/**
 * @file
 * Utils functions for Smartling module.
 */

/**
 * Returns whether the given node type has support for Smartling translations.
 *
 * @param string $entity_type
 *   Entity type.
 * @param string $bundle
 *   Entity bundle.
 *
 * @return bool
 *   Return TRUE if smartling support this bundle.
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
      $vocabulary = taxonomy_vocabulary_machine_name_load($bundle);
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
      switch ($vocabulary_mode) {
        case I18N_MODE_TRANSLATE:
          $smartling_supported_explicitly = TRUE;
          break;

        case I18N_MODE_LOCALIZE:
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

/**
 * Checks entity. If entity have translatable field return TRUE.
 *
 * @param string $entity_type
 *   Entity type.
 * @param string $bundle
 *   Entity bundle.
 *
 * @return bool
 *   Return TRUE if entity has translatable fields.
 */
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

/**
 * Checks node method.
 *
 * @param string $type
 *   Node content type.
 *
 * @return bool
 *   Return TRUE if this node type set in nodes method translate.
 */
function smartling_nodes_method($type) {
  $nodes_method = variable_get('language_content_type_' . $type, NULL) == SMARTLING_NODES_METHOD_KEY;
  return $nodes_method;
}

/**
 * Checks fields method.
 *
 * @param string $type
 *   Node content type.
 *
 * @return bool
 *   Return TRUE if this node type set in fields method translate.
 */
function smartling_fields_method($type) {
  $fields_method = variable_get('language_content_type_' . $type, NULL) == SMARTLING_FIELDS_METHOD_KEY;
  return $fields_method;
}

/**
 * Returns node status for nodes method by node id.
 *
 * @param int $nid
 *   Node id.
 *
 * @return string
 *   Return unical node status for nodes method translate.
 */
function smartling_nodes_method_node_status($nid) {
  if (is_int($nid) || is_numeric($nid)) {
    $node = node_load($nid);
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
 * Checks any required configuration parameters are missing.
 *
 * @return bool
 *   Return TRUE if configuration parameters is set.
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
    if (empty($val) && in_array($required_variable, array(
        'smartling_api_url',
        'smartling_key',
        'smartling_project_id',
        'smartling_target_locales',
      ))
    ) {
      drupal_set_message(t('Please follow the page <a href="@link">"Smartling settings"</a> to setup Smartling configuration.', array('@link' => url('admin/config/regional/smartling'))), 'warning');
      return FALSE;
    }
  }
  // All required configuration variables are set.
  return TRUE;
}

/**
 * Checks settings for node content type.
 *
 * @param string $type
 *   Node content type.
 *
 * @return bool
 *   Return TRUE if translate fields is set.
 */
function smartling_translate_fields_is_set($type) {
  $val = smartling_settings_get_handler()->nodeGetFieldsSettingsByBundle($type);

  if (empty($val)) {
    $result = FALSE;
  }
  else {
    $result = TRUE;
  }
  if (!$result) {
    drupal_set_message(t('Please follow the page <a href="@link">"Smartling settings"</a> to setup Smartling configuration.', array('@link' => url('admin/config/regional/smartling'))), 'warning');
  }

  return $result;
}

/**
 * Return translatable field types.
 *
 * @return array
 *   Return translatable field types.
 */
function smartling_get_translatable_field_types() {
  // What types of fields DO we translate?
  $included_fields = array(
    'text',
    'text_long',
    'text_textfield',
    'text_textarea',
    'text_textarea_with_summary',
    'image_image',
    // 'options_select',
    // 'options_buttons',
    // 'taxonomy_autocomplete',
  );
  /* uncomment after more tests
   $included_fields = array(
   'text',
   'text_long',
   'text_textfield',
   'text_textarea',
   'text_textarea_with_summary',
   'field_collection_embed',
   );

   if (module_exists('link')) {
   $included_fields[] = 'link_field';
   }
   */
  return $included_fields;
}

/**
 * Content node types linked to 'translatable' fields.
 *
 * @return array
 *   Return node types with translatable fields.
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
 * Goes though ALL the fields in the system and gets the details.
 *
 * @return array
 *   Return translatable fields details.
 */
function smartling_translatable_node_field_details() {
  $fields = field_info_fields();
  $translatable_fields = array();

  foreach ($fields as $field) {
    foreach (array_keys($field['bundles']) as $type) {
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
 * Checks translatable field by field name.
 *
 * @param string $field_name
 *   Field name.
 *
 * @return bool
 *   Return TRUE if field is translatable.
 */
function smartling_field_is_translatable_by_field_name($field_name) {
  $field = field_info_field($field_name);
  return (bool) $field['translatable'];
}

/**
 * Update filename in smartling transaction.
 *
 * @param string $file_name
 *   File name.
 * @param int $rid
 *   Related id.
 * @param string $d_locale
 *   Locale in drupal format (ru, en).
 * @param bool $translated_file
 *   TRUE if save translated file. FALSE by default.
 * @param string $entity_type
 *   Entity type.
 */
function smartling_entity_data_filename_update($file_name, $rid, $d_locale, $translated_file = FALSE, $entity_type = '') {
  if (is_array($d_locale)) {
    if (!$translated_file) {
      foreach ($d_locale as $value) {
        $entity_data = smartling_entity_load_by_conditions(array(
          'rid' => $rid,
          'entity_type' => $entity_type,
          'target_language' => $value,
        ));
        $entity_data->file_name = $file_name;
        smartling_entity_data_save($entity_data);
      }
    }
    else {
      foreach ($d_locale as $value) {
        $entity_data = smartling_entity_load_by_conditions(array(
          'rid' => $rid,
          'entity_type' => $entity_type,
          'target_language' => $value,
        ));
        $entity_data->translated_file_name = $file_name;
        smartling_entity_data_save($entity_data);
      }
    }
  }
  else {
    if (!$translated_file) {
      $entity_data = smartling_entity_load_by_conditions(array(
        'rid' => $rid,
        'entity_type' => $entity_type,
        'target_language' => $d_locale,
      ));
      $entity_data->file_name = $file_name;
      smartling_entity_data_save($entity_data);
    }
    else {
      $entity_data = smartling_entity_load_by_conditions(array(
        'rid' => $rid,
        'entity_type' => $entity_type,
        'target_language' => $d_locale,
      ));
      $entity_data->translated_file_name = $file_name;
      smartling_entity_data_save($entity_data);
    }
  }
}

/**
 * Get full language name.
 *
 * @param string $d_locale
 *   Locale in drupal format (ru, en).
 *
 * @return string|null
 *   Return full language name or NULL.
 */
function smartling_get_full_language_name($d_locale) {
  $languages = language_list();
  if (isset($languages[$d_locale])) {
    return check_plain($languages[$d_locale]->name);
  }
  else {
    return NULL;
  }
}

/**
 * This functions convert locale format. Example: 'en' => 'en-US'.
 *
 * @param string $locale
 *   Locale string in some foormat: 'en' or 'en-US'.
 * @param bool $reverse
 *   If TRUE, convert format: 'en-US' => 'en'. FALSE by default.
 *
 * @return string|null
 *   Return locale or NULL.
 */
function smartling_convert_locale_drupal_to_smartling($locale, $reverse = FALSE) {
  $locales = smartling_settings_get_handler()->getLocalesConvertArray();
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
 * Get smartling directory.
 *
 * @param string $file_name
 *   File name.
 *
 * @return string
 *   Return path for smartling dir.
 */
function smartling_get_dir($file_name = '') {
  $smartling_dir = (variable_get('file_private_path', FALSE)) ? (variable_get('file_private_path') . '/smartling') : (variable_get('file_public_path', conf_path() . '/files') . '/smartling');
  $smartling_dir .= (empty($file_name)) ? '' : '/' . $file_name;
  return (string) $smartling_dir;
}

/**
 * Return clean filename, sanitized for path traversal vulnerability.
 *
 * Url (https://code.google.com/p/teenage-mutant-ninja-turtles
 * /wiki/AdvancedObfuscationPathtraversal).
 *
 * @param string $filename
 *   File name.
 * @param bool $allow_dirs
 *   TRUE if allow dirs. FALSE by default.
 *
 * @return string
 *   Return clean filename.
 */
function smartling_clean_filename($filename, $allow_dirs = FALSE) {
  // Prior to PHP 5.5, empty() only supports variables.
  // (http://www.php.net/manual/en/function.empty.php).
  $trim_filename = trim($filename);
  if (empty($trim_filename)) {
    return '';
  }

  $pattern = '/[^a-zA-Z0-9_\-]/i';
  $info = pathinfo(trim($filename));
  $filename = preg_replace($pattern, '_', $info['filename']);
  if (isset($info['extension']) && !empty($info['extension'])) {
    $filename .= '.' . preg_replace($pattern, '_', $info['extension']);
  }

  if ($allow_dirs && isset($info['dirname']) && !empty($info['dirname'])) {
    $filename = preg_replace('/[^a-zA-Z0-9_\/\-]/i', '_', $info['dirname']) . '/' . $filename;
  }

  return (string) $filename;
}

/**
 * Save xml document.
 *
 * @param object $xml_doc
 *   Xml document.
 * @param int $rid
 *   Related id.
 * @param string $d_locale
 *   Locale in drupal format (ru, en).
 * @param string $file_name
 *   File name.
 * @param bool $translated_file
 *   TRUE if save translated file. FALSE by default.
 * @param string $entity_type
 *   Entity type.
 *
 * @return object|null
 *   Return xml document or NULL.
 */
function smartling_save_xml($xml_doc, $rid = NULL, $d_locale = FALSE, $file_name = NULL, $translated_file = FALSE, $entity_type = NULL) {
  $log = new SmartlingLog();

  if (is_null($file_name)) {
    $file_name = 'smartling_translate_' . $entity_type . '_' . $rid . '.xml';
  }

  $file_name = file_munge_filename(preg_replace('@^.*/@', '', $file_name), '', TRUE);
  $directory = smartling_get_dir();
  $path = smartling_clean_filename($directory . '/' . $file_name, TRUE);
  $file_type = 'original';

  if (is_dir($directory) && is_writable($directory)) {
    $xml_doc->save($path);

    if ($translated_file == TRUE) {
      $file_type = 'translated';
    }

    if (is_array($d_locale)) {
      $log->setMessage('Smartling saves @file_type xml file for entity_type - @entity_type, id - @rid. Locale: @locale')
        ->setVariables(array(
          '@file_type' => $file_type,
          '@entity_type' => $entity_type,
          '@rid' => $rid,
          '@locale' => implode('; ', $d_locale),
        ))
        ->setLink(l(t('View file'), $path))
        ->execute();
    }
    else {
      $log->setMessage('Smartling saves @file_type xml file for entity_type - @entity_type, id - @rid. Locale: @locale')
        ->setVariables(array(
          '@file_type' => $file_type,
          '@entity_type' => $entity_type,
          '@rid' => $rid,
          '@locale' => $d_locale,
        ))
        ->setLink(l(t('View file'), $path))
        ->execute();
    }

    if (!is_null($rid)) {
      smartling_entity_data_filename_update($file_name, $rid, $d_locale, $translated_file, $entity_type);
    }
    return $xml_doc;
  }
  elseif (drupal_mkdir($directory) && drupal_chmod($directory)) {
    $xml_doc->save($path);

    if ($translated_file == TRUE) {
      $file_type = 'translated';
    }

    if (is_array($d_locale)) {
      $log->setMessage('Smartling saves @file_type xml file for entity_type - @entity_type, id - @rid. Locale: @locale')
        ->setVariables(array(
          '@file_type' => $file_type,
          '@entity_type' => $entity_type,
          '@rid' => $rid,
          '@locale' => implode('; ', $d_locale),
        ))
        ->setLink(l(t('View file'), $path))
        ->execute();
    }
    else {
      $log->setMessage('Smartling saves @file_type xml file for entity_type - @entity_type, id - @rid. Locale: @locale')
        ->setVariables(array(
          '@file_type' => $file_type,
          '@entity_type' => $entity_type,
          '@rid' => $rid,
          '@locale' => $d_locale,
        ))
        ->setLink(l(t('View file'), $path))
        ->execute();
    }

    if (!is_null($rid)) {
      smartling_entity_data_filename_update($file_name, $rid, $d_locale, $translated_file, $entity_type);
    }
    return $xml_doc;
  }
  else {
    $log->setMessage('Smartling file was not saved because of some errors. Filename: @file_name, related entity - @rid')
      ->setVariables(array('@file_name' => $file_name, '@rid' => $rid))
      ->setConsiderLog(FALSE)
      ->setSeverity(WATCHDOG_ERROR)
      ->execute();
    drupal_set_message(t('File was not saved because of some errors.'), 'error');
    return NULL;
  }
}

/**
 * Build xml document and save in file.
 *
 * @param int $rid
 *   Related id.
 * @param string $d_locale
 *   Locale in drupal format (ru, en).
 * @param string $title
 *   Entity title.
 * @param string $entity_type
 *   Entity type.
 *
 * @return string|bool
 *   Return filename or FALSE.
 */
function smartling_build_xml($rid, $d_locale, $title = '', $entity_type = NULL) {
  $log = new SmartlingLog();
  $content_to_translate = array();
  switch ($entity_type) {
    case 'node':
      $content_to_translate = smartling_node_content_to_translate($rid);
      break;

    case 'taxonomy_term':
      $content_to_translate = smartling_taxonomy_term_content_to_translate($rid);
      break;

    case 'comment':
      $content_to_translate = smartling_comment_content_to_translate($rid);
      break;

    case 'user':
      $content_to_translate = smartling_user_content_to_translate($rid);
      break;

    default:
      $file_name = FALSE;
      break;
  }

  if (!empty($content_to_translate)) {
    $xml = new DOMDocument('1.0', 'UTF-8');

    $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string '));
    $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string '));

    $data = $xml->createElement('data');

    $data->appendChild(smartling_build_localize($xml, $rid, $content_to_translate));
    $xml->appendChild($data);

    $file_name = strtolower(trim(preg_replace('#\W+#', '_', $title), '_')) . '_' . $entity_type . '_' . $rid . '.xml';

    $xml = smartling_save_xml($xml, $rid, $d_locale, $file_name, FALSE, $entity_type);
    if (is_null($xml)) {
      $file_name = FALSE;
    }
  }
  else {
    smartling_entity_delete_all_by_conditions(array(
      'rid' => $rid,
      'entity_type' => $entity_type,
    ));
    $log->setMessage('Entity has no strings to translate for entity_type - @entity_type, id - @rid.')
      ->setVariables(array('@entity_type' => $entity_type, '@rid' => $rid))
      ->setSeverity(WATCHDOG_WARNING)
      ->execute();
    $file_name = FALSE;
  }

  return $file_name;
}

/**
 * Build localize object.
 *
 * @param object $dom
 *   Dom object.
 * @param string $title
 *   This param set value of attribute title.
 * @param array $strings
 *   This is array of strings to be create.
 * @param string $index
 *   This is index of localize or nolocalize tag.
 *   Such as localize2 or nolocalize3.
 *   Is empty string by default.
 * @param bool $no
 *   If TRUE tag -> nolocalize, if FALSE tag -> localize.
 *   Tag localize use by default.
 *
 * @return object
 *   Return localize object.
 */
function smartling_build_localize($dom, $title = '', array $strings = array(), $index = '', $no = FALSE) {
  $element = ($no !== FALSE) ? 'nolocalize' . $index : 'localize' . $index;

  $localize = $dom->createElement($element);
  $localize_attr = $dom->createAttribute('title');
  $localize_attr->value = $title;
  $localize->appendChild($localize_attr);

  foreach ($strings as $field_name => $field_value) {
    if (is_array($field_value)) {
      // Fields body-summary or image or text.
      if (!is_string($field_value[0]) && isset($field_value[0]['body'])) {
        // Field body-summary.
        $quantity = count($field_value);
        foreach ($field_value as $key => $value) {
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value['body']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-body-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity.
          $string_attr = $dom->createAttribute('quantity');
          $string_attr->value = $quantity;
          $string->appendChild($string_attr);
          $localize->appendChild($string);

          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value['summary']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-summary-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity.
          $string_attr = $dom->createAttribute('quantity');
          $string_attr->value = $quantity;
          $string->appendChild($string_attr);
          $localize->appendChild($string);
        }
      }
      elseif (!is_string($field_value[0]) && isset($field_value[0]['alt-img'])) {
        // Field image.
        $quantity = count($field_value);
        foreach ($field_value as $key => $value) {
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value['alt-img']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-alt-img-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity.
          $string_attr = $dom->createAttribute('quantity');
          $string_attr->value = $quantity;
          $string->appendChild($string_attr);
          // Set image fid.
          $string_attr = $dom->createAttribute('fid');
          $string_attr->value = $value['fid-img'];
          $string->appendChild($string_attr);
          $localize->appendChild($string);

          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value['title-img']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-title-img-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity.
          $string_attr = $dom->createAttribute('quantity');
          $string_attr->value = $quantity;
          $string->appendChild($string_attr);
          // Set image fid.
          $string_attr = $dom->createAttribute('fid');
          $string_attr->value = $value['fid-img'];
          $string->appendChild($string_attr);
          $localize->appendChild($string);
        }
      }
      elseif (is_string($field_value[0])) {
        // Field text.
        $quantity = count($field_value);
        foreach ($field_value as $key => $value) {
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($value);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_name . '-' . $key;
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          // Set quantity.
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
 * Download translate for entity.
 *
 * @param int $rid
 *   Related id.
 * @param string|int $d_locale
 *   Locale in drupal format (ru, en) or smartling transaction entity id.
 * @param object $entity_data
 *   Smartling transaction entity.
 */
function smartling_download_translate($rid, $d_locale = '', $entity_data = NULL) {
  if (smartling_is_configured()) {
    // Load entity.
    if (is_null($entity_data) && is_numeric($d_locale)) {
      // Use in views report (Download).
      $entity_data = smartling_entity_load_by_conditions(array(
        'rid' => $rid,
        'eid' => $d_locale,
      ));
      $d_locale = $entity_data->target_language;
    }

    if (($entity_data !== FALSE) && (smartling_supported_type($entity_data->entity_type, $entity_data->bundle))) {
      $s_locale = smartling_convert_locale_drupal_to_smartling($entity_data->target_language);
      $progress = smartling_get_progress_status($entity_data);

      if ($progress !== FALSE) {
        $xml = smartling_download_file($entity_data);

        $file_name = substr($entity_data->file_name, 0, strlen($entity_data->file_name) - 4);
        $translated_filename = $file_name . '_' . $entity_data->target_language . '.xml';

        // Save result.
        $save = smartling_save_xml($xml, $rid, $d_locale, $translated_filename, TRUE, $entity_data->entity_type);

        // If result is saved.
        if (is_object($save)) {
          smartling_update_translated_fields($rid, $d_locale, $entity_data->entity_type, $entity_data->bundle);
          $entity_data->progress = $progress;
          smartling_entity_data_save($entity_data);
          drupal_set_message(t('Downloaded for language translation @language', array('@language' => $s_locale)), 'status');
        }
      }
      else {
        drupal_set_message(t('Progress status Failed for language translation @language', array('@language' => $s_locale)), 'error');
      }
    }
    else {
      drupal_set_message(t('Progress status Failed for language translation @language', array('@language' => $s_locale)), 'error');
    }
  }
  else {
    drupal_set_message(t('Smartling is not configured'), 'error');
  }
}

/**
 * Download file from service.
 *
 * @param object $entity_data
 *   Smartling transaction entity.
 *
 * @return \DOMDocument
 *   Return xml dom from downloaded file.
 */
function smartling_download_file($entity_data) {
  $file_name_unic = $entity_data->file_name;
  $s_locale = smartling_convert_locale_drupal_to_smartling($entity_data->target_language);
  $smartling_settings = smartling_settings_get_handler();

  // Init api object.
  $api = new SmartlingAPI($smartling_settings->getApiUrl(), $smartling_settings->getKey(), $smartling_settings->getProjectId(), SMARTLING_PRODUCTION_MODE);

  $download_param = array(
    'retrievalType' => $smartling_settings->getRetrievalType(),
  );

  // Try to download file.
  $download_result = $api->downloadFile($file_name_unic, $s_locale, $download_param);
  // This is a download result.
  $xml = new DOMDocument();
  $xml->loadXML($download_result);

  return $xml;
}

/**
 * Get progress for transaction.
 *
 * @param object $entity_data
 *   Smartling transaction entity.
 *
 * @return int|bool
 *   Return number of progress or FALSE.
 */
function smartling_get_progress_status($entity_data) {
  if (is_string($entity_data->file_name) && (strlen($entity_data->file_name) > 0)) {
    $log = new SmartlingLog();
    $smartling_settings = smartling_settings_get_handler();
    $link_to_entity = smartling_get_link_to_entity($entity_data->rid, $entity_data->entity_type);

    $d_locale = $entity_data->target_language;
    $s_locale = smartling_convert_locale_drupal_to_smartling($d_locale);
    $file_name_unic = $entity_data->file_name;
    $file_uri = smartling_clean_filename(smartling_get_dir() . '/' . $entity_data->file_name, TRUE);

    // Init api object.
    $api = new SmartlingAPI($smartling_settings->getApiUrl(), $smartling_settings->getKey(), $smartling_settings->getProjectId(), SMARTLING_PRODUCTION_MODE);

    // Try to retrieve file status.
    $status_result = $api->getStatus($file_name_unic, $s_locale);
    $status_result = json_decode($status_result);

    // This is a get status.
    if ($api->getCodeStatus() == 'SUCCESS') {
      // Calc progress.
      if ($status_result->response->data->approvedStringCount !== 0) {
        $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);
      }
      else {
        $progress = 100;
      }

      $log->setMessage('Smartling progress status for entity id - @rid (@d_locale). approvedString = @as, completedString = @cs, progress = @progress')
        ->setVariables(array(
          '@rid' => $entity_data->rid,
          '@d_locale' => $d_locale,
          '@as' => $status_result->response->data->approvedStringCount,
          '@cs' => $status_result->response->data->completedStringCount,
          '@progress' => $progress,
        ))
        ->setLink(l(t('View original file'), $file_uri))
        ->execute();
      return $progress;
    }
    else {
      $log->setMessage('Smartling tried to check file progress: <br/>
      Project Id: @project_id <br/>
      Action: get progress <br/>
      URI: @file_uri <br/>
      Locale: @d_locale <br/>
      Error: response code -> @code and message -> @message')
        ->setVariables(array(
          '@project_id' => $smartling_settings->getProjectId(),
          '@file_uri' => $file_uri,
          '@d_locale' => $d_locale,
          '@code' => $status_result->response->code,
          '@message' => $status_result->response->messages[0],
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      drupal_set_message(t('Smartling tried to check file progress: response code - @code, message - @message', array(
        '@code' => $status_result->response->code,
        '@message' => $status_result->response->messages[0],
      )), 'error');
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

/**
 * Smartling language options list.
 *
 * @return array
 *   Return language options list or empty array.
 */
function smartling_language_options_list() {
  $language_options_list = array();
  $languages = smartling_language_list();

  foreach ($languages as $langcode => $language) {
    if ($language->enabled != '0') {
      $language_options_list[$langcode] = check_plain($language->name);
    }
  }
  return $language_options_list;
}

/**
 * Smartling language list.
 *
 * @return array
 *   Return language list or empty array.
 */
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
 * Change smartling transaction entity status.
 *
 * @param object $entity_data
 *   Smartling transaction entity.
 * @param string $event
 *   Event for change status.
 */
function smartling_set_translation_status($entity_data = NULL, $event = NULL) {
  if (!is_null($entity_data) && !is_null($event)) {
    $entity_data = smartling_entity_load_by_conditions(array(
      'rid' => $entity_data->rid,
      'entity_type' => $entity_data->entity_type,
      'target_language' => $entity_data->target_language,
    ));
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

      case SMARTLING_STATUS_EVENT_FAILED_UPLOAD:
        $entity_data->status = SMARTLING_STATUS_FAILED;
        smartling_entity_data_save($entity_data);
        break;

      default:
        break;
    }
  }
}

/**
 * Get link to entity.
 *
 * @param int $rid
 *   Related id.
 * @param string $entity_type
 *   Entity type.
 *
 * @return string
 *   Return link to entity.
 */
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

/**
 * Get original entity.
 *
 * @param int $rid
 *   Related id.
 * @param string $entity_type
 *   Entity type.
 *
 * @return object|bool
 *   Return entity object or FALSE.
 */
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

/**
 * Get field format.
 *
 * @param object $entity_wrapper
 *   Entity.
 * @param string $field_name
 *   Field name.
 * @param string $language_key_format
 *   Field language.
 * @param int $i
 *   Field delta.
 * @param object $original_entity
 *   Original entity. NULL by default.
 *
 * @return string|null
 *   Return field format like "full_html" or NULL.
 */
function smartling_get_field_format($entity_wrapper, $field_name, $language_key_format, $i = 0, $original_entity = NULL) {
  if (isset($entity_wrapper->{$field_name}[$language_key_format][$i]['format']) && isset($original_entity->{$field_name}[$language_key_format][$i]['format'])) {
    $format = $original_entity->{$field_name}[$language_key_format][$i]['format'];
  }
  else {
    $format = NULL;
  }

  return $format;
}

/**
 * Get fields by type.
 *
 * @param string $type
 *   Like entityreference field type or other.
 *
 * @return array
 *   Return array with fields by type.
 */
function smartling_get_fields_by_type($type) {
  return field_read_fields(array('type' => $type));
}

/**
 * Translation handler factory.
 *
 * @param string $entity_type
 *   Entity type.
 * @param object $entity
 *   Entity.
 *
 * @return object
 *   Return translation handler object.
 */
function smartling_entity_translation_get_handler($entity_type, $entity) {
  $entity_info = entity_get_info($entity_type);
  $class = 'SmartlingEntityTranslationDefaultHandler';
  // @todo remove fourth parameter once 3rd-party translation handlers have
  // been fixed and no longer require the deprecated entity_id parameter.
  $handler = new $class($entity_type, $entity_info, $entity, NULL);
  return $handler;
}

/**
 * Get settings handler.
 *
 * @return object
 *   Return handler object.
 */
function smartling_settings_get_handler() {
  $class = 'SmartlingSettingsHandler';
  $handler = new $class();
  return $handler;
}

/**
 * Save entity translation.
 *
 * @param object $handler
 *   Handler.
 * @param object $entity
 *   Entity.
 */
function smartling_entity_translation_save($handler, $entity) {
  $handler->saveTranslations();
  // Update the wrapped entity.
  $handler->setEntity($entity);
  $handler->smartlingEntityTranslationFieldAttach();
}

/**
 * Check filename length.
 *
 * @param string $title
 *   Entity title.
 * @param string $entity_type
 *   Entity type.
 * @param int $rid
 *   Related id.
 *
 * @return bool
 *   Return TRUE if normal filename.
 */
function smartling_check_filename_length($title, $entity_type, $rid) {
  $log = new SmartlingLog();
  $file_name = strtolower(trim(preg_replace('#\W+#', '_', $title), '_')) . '_' . $entity_type . '_' . $rid . '.xml';

  if (strlen($file_name) > 255) {
    $count = (strlen($file_name) - 240);
    $link_to_entity = smartling_get_link_to_entity($rid, $entity_type);
    $log->setMessage('Generated file name is too long. Remove approximately @count characters. Filename: @file_name, related entity - @rid, entity type - @entity_type')
      ->setVariables(array(
        '@count' => $count,
        '@file_name' => $file_name,
        '@rid' => $rid,
        '@entity_type' => $entity_type,
      ))
      ->setConsiderLog(FALSE)
      ->setSeverity(WATCHDOG_ERROR)
      ->setLink($link_to_entity)
      ->execute();
    return FALSE;
  }
  return TRUE;
}

/**
 * Check field by name.
 *
 * Return TRUE if this is fake field for entites translate method.
 *
 * @param string $field_name
 *   Field name for check.
 *
 * @return bool
 *   Return fake parameter.
 */
function smartling_check_fake_field_for_entities_translate_method($field_name) {
  $fake = FALSE;
  $fake_field_array = array(
    'title_property_field',
    'name_property_field',
    'description_property_field',
  );

  if (in_array($field_name, $fake_field_array)) {
    $fake = TRUE;
  }
  else {
    $fake = FALSE;
  }

  return $fake;
}

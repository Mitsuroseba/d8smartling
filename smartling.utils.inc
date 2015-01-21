<?php

/**
 * @file
 * Utils functions for Smartling module.
 */

use Drupal\smartling\EntityProcessorFactory;
use Drupal\smartling\Settings\SmartlingSettingsHandler;
use Drupal\smartling\Processors\NodeProcessor;
use Drupal\smartling\Processors\TaxonomyTermProcessor;
use Drupal\smartling\Processors\GenericEntityProcessor;

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
      if (($smartling_supported_explicitly == SMARTLING_NODES_METHOD_KEY || $smartling_supported_explicitly == SMARTLING_FIELDS_METHOD_KEY)) {
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

    //@todo: implement normal check for this type of entity
    case 'fieldable_panels_pane':
      $smartling_supported_explicitly = TRUE;//smartling_entity_has_translatable_field($entity_type, $bundle);
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
 *
 * @todo Should be part of Settings class
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
 *
 * @todo Should be part of Settings class
 */
function smartling_fields_method($type) {
  $fields_method = variable_get('language_content_type_' . $type, NULL) != SMARTLING_NODES_METHOD_KEY;
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
 * @param string $bundle
 *   Bundle.
 * @param string $entity_type
 *   Entity type.
 *
 * @return bool
 *   Return TRUE if translate fields is set.
 */
function smartling_translate_fields_is_set($bundle, $entity_type) {
  $sm_settings = smartling_settings_get_handler();
  $val = $sm_settings->getFieldsSettingsByBundle($entity_type, $bundle);

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
    'field_collection_embed',
  );
  return $included_fields;
}

/**
 * Checks translatable field by field name.
 *
 * @param string $field_name
 *   Field name.
 *
 * @param string $entity_type
 *   Entity type machine name.
 * @return bool
 *   Return TRUE if field is translatable.
 */
function smartling_field_is_translatable_by_field_name($field_name, $entity_type) {
  $field = field_info_field($field_name);
  return field_is_translatable($entity_type, $field);
}

/**
 * Update filename in smartling transaction.
 *
 * @param string $file_name
 *   File name.
 * @param stdClass $smartling_entity
 *   Smartling entity.
 * @param bool $translated_file
 *   TRUE if save translated file. FALSE by default.
 */
function smartling_entity_data_filename_update($file_name, $smartling_entity, $translated_file = FALSE) {
    if ($translated_file) {
      $smartling_entity->translated_file_name = $file_name;
    }
    else {
      $smartling_entity->file_name = $file_name;
    }

    smartling_entity_data_save($smartling_entity);
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
 *   Locale string in some format: 'en' or 'en-US'.
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
 *   Return path for smartling dir without end '/' or file path with dir.
 */
function smartling_get_dir($file_name = '') {
  $smartling_dir = (variable_get('file_private_path', FALSE)) ? ('private://smartling') : ('public://smartling');
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

  $pattern = '/[^a-zA-Z0-9_\-\:]/i';
  $info = pathinfo(trim($filename));
  $filename = preg_replace($pattern, '_', $info['filename']);
  if (isset($info['extension']) && !empty($info['extension'])) {
    $filename .= '.' . preg_replace($pattern, '_', $info['extension']);
  }

  if ($allow_dirs && isset($info['dirname']) && !empty($info['dirname'])) {
    $filename = preg_replace('/[^a-zA-Z0-9_\/\-\:]/i', '_', $info['dirname']) . '/' . $filename;
  }

  return (string) $filename;
}

/**
 * @todo Add new 'Debug' option to smartling settings and save files to disk only if debugging is enabled
 * Otherwise skip saving. Other function must not require a phisical file on a disk
 *
 * Save xml document.
 *
 * @param object $xml_doc
 *   Xml document.
 * @param stdClass $smartling_entity
 *   Locale in drupal format (ru, en).
 * @param string $file_name
 *   File name.
 * @param bool $translated_file
 *   TRUE if save translated file. FALSE by default.
 * @param string $entity_type
 *   Entity type.
 *
 * @return bool
 *   Was file creation successful or not.
 */
function smartling_save_xml($xml_doc, $smartling_entity, $file_name = NULL, $translated_file = FALSE) {
  $log = smartling_log_get_handler();

  if (is_null($file_name)) {
    $file_name = 'smartling_translate_' . $smartling_entity->entity_type . '_' . $smartling_entity->rid . '.xml';
  }

  $file_name = file_munge_filename(preg_replace('@^.*/@', '', $file_name), '', TRUE);
  $directory = smartling_get_dir();
  $path = $directory . '/' . smartling_clean_filename($file_name);
  $file_type = 'original';

  if (file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    $xml_doc->save(drupal_realpath($path));

    if ($translated_file == TRUE) {
      $file_type = 'translated';
    }


    $log->setMessage('Smartling saves @file_type xml file for entity_type - @entity_type, id - @rid. Locale: @locale')
      ->setVariables(array(
        '@file_type' => $file_type,
        '@entity_type' => $smartling_entity->entity_type,
        '@rid' => $smartling_entity->rid,
        '@locale' => $smartling_entity->target_language,
      ))
      ->setLink(l(t('View file'), file_create_url($path)))
      ->execute();


    if (!is_null($smartling_entity->rid)) {
      smartling_entity_data_filename_update($file_name, $smartling_entity, $translated_file);
    }
    return TRUE;
  }
  else {
    $log->setMessage('Smartling file was not saved because of some errors. Filename: @file_name, related entity - @rid, directory: @dir.')
      ->setVariables(array('@file_name' => $file_name, '@rid' => $smartling_entity->rid, '@dir' => $directory))
      ->setConsiderLog(FALSE)
      ->setSeverity(WATCHDOG_ERROR)
      ->execute();
    drupal_set_message(t('File was not saved because of some errors. Please see the logs for more details.'), 'error');
    return FALSE;
  }
}

function smartling_get_entity_processor($smartling_entity) {
  return drupal_container()->get('smartling.entity_processor_factory')->getProcessor($smartling_entity);
}

/**
 * Build xml document and save in file.
 *
 * @param string $file_name
 *   XML file name to be created.
 * @param array $content_to_translate
 *   Array of string to be translated.
 *
 * @return string|bool
 *   Return filename or FALSE.
 */
function smartling_build_xml($processor, $rid) {
  $xml = new DOMDocument('1.0', 'UTF-8');

  $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string, data/localize/field_collection/string, data/localize/field_collection/field_collection/string, data/localize/field_collection/field_collection/field_collection/string, data/localize/field_collection/field_collection/field_collection/field_collection/string '));
  // @todo remove hardcoded mappping of nested field colelctions.
  $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string, html : data/localize/field_collection/string, html : data/localize/field_collection/field_collection/string, html : data/localize/field_collection/field_collection/field_collection/string '));
  $xml->appendChild($xml->createComment(' smartling.placeholder_format_custom = (@|%|!)[\w-]+ '));

  $data = $xml->createElement('data');

  $localize = $processor->exportContentToTranslation($xml, $rid);

  //$data->appendChild(smartling_build_localize($xml, $rid, $content_to_translate));
  $data->appendChild($localize);

// @todo Verify how many child has $data. If zero, then write to log and stop upload
// This logic was lost in OOP branch
//  {
//    smartling_entity_delete_all_by_conditions(array(
//      'rid' => $rid,
//      'entity_type' => $entity_type,
//    ));
//    $log->setMessage('Entity has no strings to translate for entity_type - @entity_type, id - @rid.')
//      ->setVariables(array('@entity_type' => $entity_type, '@rid' => $rid))
//      ->setSeverity(WATCHDOG_WARNING)
//      ->execute();
//    $file_name = FALSE;
//  }


  $xml->appendChild($data);


  return $xml;
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
 *
 * @todo it's a duplicate of setProgressStatus() method of GenericEntity. Need to resolve it.
 */
function smartling_set_translation_status($entity_data = NULL, $event = NULL) {
  if (!is_null($entity_data) && !is_null($event)) {
    // @todo: why do we need reload this entity everywhere?
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
 * @deprecated
 * @todo Looks like we have similar function in GenericEntityProcessor.linkToContent()
 *
 * @return string
 *   Return link to entity.
 */
function smartling_get_link_to_entity($rid, $entity_type) {
  $function_uri = $entity_type . '_uri';
  $function_load = $entity_type . '_load';
  if (function_exists($function_load)) {
    $entity = $function_load($rid);
  }
  if (function_exists($function_uri)) {
    $uri = $function_uri($entity);
    $path = $uri['path'];
  }

  switch ($entity_type) {
    case 'node':
    case 'taxonomy_term':
    case 'comment':
    case 'user':
      $link = l(t('Related entity'), $path);
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
 * @deprecated
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
 * @param object $original_entity
 *   Original entity.
 * @param string $field_name
 *   Field name.
 * @param string $language_key_format
 *   Field language.
 * @param int $i
 *   Field delta.
 *
 * @return string|null
 *   Return field format like "full_html" or NULL.
 */
function smartling_get_field_format($original_entity, $field_name, $language_key_format, $i = 0) {
  if (isset($original_entity->{$field_name}[$language_key_format][$i]['format'])) {
    return $original_entity->{$field_name}[$language_key_format][$i]['format'];
  }
  else {
    return NULL;
  }
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
  return drupal_container()->get('smartling.settings');
}

/**
 * Get log handler.
 *
 * @return Drupal\smartling\Log\SmartlingLog
 *   Return handler object.
 */
function smartling_log_get_handler() {
  return drupal_container()->get('smartling.log');
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
  $log = smartling_log_get_handler();
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
 * @deprecated
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

/**
 * Check field by field name.
 *
 * Return TRUE if this field is replacement field by title module.
 *
 * @param string $field_name
 *   Field name.
 *
 * @deprecated
 *
 * @return boolean
 *   Return check result.
 */
function smartling_is_title_module_field($field_name) {
  $title_module = FALSE;
  $field_array = array(
    'title_field',
    'name_field',
    'description_field',
  );

  if (in_array($field_name, $field_array)) {
    $title_module = TRUE;
  }
  else {
    $title_module = FALSE;
  }

  return $title_module;
}

/**
 * Set duplicated values for fields method.
 *
 * We need to duplicate the original value in the first cell of
 * a target language for the fields that are
 * not in the settings smartling and that are translated so far.
 *
 * @param string $entity_type
 *   Entity type.
 * @param string $bundle
 *   Bundle.
 * @param object $original_entity
 *   Original entity.
 * @param string $d_locale
 *   Locale.
 * @param array $fields
 *   Fields in smartling settings for entity.
 *
 * @deprecated
 *
 * @return object
 *   Return entity object.
 */
function smartling_duplicate_values_for_fields_method($entity_type, $bundle, $original_entity, $d_locale, array $fields) {
  $field_info_instances = field_info_instances($entity_type, $bundle);
  $need_save = FALSE;
  foreach ($field_info_instances as $field) {
    if (!in_array($field['field_name'], $fields) && smartling_field_is_translatable_by_field_name($field['field_name'], $entity_type) && isset($original_entity->{$field['field_name']})) {
      $need_save = TRUE;
      $original_lang = entity_language($entity_type, $original_entity);
      $original_entity->{$field['field_name']}[$d_locale] = $original_entity->{$field['field_name']}[$original_lang];
    }
  }
  if ($need_save) {
    $function_name = $entity_type . '_save';
    if (function_exists($function_name)) {
      $function_name($original_entity);
    }
  }

  return $original_entity;
}

/**
 * Copy devel_create_greeking().
 *
 * @param int $word_count
 *   Words count.
 * @param bool $title
 *   TRUE if is title. FALSE by default.
 *
 * @return string
 *   Return random string.
 */
function smartling_devel_create_greeking($word_count, $title = FALSE) {
  $dictionary = array("abbas", "abdo", "abico", "abigo", "abluo", "accumsan",
    "acsi", "ad", "adipiscing", "aliquam", "aliquip", "amet", "antehabeo",
    "appellatio", "aptent", "at", "augue", "autem", "bene", "blandit",
    "brevitas", "caecus", "camur", "capto", "causa", "cogo", "comis",
    "commodo", "commoveo", "consectetuer", "consequat", "conventio", "cui",
    "damnum", "decet", "defui", "diam", "dignissim", "distineo", "dolor",
    "dolore", "dolus", "duis", "ea", "eligo", "elit", "enim", "erat",
    "eros", "esca", "esse", "et", "eu", "euismod", "eum", "ex", "exerci",
    "exputo", "facilisi", "facilisis", "fere", "feugiat", "gemino",
    "genitus", "gilvus", "gravis", "haero", "hendrerit", "hos", "huic",
    "humo", "iaceo", "ibidem", "ideo", "ille", "illum", "immitto",
    "importunus", "imputo", "in", "incassum", "inhibeo", "interdico",
    "iriure", "iusto", "iustum", "jugis", "jumentum", "jus", "laoreet",
    "lenis", "letalis", "lobortis", "loquor", "lucidus", "luctus", "ludus",
    "luptatum", "macto", "magna", "mauris", "melior", "metuo", "meus",
    "minim", "modo", "molior", "mos", "natu", "neo", "neque", "nibh",
    "nimis", "nisl", "nobis", "nostrud", "nulla", "nunc", "nutus", "obruo",
    "occuro", "odio", "olim", "oppeto", "os", "pagus", "pala", "paratus",
    "patria", "paulatim", "pecus", "persto", "pertineo", "plaga", "pneum",
    "populus", "praemitto", "praesent", "premo", "probo", "proprius",
    "quadrum", "quae", "qui", "quia", "quibus", "quidem", "quidne", "quis",
    "ratis", "refero", "refoveo", "roto", "rusticus", "saepius",
    "sagaciter", "saluto", "scisco", "secundum", "sed", "si", "similis",
    "singularis", "sino", "sit", "sudo", "suscipere", "suscipit", "tamen",
    "tation", "te", "tego", "tincidunt", "torqueo", "tum", "turpis",
    "typicus", "ulciscor", "ullamcorper", "usitas", "ut", "utinam",
    "utrum", "uxor", "valde", "valetudo", "validus", "vel", "velit",
    "veniam", "venio", "vereor", "vero", "verto", "vicis", "vindico",
    "virtus", "voco", "volutpat", "vulpes", "vulputate", "wisi", "ymo",
    "zelus");
  $dictionary_flipped = array_flip($dictionary);

  $greeking = '';

  if (!$title) {
    $words_remaining = $word_count;
    while ($words_remaining > 0) {
      $sentence_length = mt_rand(3, 10);
      $words = array_rand($dictionary_flipped, $sentence_length);
      $sentence = implode(' ', $words);
      $greeking .= ucfirst($sentence) . '. ';
      $words_remaining -= $sentence_length;
    }
  }
  else {
    // Use slightly different method for titles.
    $words = array_rand($dictionary_flipped, $word_count);
    $words = is_array($words) ? implode(' ', $words) : $words;
    $greeking = ucwords($words);
  }

  // Work around possible php garbage collection bug. Without an unset(), this
  // function gets very expensive over many calls (php 5.2.11).
  unset($dictionary, $dictionary_flipped);
  return trim($greeking);
}

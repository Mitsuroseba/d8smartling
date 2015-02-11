<?php

/**
 * @file
 * Utils functions for Smartling module.
 */

/**
 * Returns whether the given entity type has support for Smartling translations.
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
      $transl_method = variable_get('language_content_type_' . $bundle, NULL);
      $smartling_supported_explicitly = in_array($transl_method, array(SMARTLING_NODES_METHOD_KEY, SMARTLING_FIELDS_METHOD_KEY));
      break;

    case 'taxonomy_term':
      $vocabulary = taxonomy_vocabulary_machine_name_load($bundle);
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
      $smartling_supported_explicitly = in_array($vocabulary_mode, array(I18N_MODE_TRANSLATE, I18N_MODE_LOCALIZE));
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
  foreach (field_info_instances($entity_type, $bundle) as $instance) {
    if (smartling_field_is_translatable($instance['field_name'], $entity_type)) {
      return TRUE;
    }
  }
  return FALSE;
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
 * @param object $node
 *   Node object.
 *
 * @return string
 *   Return unical node status for nodes method translate.
 */
function smartling_nodes_method_node_status($node) {
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
    if (empty($val)) {
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
 *   Return TRUE if fields for translation are set.
 */
function smartling_translate_fields_configured($bundle, $entity_type) {
  $val = smartling_settings_get_handler()->getFieldsSettingsByBundle($entity_type, $bundle);

  if (empty($val)) {
    drupal_set_message(t('Please follow the page <a href="@link">"Smartling settings"</a> to setup Smartling configuration.', array('@link' => url('admin/config/regional/smartling'))), 'warning');
  }

  return !empty($val);
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
function smartling_field_is_translatable($field_name, $entity_type) {
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
 * @return string
 *   Return full language name or empty string.
 */
function smartling_get_full_language_name($d_locale) {
  $languages = language_list();
  return (isset($languages[$d_locale]))? $languages[$d_locale]->name : '';
}

/**
 * This functions convert locale format. Example: 'en' => 'en-US'.
 *
 * @param string $locale
 *   Locale string in Drupal format: 'en'.
 *
 * @return string
 *   Return locale or empty string.
 */
function smartling_convert_locale_drupal_to_smartling($locale) {
  $locales = smartling_settings_get_handler()->getLocalesConvertArray();
  return (isset($locales[$locale])) ? $locales[$locale] : '';
}

/**
 * This functions convert locale format. Example: 'en-US' => 'en'.
 *
 * @param string $locale
 *   Locale string in Smartling format: 'en-US'.
 *
 * @return string
 *   Return locale or empty string.
 */
function smartling_convert_locale_smartling_to_drupal($locale) {
  $locales = smartling_settings_get_handler()->getLocalesConvertArray();
  return (string) array_search($locale, $locales);
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
 * @param object $processor
 *   Drupal entity processor
 * @param int $rid
 *
 * @return DOMDocument
 *   Returns XML object.
 */
function smartling_build_xml($processor, $rid) {
  $xml = new DOMDocument('1.0', 'UTF-8');

  $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string, data/localize/field_collection/string, data/localize/field_collection/field_collection/string, data/localize/field_collection/field_collection/field_collection/string, data/localize/field_collection/field_collection/field_collection/field_collection/string '));
  // @todo remove hardcoded mappping of nested field colelctions.
  $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string, html : data/localize/field_collection/string, html : data/localize/field_collection/field_collection/string, html : data/localize/field_collection/field_collection/field_collection/string '));
  $xml->appendChild($xml->createComment(' smartling.placeholder_format_custom = (@|%|!)[\w-]+ '));

  $data = $xml->createElement('data');

  $localize = $processor->exportContentToTranslation($xml, $rid);

  $data->appendChild($localize);
  $xml->appendChild($data);

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

  return $xml;
}

/**
 * Smartling language options list.
 * It's prepared for the list/checkboxes-item on the form.
 *
 * @return array
 *   Return language options list or empty array.
 */
function smartling_language_options_list() {
  $languages = smartling_language_list();

  foreach ($languages as $lang => $language) {
    $languages[$lang] = check_plain($language->name);
  }
  return $languages;
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
 * @param string $entity_type
 *   Entity type.
 * @param object $entity
 *   Smartling data object
 * @param string $link_title
 *
 * @todo Looks like we have similar function in GenericEntityProcessor.linkToContent()
 *
 * @return string
 *   Return link to entity.
 */
function smartling_get_link_to_entity($entity_type, $entity, $link_title = '') {
  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  $link_title = (!empty($link_title)) ? $link_title : $wrapper->label();
  $link_title = (!empty($link_title)) ? $link_title : t('Related entity');

  $uri     = entity_uri($entity_type, $entity);
  $path    = $uri['path'];

  return l($link_title, $path);
}

/**
 * Get original entity.
 *
 * @param objext $term
 *
 * @deprecated
 *
 * @return object
 *   Return entity object or NULL.
 */
function smartling_get_original_taxonomy_term($term) {
  $entity = NULL;
  if ($term->language == language_default()->language) {
    $entity = $term;
  }
  elseif (isset($term->i18n_tsid) && is_numeric($term->i18n_tsid)) {
    $entity = i18n_taxonomy_term_get_translation($term, language_default()->language);
  }

  return $entity;
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

/*
 * Creates an HTML-ready translation status of an entity.
 *
 * @param string $entity_name
 * @param int $status
 * @param string $language_name
 * @param int $progress
 *
 * @return string
 */
function smartling_entity_status_message($entity_name, $status, $language_name, $progress) {
  $res = '';
  $entity_name   = check_plain($entity_name);
  $language_name = check_plain($language_name);
  $progress      = intval($progress);
  switch ($status) {
    case SMARTLING_STATUS_IN_QUEUE:
      $res = '<span title="' . $entity_name . ' ' . t('is waiting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-queue">' . $progress . '%</span></span>';
      break;

    case SMARTLING_STATUS_IN_TRANSLATE:
      $res = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-translate">' . $progress . '%</span></span>';
      break;

    case SMARTLING_STATUS_TRANSLATED:
      $res = '<span title="'  . $entity_name . ' ' . t('is translated') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status translated">' . $progress . '%</span></span>';
      break;

    case SMARTLING_STATUS_CHANGE:
      $res = '<span title="' . $entity_name . ' ' . t('was changed after submitting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-changed">' . $progress . '%</span></span>';
      break;

    case SMARTLING_STATUS_FAILED:
      $res = '<span title="' . t('Request to service failed') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-failed">' . $progress . '%</span></span>';
      break;
  }

  return $res;
}


/**
 * Create Smartling entity.
 *
 * @param stdClass $drupal_entity
 *   Drupal content entity.
 * @param string $entity_type
 *   Drupal entity type machine name.
 * @param string $origin_language
 *   Language key of source drupal content entity.
 * @param string $target_language
 *   Target language key for smartling entity.
 *
 * @return SmartlingEntityData
 *
 * @todo try to avoid original language in arguments.
 */
function smartling_create_from_entity($drupal_entity, $entity_type, $origin_language, $target_language) {
  global $user;
  $entity_data = smartling_entity_data_create();
  $wrapper = entity_metadata_wrapper($entity_type, $drupal_entity);
  $entity_data->rid = $wrapper->getIdentifier();
  $entity_data->entity_type = $entity_type;
  $entity_data->title  = $wrapper->label();
  $entity_data->bundle = $wrapper->getBundle();
  $entity_data->original_language = $origin_language;
  $entity_data->target_language = $target_language;
  $entity_data->progress = 0;

  $entity_data->translated_file_name = FALSE;
  $entity_data->submitter = $user->uid;
  $entity_data->submission_date = time();
  smartling_entity_data_save($entity_data);

  return $entity_data;
}
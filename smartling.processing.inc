<?php

/**
 * @file
 * Processing functions for Smartling module.
 */

/**
 *
 * @param type $field_name
 * @return string|boolean
 */
function smartling_get_field_content_processing_mode($field_name) {
  $info = field_info_field($field_name);
  $swap_array = array('title_field', 'name_field', 'description_field', 'subject_field');
  if (in_array($field_name, $swap_array)) {
    return 'text-field';
  }
  else {
    switch ($info['type']) {
      case 'text':
        return 'text-field';
        break;
      case 'text_with_summary':
        return 'body-with-summary-field';
        break;
      case 'text_long':
        return 'text-field';
        break;
      case 'image':
        return 'image-field';
        break;

      default:
        break;
    }
  }
  return FALSE;
}

/**
 *
 * @param type $nid
 * Node id
 * @return array
 * Structured array of content that will be sent for translation
 */
function smartling_node_content_to_translate($nid) {
  $node_current_translatable_content = array();
  if (is_numeric($nid)) {
    $node = node_load($nid);
  }
  else {
    $node = $nid;
  }

  foreach (smartling_settings_get_handler()->nodeGetFieldsSettingsByBundle($node->type) as $field_name) {
    $language = (smartling_field_is_translatable_by_field_name($field_name)) ? entity_language('node', $node) : LANGUAGE_NONE;
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'node-title':
        if (!empty($node->title)) {
          $node_current_translatable_content[$field_name][0] = $node->title;
        }
        break;

      case 'text-field':
        if (!empty($node->{$field_name}[$language])) {
          foreach ($node->{$field_name}[$language] as $key => $item) {
            $node_current_translatable_content[$field_name][$key] = $item['value'];
          }
        }
        break;

      case 'body-with-summary-field':
        if (!empty($node->{$field_name}[$language])) {
          foreach ($node->{$field_name}[$language] as $key => $item) {
            $node_current_translatable_content[$field_name][$key]['body'] = $item['value'];
            $node_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
          }
        }
        break;

      case 'image-field':
        if (is_array($node->{$field_name}) && !empty($node->{$field_name}[$language])) {
          foreach ($node->{$field_name}[$language] as $key => $item) {
            $node_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $node_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $node_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog("Smartling didn't process content of field - @field_name", FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
        break;
    }
  }
  return $node_current_translatable_content;
}

/**
 *
 * @param type $tid
 * Term id
 * @return array
 * Structured array of content that will be sent for translation
 */
function smartling_term_content_to_translate($tid) {
  $term_current_translatable_content = array();
  if (is_numeric($tid)) {
    $term = taxonomy_term_load($tid);
  }
  else {
    $term = $tid;
  }

  foreach (smartling_settings_get_handler()->taxonomy_termGetFieldsSettingsByBundle($term->vocabulary_machine_name) as $field_name) {
    $language = (smartling_field_is_translatable_by_field_name($field_name)) ? entity_language('taxonomy_term', $term) : LANGUAGE_NONE;
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'taxonomy-term-title':
        if (!empty($term->name)) {
          $term_current_translatable_content[$field_name][0] = $term->name;
        }
        break;

      case 'taxonomy-term-description':
        if (!empty($term->description)) {
          $term_current_translatable_content[$field_name][0] = $term->description;
        }
        break;

      case 'text-field':
        if (!empty($term->{$field_name}[$language])) {
          foreach ($term->{$field_name}[$language] as $key => $item) {
            $term_current_translatable_content[$field_name][$key] = $item['value'];
          }
        }
        break;

      case 'body-with-summary-field':
        if (!empty($term->{$field_name}[$language])) {
          foreach ($term->{$field_name}[$language] as $key => $item) {
            $term_current_translatable_content[$field_name][$key]['body'] = $item['value'];
            $term_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
          }
        }
        break;

      case 'image-field':
        if (is_array($term->{$field_name}) && !empty($term->{$field_name}[$language])) {
          foreach ($term->{$field_name}[$language] as $key => $item) {
            $term_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $term_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $term_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog("Smartling didn't process content of field - @field_name", FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
        break;
    }
  }

  return $term_current_translatable_content;
}

/**
 *
 * @param type $cid
 * Comment id
 * @return array
 * Structured array of content that will be sent for translation
 */
function smartling_comment_content_to_translate($cid) {
  $comment_current_translatable_content = array();
  if (is_numeric($cid)) {
    $comment = comment_load($cid);
  }
  else {
    $comment = $cid;
  }

  foreach (smartling_settings_get_handler()->commentGetFieldsSettingsByBundle($comment->node_type) as $field_name) {
    $language = (smartling_field_is_translatable_by_field_name($field_name)) ? entity_language('comment', $comment) : LANGUAGE_NONE;
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'text-field':
        if (!empty($comment->{$field_name}[$language])) {
          foreach ($comment->{$field_name}[$language] as $key => $item) {
            $comment_current_translatable_content[$field_name][$key] = $item['value'];
          }
        }
        break;

      case 'body-with-summary-field':
        if (!empty($comment->{$field_name}[$language])) {
          foreach ($comment->{$field_name}[$language] as $key => $item) {
            $comment_current_translatable_content[$field_name][$key]['body'] = $item['value'];
            $comment_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
          }
        }
        break;

      case 'image-field':
        if (is_array($comment->{$field_name}) && !empty($comment->{$field_name}[$language])) {
          foreach ($comment->{$field_name}[$language] as $key => $item) {
            $comment_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $comment_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $comment_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog("Smartling didn't process content of field - @field_name", FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
        break;
    }
  }

  return $comment_current_translatable_content;
}

/**
 *
 * @param type $uid
 * User id
 * @return array
 * Structured array of content that will be sent for translation
 */
function smartling_user_content_to_translate($uid) {
  $user_current_translatable_content = array();
  if (is_numeric($uid)) {
    $user = user_load($uid);
  }
  else {
    $user = $uid;
  }

  foreach (smartling_settings_get_handler()->userGetFieldsSettingsByBundle('user') as $field_name) {
    $language = (smartling_field_is_translatable_by_field_name($field_name)) ? entity_language('user', $user) : LANGUAGE_NONE;
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'text-field':
        if (!empty($user->{$field_name}[$language])) {
          foreach ($user->{$field_name}[$language] as $key => $item) {
            $user_current_translatable_content[$field_name][$key] = $item['value'];
          }
        }
        break;

      case 'body-with-summary-field':
        if (!empty($user->{$field_name}[$language])) {
          foreach ($user->{$field_name}[$language] as $key => $item) {
            $user_current_translatable_content[$field_name][$key]['body'] = $item['value'];
            $user_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
          }
        }
        break;

      case 'image-field':
        if (is_array($user->{$field_name}) && !empty($user->{$field_name}[$language])) {
          foreach ($user->{$field_name}[$language] as $key => $item) {
            $user_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $user_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $user_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog("Smartling didn't process content of field - @field_name", FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
        break;
    }
  }

  return $user_current_translatable_content;
}

/**
 * This function return processing mode for field
 *
 * @param type $field_name
 * @param type $xpath
 * @return string
 */
function smartling_get_update_field_processing_mode($field_name, $xpath) {
  // check field variant
  $text_field = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
  $body = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);
  $summary = $xpath->query('//string[@id="' . $field_name . '-summary-0' . '"][1]')->item(0);
  $image_alt = $xpath->query('//string[@id="' . $field_name . '-alt-img-0' . '"][1]')->item(0);
  $image_title = $xpath->query('//string[@id="' . $field_name . '-title-img-0' . '"][1]')->item(0);

  if (is_object($text_field)) {
    // If this field is a text field
    if ($text_field instanceof DomElement) {
      return 'text-field';
    }
  }
  elseif (is_object($body) || is_object($summary)) {
    // If this field is a body/summary
    if (($body instanceof DomElement) || ($summary instanceof DomElement)) {
      return 'body-with-summary-field';
    }
  }
  elseif (is_object($image_alt) || is_object($image_title)) {
    // If this field is a image
    if (($image_alt instanceof DomElement) || ($image_title instanceof DomElement)) {
      return 'image-field';
    }
  }
  return FALSE;
}

/**
 * Processing for text field
 */
function smartling_text_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format) {
  $quantity_value = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
  $quantity = $quantity_value->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $field = $xpath->query('//string[@id="' . $field_name . '-' . $i . '"][1]')->item(0);
    $string = (string) $field->nodeValue;
    $format = smartling_get_field_format($entity_wrapper, $field_name, $language_key_format, $i);

    $entity_wrapper->{$field_name}[$language_key][$i]['value'] = $string;
    $entity_wrapper->{$field_name}[$language_key][$i]['format'] = $format;
  }
  return $entity_wrapper;
}

/**
 * Processing for body/summary field
 */
function smartling_body_with_summary_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format) {
  $quantity_value = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);
  $quantity = $quantity_value->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $body_obj = $xpath->query('//string[@id="' . $field_name . '-body-' . $i . '"][1]')->item(0);
    $summary_obj = $xpath->query('//string[@id="' . $field_name . '-summary-' . $i . '"][1]')->item(0);
    $body_string = (string) $body_obj->nodeValue;
    $summary_string = (string) $summary_obj->nodeValue;
    $format = smartling_get_field_format($entity_wrapper, $field_name, $language_key_format, $i);

    $entity_wrapper->{$field_name}[$language_key][$i]['value'] = $body_string;
    $entity_wrapper->{$field_name}[$language_key][$i]['format'] = $format;
    $entity_wrapper->{$field_name}[$language_key][$i]['summary'] = $summary_string;
  }
  return $entity_wrapper;
}

/**
 * Processing for image field
 */
function smartling_image_field_processing($entity_wrapper, $field_name, $xpath, $language_key) {
  $image_multiple = $xpath->query('//string[@id="' . $field_name . '-alt-img-0' . '"][1]')->item(0);
  $quantity = $image_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $alt_obj = $xpath->query('//string[@id="' . $field_name . '-alt-img-' . $i . '"][1]')->item(0);
    $title_obj = $xpath->query('//string[@id="' . $field_name . '-title-img-' . $i . '"][1]')->item(0);
    $fid = $alt_obj->getAttribute('fid');
    $file_img = file_load($fid);

    if ($file_img) {
      $alt_string = (string) $alt_obj->nodeValue;
      $title_string = (string) $title_obj->nodeValue;
      $entity_wrapper->{$field_name}[$language_key][$i] = (array) $file_img + array(
          'alt' => $alt_string,
          'title' => $title_string
        );
    }
  }
  return $entity_wrapper;
}

/**
 * Parsing the translated file and record the results in the field.
 * @param type $rid
 * @param type $d_locale
 * @param type $entity_type
 * @param type $bundle
 * @return boolean
 */
function smartling_update_translated_fields($rid, $d_locale, $entity_type = NULL, $bundle = NULL) {

  smartling_watchdog('Smartling starts update fields of entity @entity_type, id - @rid. Locale: @locale', TRUE, array(
    '@entity_type' => $entity_type,
    '@rid' => $rid,
    '@locale' => $d_locale
  ), WATCHDOG_INFO);

  if (smartling_supported_type($entity_type, $bundle)) {
    $smartling_settings = smartling_settings_get_handler();
    $entity_data = smartling_entity_load_by_conditions(array(
      'rid' => $rid,
      'entity_type' => $entity_type,
      'target_language' => $d_locale
    ));
    $file_path = smartling_clean_filename(smartling_get_dir($entity_data->translated_file_name), TRUE);

    // Get field names for entity
    switch ($entity_type) {
      case 'node':
        $fields = $smartling_settings->nodeGetFieldsSettingsByBundle($entity_data->bundle);
        break;

      case 'taxonomy_term':
        $fields = $smartling_settings->taxonomy_termGetFieldsSettingsByBundle($entity_data->bundle);
        break;

      case 'comment':
        $fields = $smartling_settings->commentGetFieldsSettingsByBundle($entity_data->bundle);
        break;

      case 'user':
        $fields = $smartling_settings->userGetFieldsSettingsByBundle($entity_data->bundle);
        break;

      default:
        $fields = array();
        break;
    }

    // Preproccess for entity type
    switch ($entity_type) {
      case 'node':
        if (smartling_nodes_method($bundle)) {
          $original_node = node_load($rid);
          $translations = translation_node_get_translations($original_node->tnid);
          if (isset($translations[$d_locale])) {
            $rid = $translations[$d_locale]->nid;
          }
          else {
            // If node not exist, need clone
            $node = clone $original_node;
            unset($node->nid);
            unset($node->vid);
            node_object_prepare($node);
            $node->language = $d_locale;
            $node->uid = $entity_data->submitter;
            $node->tnid = $original_node->nid;

            $node_fields = field_info_instances('node', $original_node->type);
            foreach ($node_fields as $field) {
              $field_info = field_info_field($field['field_name']);
              if (($field_info['type'] == 'taxonomy_term_reference') && ($field_info['translatable'] == '1')) {
                foreach ($original_node->{$field['field_name']} as $language => $items) {
                  foreach ($items as $index => $item) {
                    $term = taxonomy_term_load($original_node->{$field['field_name']}[$original_node->language][$index]['tid']);
                    if ($translation = i18n_taxonomy_term_get_translation($term, $d_locale)) {
                      $node->{$field['field_name']}[$d_locale][$index] = array(
                        'taxonomy_term' => $translation,
                        'tid' => $translation->tid
                      );
                    }
                    $field['settings']['options_list_callback'] = 'i18n_taxonomy_allowed_values';
                  }
                }
              }
              else {
                $node->{$field['field_name']} = $original_node->{$field['field_name']};
              }
            }

            node_object_prepare($node);
            node_save($node);

            $rid = $node->nid;
          }
        }
        break;

      case 'taxonomy_term':
        $original_term = taxonomy_term_load($rid);
        $term = i18n_taxonomy_term_get_translation($original_term, $d_locale);
        if (!is_null($term) && ($term->language != $original_term->language)) {
          $rid = $term->tid;
        }
        else {
          // If term not exist, need create new term
          $vocabulary = taxonomy_vocabulary_machine_name_load($bundle);

          // Add language field or not depending on taxonomy mode.
          $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
          switch ($vocabulary_mode) {
            case I18N_MODE_TRANSLATE:

              // If the term to be added will be a translation of a source term,
              // set the default value of the option list to the target language and
              // create a form element for storing the translation set of the source term.
              $source_term = taxonomy_term_load($rid);
              $term = clone $source_term;
              unset($term->tid);

              $target_language = i18n_language_object($d_locale);
              // Set context language to target language.
              i18n_language_context($target_language);

              $term->language = $target_language->language;

              // Add the translation set to the form so we know the new term
              // needs to be added to that set.
              if (!empty($source_term->i18n_tsid)) {
                $translation_set = i18n_taxonomy_translation_set_load($source_term->i18n_tsid);
              }
              else {
                // No translation set yet, build a new one with the source term.
                $translation_set = i18n_translation_set_create('taxonomy_term', $vocabulary->machine_name)
                  ->add_item($source_term);
                taxonomy_term_save($source_term);
              }
              $term->i18n_tsid = $translation_set->tsid;

              break;

            case I18N_MODE_LANGUAGE:
            case I18N_MODE_LOCALIZE:
            case I18N_MODE_NONE:
              smartling_watchdog('Translatable @entity_type with id - @rid FAIL. Vocabulary mode - @vocabulary_mode', TRUE, array(
                '@entity_type' => $entity_type,
                '@rid' => $rid,
                '@vocabulary_mode' => $vocabulary_mode
              ), WATCHDOG_INFO);
              return FALSE;
              break;
            default:
              smartling_watchdog('Translatable @entity_type with id - @rid FAIL', TRUE, array(
                '@entity_type' => $entity_type,
                '@rid' => $rid
              ), WATCHDOG_INFO);
              return FALSE;
              break;
          }
          // Update field
          foreach ($fields as $field_name) {
            if (!empty($term->{$field_name}[$original_term->language])) {
              $term->{$field_name}[$term->language] = $term->{$field_name}[$original_term->language];
              unset($term->{$field_name}[$original_term->language]);
            }
          }
          taxonomy_term_save($term);
          $rid = $term->tid;
        }


        break;

      case 'comment':
        break;

      case 'user':
        break;

      default:
        break;
    }

    $xml = new DOMDocument();
    $xml->load($file_path);

    if (smartling_copy_translations_from_xml_to_fields($entity_type, $xml, $rid, $d_locale, $entity_data->original_language, $fields)) {

      $entity_data->download = 1;
      smartling_entity_data_save($entity_data);
      smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_UPDATE_FIELDS);

      $link_to_entity = smartling_get_link_to_entity($rid, $entity_type);
      smartling_watchdog('Smartling updates all translated fields for @entity_type id - @rid. Locale: @locale', TRUE, array(
        '@entity_type' => $entity_type,
        '@rid' => $rid,
        '@locale' => $d_locale
      ), WATCHDOG_INFO, $link_to_entity);

      // Update Translations info
      switch ($entity_type) {
        case 'taxonomy_term':

          break;

        case 'node':
        case 'comment':
        case 'user':
          if (($entity_type == 'node') && smartling_nodes_method($entity_data->bundle)) {
            break;
          }
          $entity_load = entity_load_single($entity_type, $rid);
          $handler = smartling_entity_translation_get_handler($entity_type, $entity_load);
          $translations = $handler->getTranslations();

          // Initialize translations if they are empty.
          if (empty($translations->original)) {
            $handler->initTranslations();
            smartling_entity_translation_save($handler, $entity_load);
            $translations = $handler->getTranslations();
          }

          $entity_translation = array(
            'entity_type' => $entity_type,
            'entity_id' => $rid,
            'translate' => '0',
            'status' => $entity_load->status,
            'language' => $d_locale,
            'uid' => $entity_data->submitter,
            'changed' => $entity_data->submission_date,
          );

          if (isset($translations->data[$d_locale])) {
            $handler->setTranslation($entity_translation);
          }
          else {
            // Add the new translation.
            $entity_translation += array(
              'source' => $translations->original,
              'created' => $entity_load->created,
            );
            $handler->setTranslation($entity_translation);
          }
          smartling_entity_translation_save($handler, $entity_load);

          break;

        default:
          break;
      }

      if (module_exists('rules') && ($entity_type == 'node')) {
        $node_event = node_load($rid);
        rules_invoke_event('smartling_translation_download_event', $node_event);
      }
    }
    else {
      smartling_watchdog('Translatable @entity_type with id - @rid not found in database', TRUE, array(
        '@entity_type' => $entity_type,
        '@rid' => $rid
      ), WATCHDOG_INFO);
    }
  }
}

/**
 *
 * @param type $entity_type
 * @param type $xml
 * @param type $rid
 * @param type $d_locale
 * @param type $original_locale
 * @param type $fields
 * @return boolean
 */
function smartling_copy_translations_from_xml_to_fields($entity_type, $xml, $rid, $d_locale, $original_locale, $fields) {
  $xpath = new DomXpath($xml);
  $entity_wrapper = entity_load_single($entity_type, $rid);
  if ($entity_wrapper) {
    foreach ($fields as $field_name) {
      // Get language key for field translatable type
      if (smartling_field_is_translatable_by_field_name($field_name)) {
        $language_key = $d_locale;
        $language_key_format = $original_locale;
      }
      else {
        $language_key = LANGUAGE_NONE;
        $language_key_format = LANGUAGE_NONE;
      }

      switch (smartling_get_update_field_processing_mode($field_name, $xpath)) {
        case 'text-field':
          $entity_wrapper = smartling_text_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format);
          break;

        case 'body-with-summary-field':
          $entity_wrapper = smartling_body_with_summary_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format);
          break;

        case 'image-field':
          $entity_wrapper = smartling_image_field_processing($entity_wrapper, $field_name, $xpath, $language_key);
          break;

        default:
          smartling_watchdog("Smartling didn't process field - @field_name", TRUE, array('@field_name' => $field_name), WATCHDOG_INFO);
          break;
      }
    }

    entity_save($entity_type, $entity_wrapper);
    return TRUE;
  }
  else {
    return FALSE;
  }
}
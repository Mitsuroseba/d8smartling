<?php

/**
 * @file
 * Processing functions for Smartling module.
 */
use Drupal\smartling\Processors\GenericEntityProcessor;

/**
 * Get field processing mode.
 *
 * @param string $field_name
 *   Field name.
 *
 * @deprecated
 *
 * @return string|bool
 *   Return processing mode or FALSE.
 */
function smartling_get_field_content_processing_mode($field_name) {
  $processing_mode = FALSE;
  $info = field_info_field($field_name);
  $swap_array = array(
    'title_field',
    'name_field',
    'description_field',
    'subject_field',
  );

  if (smartling_check_fake_field_for_entities_translate_method($field_name)) {
    $processing_mode = $field_name;
  }
  elseif (in_array($field_name, $swap_array)) {
    $processing_mode = 'text-field';
  }
  else {
    switch ($info['type']) {
      case 'text':
        $processing_mode = 'text-field';
        break;

      case 'text_with_summary':
        $processing_mode = 'body-with-summary-field';
        break;

      case 'text_long':
        $processing_mode = 'text-field';
        break;

      case 'image':
        $processing_mode = 'image-field';
        break;

      default:
        $processing_mode = FALSE;
        break;
    }
  }
  return $processing_mode;
}

/**
 * Get content to translate for node entity.
 *
 * @param int $nid
 *   Node id.
 *
 * @deprecated
 *
 * @return array
 *   Structured array of content that will be sent for translation.
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
    $language = (smartling_field_is_translatable_by_field_name($field_name, 'node')) ? entity_language('node', $node) : LANGUAGE_NONE;
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'title_property_field':
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
        $log = smartling_log_get_handler();
        $log->setMessage("Smartling didn't process content of field - @field_name")
          ->setVariables(array('@field_name' => $field_name))
          ->setConsiderLog(FALSE)
          ->execute();
        break;
    }
  }
  return $node_current_translatable_content;
}

/**
 * Get content to translate for taxonomy_term entity.
 *
 * @param int $tid
 *   Term id.
 *
 * @deprecated
 *
 * @return array
 *   Structured array of content that will be sent for translation.
 */
function smartling_taxonomy_term_content_to_translate($tid) {
  $term_current_translatable_content = array();
  if (is_numeric($tid)) {
    $term = taxonomy_term_load($tid);
  }
  else {
    $term = $tid;
  }

  foreach (smartling_settings_get_handler()->taxonomyTermGetFieldsSettingsByBundle($term->vocabulary_machine_name) as $field_name) {
    $language = (smartling_field_is_translatable_by_field_name($field_name, 'taxonomy_term')) ? entity_language('taxonomy_term', $term) : LANGUAGE_NONE;
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'name_property_field':
        if (!empty($term->name)) {
          $term_current_translatable_content[$field_name][0] = $term->name;
        }
        break;

      case 'description_property_field':
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
        $log = smartling_log_get_handler();
        $log->setMessage("Smartling didn't process content of field - @field_name")
          ->setVariables(array('@field_name' => $field_name))
          ->setConsiderLog(FALSE)
          ->execute();
        break;
    }
  }

  return $term_current_translatable_content;
}

/**
 * Get content to translate for comment entity.
 *
 * @param int $cid
 *   Comment id.
 *
 * @deprecated
 *
 * @return array
 *   Structured array of content that will be sent for translation.
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
    $language = (smartling_field_is_translatable_by_field_name($field_name, 'comment')) ? entity_language('comment', $comment) : LANGUAGE_NONE;
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
        $log = smartling_log_get_handler();
        $log->setMessage("Smartling didn't process content of field - @field_name")
          ->setVariables(array('@field_name' => $field_name))
          ->setConsiderLog(FALSE)
          ->execute();
        break;
    }
  }

  return $comment_current_translatable_content;
}

/**
 * Get content to translate for user entity.
 *
 * @param int $uid
 *   User id.
 *
 * @deprecated
 *
 * @return array
 *   Structured array of content that will be sent for translation.
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
    $language = (smartling_field_is_translatable_by_field_name($field_name, 'user')) ? entity_language('user', $user) : LANGUAGE_NONE;
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
        $log = smartling_log_get_handler();
        $log->setMessage("Smartling didn't process content of field - @field_name")
          ->setVariables(array('@field_name' => $field_name))
          ->setConsiderLog(FALSE)
          ->execute();
        break;
    }
  }

  return $user_current_translatable_content;
}

/**
 * This function return processing mode for field.
 *
 * @param string $field_name
 *   Field name.
 * @param object $xpath
 *   Xpath object.
 *
 * @deprecated
 *
 * @return string|bool
 *   Return processing mode or FALSE.
 */
function smartling_get_update_field_processing_mode($field_name, $xpath) {
  // Check field variant.
  $text_field = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')
    ->item(0);
  $body = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')
    ->item(0);
  $summary = $xpath->query('//string[@id="' . $field_name . '-summary-0' . '"][1]')
    ->item(0);
  $image_alt = $xpath->query('//string[@id="' . $field_name . '-alt-img-0' . '"][1]')
    ->item(0);
  $image_title = $xpath->query('//string[@id="' . $field_name . '-title-img-0' . '"][1]')
    ->item(0);

  if (is_object($text_field)) {
    // If this field is a text field.
    if ($text_field instanceof DomElement) {
      return 'text-field';
    }
  }
  elseif (is_object($body) || is_object($summary)) {
    // If this field is a body/summary.
    if (($body instanceof DomElement) || ($summary instanceof DomElement)) {
      return 'body-with-summary-field';
    }
  }
  elseif (is_object($image_alt) || is_object($image_title)) {
    // If this field is a image.
    if (($image_alt instanceof DomElement) || ($image_title instanceof DomElement)) {
      return 'image-field';
    }
  }
  return FALSE;
}

/**
 * Processing for text field.
 * @deprecated
 */
function smartling_text_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_entity, $fields_method) {
  $quantity_value = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')
    ->item(0);
  $quantity = $quantity_value->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $field = $xpath->query('//string[@id="' . $field_name . '-' . $i . '"][1]')
      ->item(0);
    $string = smartling_process_content((string) $field->nodeValue, $language_key, $field_name, $original_entity);

    if (!smartling_check_fake_field_for_entities_translate_method($field_name)) {
      $format = smartling_get_field_format($original_entity, $field_name, $language_key_format, $i);
      if (smartling_is_title_module_field($field_name) && !$fields_method) {
        foreach ($entity_wrapper->{$field_name} as $lang => $values) {
          foreach (array_keys($values) as $delta) {
            unset($entity_wrapper->{$field_name}[$lang][$delta]);
          }
        }
        $property_name = str_ireplace('_field', '', $field_name);
        $entity_wrapper->{$property_name} = $string;
      }

      $entity_wrapper->{$field_name}[$language_key][$i]['value'] = $string;
      $entity_wrapper->{$field_name}[$language_key][$i]['format'] = $format;
    }
    else {
      $property_name = str_ireplace('_property_field', '', $field_name);
      $entity_wrapper->{$property_name} = $string;
    }
  }
  return $entity_wrapper;
}

/**
 * Processing for body/summary field.
 * @deprecated
 */
function smartling_body_with_summary_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_entity) {
  $quantity_value = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')
    ->item(0);
  $quantity = $quantity_value->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $body_obj = $xpath->query('//string[@id="' . $field_name . '-body-' . $i . '"][1]')
      ->item(0);
    $summary_obj = $xpath->query('//string[@id="' . $field_name . '-summary-' . $i . '"][1]')
      ->item(0);

    $body_string = smartling_process_content((string) $body_obj->nodeValue, $language_key, $field_name, $original_entity);
    $summary_string = smartling_process_content((string) $summary_obj->nodeValue, $language_key, $field_name, $original_entity);
    $format = smartling_get_field_format($original_entity, $field_name, $language_key_format, $i);

    $entity_wrapper->{$field_name}[$language_key][$i]['value'] = $body_string;
    $entity_wrapper->{$field_name}[$language_key][$i]['format'] = $format;
    $entity_wrapper->{$field_name}[$language_key][$i]['summary'] = $summary_string;
  }
  return $entity_wrapper;
}

/**
 * Processing for image field.
 * @deprecated
 */
function smartling_image_field_processing($entity_wrapper, $field_name, $xpath, $language_key) {
  $image_multiple = $xpath->query('//string[@id="' . $field_name . '-alt-img-0' . '"][1]')
    ->item(0);
  $quantity = $image_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $alt_obj = $xpath->query('//string[@id="' . $field_name . '-alt-img-' . $i . '"][1]')
      ->item(0);
    $title_obj = $xpath->query('//string[@id="' . $field_name . '-title-img-' . $i . '"][1]')
      ->item(0);
    $fid = $alt_obj->getAttribute('fid');
    $file_img = file_load($fid);

    if ($file_img) {
      $alt_string = (string) $alt_obj->nodeValue;
      $title_string = (string) $title_obj->nodeValue;
      $file_img->alt = $alt_string;
      $file_img->title = $title_string;
      $entity_wrapper->{$field_name}[$language_key][$i] = (array) $file_img;
    }
  }
  return $entity_wrapper;
}

/**
 * Parsing the translated file and record the results in the field.
 *
 * @param int $rid
 *   Related id.
 * @param string $d_locale
 *   Locale in drupal format (ru, en).
 * @param string $entity_type
 *   Entity type.
 * @param string $bundle
 *   Entity bundle.
 */
function smartling_update_translated_fields($smartling_entity) {
  $log = smartling_log_get_handler();
  $log->setMessage('Smartling starts update fields of entity @entity_type, id - @rid. Locale: @locale')
    ->setVariables(array(
      '@entity_type' => $smartling_entity->entity_type,
      '@rid' => $smartling_entity->rid,
      '@locale' => $smartling_entity->target_language,
    ))
    ->setSeverity(WATCHDOG_DEBUG)
    ->execute();

  if (smartling_supported_type($smartling_entity->entity_type, $smartling_entity->bundle)) {
    /* @var $processor GenericEntityProcessor */
    $processor = smartling_get_entity_processor($smartling_entity);
    $smartling_settings = smartling_settings_get_handler();

    $file_path = smartling_clean_filename(smartling_get_dir($smartling_entity->translated_file_name), TRUE);

    $xml = new DOMDocument();
    $xml->load(drupal_realpath($file_path));

    $processor->updateEntityFromXML($xml);

    if (module_exists('rules') && ($smartling_entity->entity_type == 'node')) {
      $node_event = node_load($smartling_entity->rid);
      rules_invoke_event('smartling_translation_download_event', $node_event);
    }
  }
}

/**
 * Copy translations from xml to fields.
 *
 * @param string $entity_type
 *   Entity type.
 * @param object $xml
 *   Xml object.
 * @param int $rid
 *   Related id.
 * @param string $d_locale
 *   Locale in drupal format (ru, en).
 * @param string $original_locale
 *   Original locale in drupal format (ru, en).
 * @param array $fields
 *   Fields array.
 * @param object $original_entity
 *   Original entity.
 * @param bool $fields_method
 *   TRUE if use fields method.
 *
 * @return bool
 *   Return TRUE if copy fields.
 */
function smartling_copy_translations_from_xml_to_fields($entity_type, $xml, $rid, $d_locale, $original_locale, array $fields, $original_entity, $fields_method) {
  $xpath = new DomXpath($xml);
  $entity_wrapper = entity_load_single($entity_type, $rid);
  if ($entity_wrapper) {
    foreach ($fields as $field_name) {
      // Get language key for field translatable type.
      if (smartling_field_is_translatable_by_field_name($field_name, $entity_type)) {
        $language_key = $d_locale;
        $language_key_format = $original_locale;
      }
      else {
        $language_key = LANGUAGE_NONE;
        $language_key_format = LANGUAGE_NONE;
      }

      switch (smartling_get_update_field_processing_mode($field_name, $xpath)) {
        case 'text-field':
          $entity_wrapper = smartling_text_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_entity, $fields_method);
          break;

        case 'body-with-summary-field':
          $entity_wrapper = smartling_body_with_summary_field_processing($entity_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_entity);
          break;

        case 'image-field':
          $entity_wrapper = smartling_image_field_processing($entity_wrapper, $field_name, $xpath, $language_key);
          break;

        default:
          $log = smartling_log_get_handler();
          $log->setMessage("Smartling didn't process field - @field_name")
            ->setVariables(array('@field_name' => $field_name))
            ->execute();
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

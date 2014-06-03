<?php

/**
 * @file
 * Processing functions for Smartling module.
 */

/**
 *
 * @param type $field_name
 * @param type $node
 * @return string|boolean
 */
function smartling_get_field_content_processing_mode($field_name) {
  $info = field_info_field($field_name);

  if ($field_name == 'node_title') {
    return 'node-title';
  }
  elseif ($field_name == 'title_field') {
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
    $node = smartling_node_load_default($nid);
  }
  else {
    $node = $nid;
  }

  $translate = variable_get('smartling_translate_fields', array());

  foreach ($translate as $type => $field_names) {
    if ($node->type === $type) {
      foreach ($field_names as $field_name) {

        $language = (smartling_field_is_translatable_by_field_name($field_name)) ? $node->language : LANGUAGE_NONE;

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
            smartling_watchdog('Smartling didn\'t process content of field - @field_name', FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
            break;
        }
      }
    }
  }
  return $node_current_translatable_content;
}

/**
 * This function return processing mode for field
 *
 * @param type $field_name
 * @param type $xpath
 * @return string
 */
function smartling_get_update_field_processing_mode($field_name, $xpath) {
  if ($field_name == 'node_title') {
    return 'node-title';
  }
  else {
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
  }
  return FALSE;
}

/**
 * Processing for node title ($node->title)
 */
function smartling_node_title_processing($node_wrapper, $field_name, $xpath) {
  $node_title = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
  if ($node_title instanceof DomElement) {
    $string = (string) $node_title->nodeValue;
    $node_wrapper->title = $string;
  }
  else {
    $node_title = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
    if ($node_title instanceof DomElement) {
      $string = (string) $node_title->nodeValue;
      $node_wrapper->title = $string;
    }
    else {
      drupal_set_message(t('Please re-upload entity <em>@title</em>', array('@title' => $node_wrapper->title)), 'error');
    }
  }
  return $node_wrapper;
}

/**
 * Processing for text field
 */
function smartling_text_field_processing($node_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_node) {
  $simple_field_multiple = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
  $quantity = $simple_field_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $field = $xpath->query('//string[@id="' . $field_name . '-' . $i . '"][1]')->item(0);
    $string = (string) $field->nodeValue;
    $format = smartling_get_field_format($original_node, $field_name, $language_key_format, $i);

    $node_wrapper->{$field_name}[$language_key][$i]['value'] = $string;
    $node_wrapper->{$field_name}[$language_key][$i]['format'] = $format;
  }
  return $node_wrapper;
}

/**
 * Processing for body/summary field
 */
function smartling_body_with_summary_field_processing($node_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_node) {
  $body_with_summary_multiple = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);
  $quantity = $body_with_summary_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $body_obj = $xpath->query('//string[@id="' . $field_name . '-body-' . $i . '"][1]')->item(0);
    $summary_obj = $xpath->query('//string[@id="' . $field_name . '-summary-' . $i . '"][1]')->item(0);
    $body_string = (string) $body_obj->nodeValue;
    $summary_string = (string) $summary_obj->nodeValue;
    $format = smartling_get_field_format($original_node, $field_name, $language_key_format, $i);

    $node_wrapper->{$field_name}[$language_key][$i]['value'] = $body_string;
    $node_wrapper->{$field_name}[$language_key][$i]['format'] = $format;
    $node_wrapper->{$field_name}[$language_key][$i]['summary'] = $summary_string;
  }
  return $node_wrapper;
}

/**
 * Processing for image field
 */
function smartling_image_field_processing($node_wrapper, $field_name, $xpath, $language_key) {
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
      $node_wrapper->{$field_name}[$language_key][$i] = (array) $file_img + array(
          'alt' => $alt_string,
          'title' => $title_string
        );
    }
  }
  return $node_wrapper;
}

/**
 * Parsing the translated file and record the results in the field.
 *
 * @param type $rid
 * @param type $s_locale locale in Smartling format
 * @param bool $nodes_method
 * If TRUE - nodes method execute
 */
function smartling_update_translated_fields($rid, $s_locale, $nodes_method = FALSE) {
  $d_locale = smartling_convert_locale_drupal_to_smartling($s_locale, TRUE);
  smartling_watchdog('Smartling starts to update fields of node id - @nid. Locale: @locale', TRUE, array(
    '@nid' => $rid,
    '@locale' => $s_locale
  ), WATCHDOG_DEBUG);

  $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
  $file_name = $entity_data->translated_file_name;
  $file_path = smartling_get_dir() . '/' . $file_name;
  $original_node = smartling_node_load_default($rid);

  if ($nodes_method) {
    $translations = translation_node_get_translations($original_node->tnid);
    if (isset($translations[$d_locale])) {
      $rid = $translations[$d_locale]->nid;
    }
    else {
      // If node not exist, need create new node
      $node = clone $original_node;
      unset($node->nid);
      unset($node->vid);
      node_object_prepare($node);
      $node->language = $d_locale;
      $node->uid = $entity_data->submitter;
      $node->tnid = $original_node->nid;

      node_save($node);

      $rid = $node->nid;
    }
  }
  $smartling_translate_fields = variable_get('smartling_translate_fields', array());

  $fields = $smartling_translate_fields[$entity_data->bundle];

  $xml = new DOMDocument();

  $xml->load($file_path);
  $xpath = new DomXpath($xml);

  $node_wrapper = node_load($rid);
  if ($node_wrapper) {
    foreach ($fields as $field_name) {
      // Get language key for field translatable type
      if (smartling_field_is_translatable_by_field_name($field_name)) {
        $language_key = $d_locale;
        $language_key_format = smartling_convert_locale_drupal_to_smartling($entity_data->original_language, TRUE);
      }
      else {
        $language_key = LANGUAGE_NONE;
        $language_key_format = LANGUAGE_NONE;
      }

      switch (smartling_get_update_field_processing_mode($field_name, $xpath)) {
        case 'node-title':
          $node_wrapper = smartling_node_title_processing($node_wrapper, $field_name, $xpath);
          break;

        case 'text-field':
          $node_wrapper = smartling_text_field_processing($node_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_node);
          break;

        case 'body-with-summary-field':
          $node_wrapper = smartling_body_with_summary_field_processing($node_wrapper, $field_name, $xpath, $language_key, $language_key_format, $original_node);
          break;

        case 'image-field':
          $node_wrapper = smartling_image_field_processing($node_wrapper, $field_name, $xpath, $language_key);
          break;

        default:
          smartling_watchdog('Smartling didn\'t process field - @field_name', TRUE, array('@field_name' => $field_name), WATCHDOG_INFO);
          break;
      }
    }

    node_save($node_wrapper);

    $entity_data->download = 1;

    smartling_entity_data_save($entity_data);

    smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_UPDATE_FIELDS);

    smartling_watchdog('Smartling updates all translated fields for node id - @nid. Locale: @locale', TRUE, array(
      '@nid' => $rid,
      '@locale' => $s_locale
    ), WATCHDOG_DEBUG, l(t('View translated node'), $d_locale . '/node/' . $rid));
    if (module_exists('rules')) {
      $node_event = smartling_node_load_default($rid);
      rules_invoke_event('smartling_translation_download_event', $node_event);
    }
  }
  else {
    smartling_watchdog('Translatable node with nid - @nid was not found in database', TRUE, array('@nid' => $rid), WATCHDOG_WARNING);
  }
}

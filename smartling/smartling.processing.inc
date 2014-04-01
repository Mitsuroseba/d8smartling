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
    if ($info['cardinality'] == 1) {
      return 'simple-text-field';
    }
    else {
      return 'multiple-text-field';
    }
  }
  else {
    if ($info['cardinality'] == 1) {
      switch ($info['type']) {
        case 'text':
          return 'simple-text-field';
          break;
        case 'text_with_summary':
          return 'body-with-summary-field';
          break;
        case 'text_long':
          return 'simple-text-field';
          break;
        case 'image':
          return 'simple-image-field';
          break;

        default:
          break;
      }
    }
    else {
      switch ($info['type']) {
        case 'text':
          return 'multiple-text-field';
          break;
        case 'text_with_summary':
          return 'body-with-summary-multiple-field';
          break;
        case 'text_long':
          return 'multiple-text-field';
          break;
        case 'image':
          return 'multiple-image-field';
          break;

        default:
          break;
      }
    }
  }
  return FALSE;
}

/**
 *
 * @param type $nid
 * Node id
 * @param type $nodes_method
 * FALSE by default
 * @return array
 * Structured array of content that will be sent for translation
 */
function smartling_node_content_to_translate($nid, $nodes_method = FALSE) {
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
        switch (smartling_get_field_content_processing_mode($field_name)) {
          case 'node-title':
            $node_current_translatable_content[$field_name] = $node->title;
            break;

          case 'simple-text-field':
            $node_current_translatable_content[$field_name] = $node->{$field_name}[$node->language][0]['value'];
            break;

          case 'multiple-text-field':
            foreach ($node->{$field_name}[$node->language] as $key => $item) {
              $node_current_translatable_content[$field_name][$key] = $item['value'];
            }
            break;

          case 'body-with-summary-field':
            $node_current_translatable_content[$field_name]['body'] = $node->{$field_name}[$node->language][0]['value'];
            $node_current_translatable_content[$field_name]['summary'] = $node->{$field_name}[$node->language][0]['summary'];
            break;

          case 'body-with-summary-multiple-field':
            foreach ($node->{$field_name}[$node->language] as $key => $item) {
              $node_current_translatable_content[$field_name][$key]['body'] = $item['value'];
              $node_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
            }
            break;

          case 'simple-image-field':
            if (is_array($node->{$field_name}) && !empty($node->{$field_name}[$node->language])) {
              $node_current_translatable_content[$field_name]['alt-img'] = $node->{$field_name}[$node->language][0]['alt'];
              $node_current_translatable_content[$field_name]['title-img'] = $node->{$field_name}[$node->language][0]['title'];
              $node_current_translatable_content[$field_name]['fid-img'] = $node->{$field_name}[$node->language][0]['fid'];
            }
            break;

          case 'multiple-image-field':
            if (is_array($node->{$field_name}) && !empty($node->{$field_name}[$node->language])) {
              foreach ($node->{$field_name}[$node->language] as $key => $item) {
                $node_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
                $node_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
                $node_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
              }
            }
            break;

          default:
            smartling_watchdog('Smartling didn\'t process content of field - @field_name', FALSE, array('@field_name' => $field_name), WATCHDOG_WARNING);
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
    $simple_field = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
    $simple_field_multiple = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
    $body_with_summary = $xpath->query('//string[@id="' . $field_name . '-body' . '"][1]')->item(0);
    $body_with_summary_multiple = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);
    $image = $xpath->query('//string[@id="' . $field_name . '-alt-img' . '"][1]')->item(0);
    $image_multiple = $xpath->query('//string[@id="' . $field_name . '-alt-img-0' . '"][1]')->item(0);

    if (is_object($simple_field)) {
      // If this field is a simple field
      if ($simple_field instanceof DomElement) {
        return 'simple-text-field';
      }
    }
    elseif (is_object($simple_field_multiple)) {
      // If this field is a simple multiple field
      if ($simple_field_multiple instanceof DomElement) {
        return 'multiple-text-field';
      }
    }
    elseif (is_object($body_with_summary)) {
      // If this field is a simple body/summary field
      if ($body_with_summary instanceof DomElement) {
        return 'body-with-summary-field';
      }
    }
    elseif (is_object($body_with_summary_multiple)) {
      // If this field is a multiple body/summary
      if ($body_with_summary_multiple instanceof DomElement) {
        return 'body-with-summary-multiple-field';
      }
    }
    elseif (is_object($image)) {
      // If this field is a simple image field
      if ($image instanceof DomElement) {
        return 'simple-image-field';
      }
    }
    elseif (is_object($image_multiple)) {
      // If this field is a multiple image
      if ($image_multiple instanceof DomElement) {
        return 'multiple-image-field';
      }
    }
  }
  return FALSE;
}

/**
 * Processing for node title ($node->title)
 */
function smartling_node_title_processing($node_wrapper, $field_name, $xpath) {
  $node_title = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
  $string = (string) $node_title->nodeValue;
  $node_wrapper->title = $string;
  return $node_wrapper;
}

/**
 * Processing for single text field
 */
function smartling_simple_text_field_processing($node_wrapper, $field_name, $xpath, $d_locale) {
  $simple_field = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
  $string = (string) $simple_field->nodeValue;
  $info = $node_wrapper->language($d_locale)->{$field_name}->info();

  if (isset($info['property info'])) {
    $node_wrapper->language($d_locale)->{$field_name}->value = $string;
  }
  else {
    $node_wrapper->language($d_locale)->{$field_name}->set($string);
  }
  return $node_wrapper;
}

/**
 * Processing for multiple text field
 */
function smartling_multiple_text_field_processing($node_wrapper, $field_name, $xpath, $d_locale) {
  $simple_field_multiple = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
  $quantity = $simple_field_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $field = $xpath->query('//string[@id="' . $field_name . '-' . $i . '"][1]')->item(0);
    $string = (string) $field->nodeValue;
    $info = $node_wrapper->language($d_locale)->{$field_name}[$i]->info();

    if (isset($info['property info'])) {
      $node_wrapper->language($d_locale)->{$field_name}[$i]->value = $string;
    }
    else {
      $node_wrapper->language($d_locale)->{$field_name}[$i]->set($string);
    }
  }
  return $node_wrapper;
}

/**
 * Processing for single body/summary field
 */
function smartling_body_with_summary_field_processing($node_wrapper, $field_name, $xpath, $d_locale) {
  $body_with_summary = $xpath->query('//string[@id="' . $field_name . '-body' . '"][1]')->item(0);

  $body = (string) $body_with_summary->nodeValue;
  $summary = $xpath->query('//string[@id="' . $field_name . '-summary' . '"][1]')->item(0);
  $summary = (string) $summary->nodeValue;
  $info = $node_wrapper->language($d_locale)->{$field_name}->info();

  if (isset($info['property info'])) {
    $node_wrapper->language($d_locale)->{$field_name}->value = $body;
    $node_wrapper->language($d_locale)->{$field_name}->summary = $summary;
  }
  else {
    $node_wrapper->language($d_locale)->{$field_name}->set(array('value' => $body, 'summary' => $summary));
  }
  return $node_wrapper;
}

/**
 * Processing for multiple body/summary field
 */
function smartling_body_with_summary_multiple_field_processing($node_wrapper, $field_name, $xpath, $d_locale) {
  $body_with_summary_multiple = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);
  $quantity = $body_with_summary_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $body_obj = $xpath->query('//string[@id="' . $field_name . '-body-' . $i . '"][1]')->item(0);
    $summary_obj = $xpath->query('//string[@id="' . $field_name . '-summary-' . $i . '"][1]')->item(0);
    $body_string = (string) $body_obj->nodeValue;
    $summary_string = (string) $summary_obj->nodeValue;
    $info = $node_wrapper->language($d_locale)->{$field_name}[$i]->info();

    if (isset($info['property info'])) {
      $node_wrapper->language($d_locale)->{$field_name}[$i]->value = $body_string;
      $node_wrapper->language($d_locale)->{$field_name}[$i]->summary = $summary_string;
    }
    else {
      $node_wrapper->language($d_locale)->{$field_name}[$i]->set(array('value' => $body_string, 'summary' => $summary_string));
    }
  }
  return $node_wrapper;
}

/**
 * Processing for single image field
 */
function smartling_simple_image_field_processing($node_wrapper, $field_name, $xpath, $d_locale) {
  $alt_img = $xpath->query('//string[@id="' . $field_name . '-alt-img' . '"][1]')->item(0);
  $fid = $alt_img->getAttribute('fid');
  $alt_img = (string) $alt_img->nodeValue;
  $title_img = $xpath->query('//string[@id="' . $field_name . '-title-img' . '"][1]')->item(0);
  $title_img = (string) $title_img->nodeValue;
  $file_img = file_load($fid);

  if ($file_img) {
    $node_wrapper->language($d_locale)->{$field_name} = (array) $file_img + array('alt' => $alt_img, 'title' => $title_img);
  }
  return $node_wrapper;
}

/**
 * Processing for multiple image field
 */
function smartling_multiple_image_field_processing($node_wrapper, $field_name, $xpath, $d_locale) {
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
      $node_wrapper->language($d_locale)->{$field_name}[$i] = (array) $file_img + array('alt' => $alt_string, 'title' => $title_string);
    }
  }
  return $node_wrapper;
}

/**
 * Parsing the translated file and record the results in the field.
 *
 * @param type $nid
 * @param type $s_locale locale in Smartling format
 * @param bool $nodes_method
 * If TRUE - nodes method execute
 */
function smartling_update_translated_fields($rid, $s_locale, $nodes_method = FALSE) {
  $d_locale = smartling_convert_locale_drupal_to_smartling($s_locale, TRUE);
  smartling_watchdog('Smartling starts to update fields of node id - @nid. Locale: @locale', TRUE, array('@nid' => $rid, '@locale' => $s_locale), WATCHDOG_DEBUG);

  $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
  $file_name = $entity_data->translated_file_name;
  $file_path = SMARTLING_DIRECTORY . '/' . $file_name;

  if ($nodes_method) {
    $original_node = smartling_node_load_default($rid);
    $translations = translation_node_get_translations($original_node->tnid);
    if (isset($translations[$d_locale])) {
      $rid = $translations[$d_locale]->nid;
    }
    else {
      // If node not exist, need create new node
      $node = new stdClass();
      $node->type = $original_node->type;
      node_object_prepare($node);
      $node->title = $original_node->title;
      $node->language = $d_locale;
      $node->uid = $entity_data->submitter;
      $node->tnid = $original_node->nid;

      $node_fields = field_info_instances('node', $original_node->type);
      foreach ($node_fields as $field) {
        $node->{$field['field_name']} = $original_node->{$field['field_name']};
      }

      node_save($node);

      $rid = $node->nid;
    }
  }
  $smartling_translate_fields = variable_get('smartling_translate_fields');

  $fields = $smartling_translate_fields[$entity_data->bundle];

  $xml = new DOMDocument();

  $xml->load($file_path);
  $xpath = new DomXpath($xml);

  $node_wrapper = entity_metadata_wrapper('node', $rid);
  if (!is_null($node_wrapper->label())) {
    foreach ($fields as $field_name) {

      switch (smartling_get_update_field_processing_mode($field_name, $xpath)) {
        case 'node-title':
          $node_wrapper = smartling_node_title_processing($node_wrapper, $field_name, $xpath);
          break;

        case 'simple-text-field':
          $node_wrapper = smartling_simple_text_field_processing($node_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'multiple-text-field':
          $node_wrapper = smartling_multiple_text_field_processing($node_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'body-with-summary-field':
          $node_wrapper = smartling_body_with_summary_field_processing($node_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'body-with-summary-multiple-field':
          $node_wrapper = smartling_body_with_summary_multiple_field_processing($node_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'simple-image-field':
          $node_wrapper = smartling_simple_image_field_processing($node_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'multiple-image-field':
          $node_wrapper = smartling_multiple_image_field_processing($node_wrapper, $field_name, $xpath, $d_locale);
          break;

        default:
          smartling_watchdog('Smartling didn\'t process field - @field_name', TRUE, array('@field_name' => $field_name), WATCHDOG_WARNING);
          break;
      }
    }

    $node_wrapper->save();

    $entity_data->download = 1;

    smartling_entity_data_save($entity_data);

    smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_UPDATE_FIELDS);

    smartling_watchdog('Smartling updates all translated fields for node id - @nid. Locale: @locale', TRUE, array('@nid' => $rid, '@locale' => $s_locale), WATCHDOG_DEBUG, l(t('View translated node'), $d_locale . '/node/' . $rid));
    if (module_exists('rules')) {
      $node_event = smartling_node_load_default($rid);
      rules_invoke_event('smartling_translation_download_event', $node_event);
    }
  }
  else {
    smartling_watchdog('Translatable node with nid - @nid was not found in database', TRUE, array('@nid' => $rid), WATCHDOG_WARNING);
  }
}

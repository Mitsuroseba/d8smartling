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
  if ($field_name == 'taxonomy-term-title') {
    return 'taxonomy-term-title';
  }
  if ($field_name == 'taxonomy-term-description') {
    return 'taxonomy-term-description';
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
            smartling_watchdog('Smartling not processed content of field - @field_name', FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
            break;
        }
      }
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

  $field_names = array();
  $field_names[] = 'taxonomy-term-title';
  $field_names[] = 'taxonomy-term-description';

  $field_names = array_merge($field_names, smartling_get_supported_field_names_by_type_and_bundle('taxonomy_term', $term->vocabulary_machine_name));

  foreach ($field_names as $field_name) {
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'taxonomy-term-title':
        $term_current_translatable_content[$field_name] = $term->name;
        break;
      case 'taxonomy-term-description':
        $term_current_translatable_content[$field_name] = $term->description;
        break;

      case 'simple-text-field':
        $term_current_translatable_content[$field_name] = $term->{$field_name}[LANGUAGE_NONE][0]['value'];
        break;

      case 'multiple-text-field':
        foreach ($term->{$field_name}[LANGUAGE_NONE] as $key => $item) {
          $term_current_translatable_content[$field_name][$key] = $item['value'];
        }
        break;

      case 'body-with-summary-field':
        $term_current_translatable_content[$field_name]['body'] = $term->{$field_name}[LANGUAGE_NONE][0]['value'];
        $term_current_translatable_content[$field_name]['summary'] = $term->{$field_name}[LANGUAGE_NONE][0]['summary'];
        break;

      case 'body-with-summary-multiple-field':
        foreach ($term->{$field_name}[LANGUAGE_NONE] as $key => $item) {
          $term_current_translatable_content[$field_name][$key]['body'] = $item['value'];
          $term_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
        }
        break;

      case 'simple-image-field':
        if (is_array($term->{$field_name}) && !empty($term->{$field_name}[LANGUAGE_NONE])) {
          $term_current_translatable_content[$field_name]['alt-img'] = $term->{$field_name}[LANGUAGE_NONE][0]['alt'];
          $term_current_translatable_content[$field_name]['title-img'] = $term->{$field_name}[LANGUAGE_NONE][0]['title'];
          $term_current_translatable_content[$field_name]['fid-img'] = $term->{$field_name}[LANGUAGE_NONE][0]['fid'];
        }
        break;

      case 'multiple-image-field':
        if (is_array($term->{$field_name}) && !empty($term->{$field_name}[LANGUAGE_NONE])) {
          foreach ($term->{$field_name}[LANGUAGE_NONE] as $key => $item) {
            $term_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $term_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $term_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog('Smartling not processed content of field - @field_name', FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
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

  $field_names = smartling_get_supported_field_names_by_type_and_bundle('comment', $comment->node_type);

  foreach ($field_names as $field_name) {
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'simple-text-field':
        $comment_current_translatable_content[$field_name] = $comment->{$field_name}[$comment->translations->original][0]['value'];
        break;

      case 'multiple-text-field':
        foreach ($comment->{$field_name}[$comment->translations->original] as $key => $item) {
          $comment_current_translatable_content[$field_name][$key] = $item['value'];
        }
        break;

      case 'body-with-summary-field':
        $comment_current_translatable_content[$field_name]['body'] = $comment->{$field_name}[$comment->translations->original][0]['value'];
        $comment_current_translatable_content[$field_name]['summary'] = $comment->{$field_name}[$comment->translations->original][0]['summary'];
        break;

      case 'body-with-summary-multiple-field':
        foreach ($comment->{$field_name}[$comment->translations->original] as $key => $item) {
          $comment_current_translatable_content[$field_name][$key]['body'] = $item['value'];
          $comment_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
        }
        break;

      case 'simple-image-field':
        if (is_array($comment->{$field_name}) && !empty($comment->{$field_name}[$comment->translations->original])) {
          $comment_current_translatable_content[$field_name]['alt-img'] = $comment->{$field_name}[$comment->translations->original][0]['alt'];
          $comment_current_translatable_content[$field_name]['title-img'] = $comment->{$field_name}[$comment->translations->original][0]['title'];
          $comment_current_translatable_content[$field_name]['fid-img'] = $comment->{$field_name}[$comment->translations->original][0]['fid'];
        }
        break;

      case 'multiple-image-field':
        if (is_array($comment->{$field_name}) && !empty($comment->{$field_name}[$comment->translations->original])) {
          foreach ($comment->{$field_name}[$comment->translations->original] as $key => $item) {
            $comment_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $comment_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $comment_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog('Smartling not processed content of field - @field_name', FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
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

  $field_names = smartling_get_supported_field_names_by_type_and_bundle('user', 'user');

  foreach ($field_names as $field_name) {
    switch (smartling_get_field_content_processing_mode($field_name)) {
      case 'simple-text-field':
        $user_current_translatable_content[$field_name] = $user->{$field_name}[$user->translations->original][0]['value'];
        break;

      case 'multiple-text-field':
        foreach ($user->{$field_name}[$user->translations->original] as $key => $item) {
          $user_current_translatable_content[$field_name][$key] = $item['value'];
        }
        break;

      case 'body-with-summary-field':
        $user_current_translatable_content[$field_name]['body'] = $user->{$field_name}[$user->translations->original][0]['value'];
        $user_current_translatable_content[$field_name]['summary'] = $user->{$field_name}[$user->translations->original][0]['summary'];
        break;

      case 'body-with-summary-multiple-field':
        foreach ($user->{$field_name}[$user->translations->original] as $key => $item) {
          $user_current_translatable_content[$field_name][$key]['body'] = $item['value'];
          $user_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
        }
        break;

      case 'simple-image-field':
        if (is_array($user->{$field_name}) && !empty($user->{$field_name}[$user->translations->original])) {
          $user_current_translatable_content[$field_name]['alt-img'] = $user->{$field_name}[$user->translations->original][0]['alt'];
          $user_current_translatable_content[$field_name]['title-img'] = $user->{$field_name}[$user->translations->original][0]['title'];
          $user_current_translatable_content[$field_name]['fid-img'] = $user->{$field_name}[$user->translations->original][0]['fid'];
        }
        break;

      case 'multiple-image-field':
        if (is_array($user->{$field_name}) && !empty($user->{$field_name}[$user->translations->original])) {
          foreach ($user->{$field_name}[$user->translations->original] as $key => $item) {
            $user_current_translatable_content[$field_name][$key]['alt-img'] = $item['alt'];
            $user_current_translatable_content[$field_name][$key]['title-img'] = $item['title'];
            $user_current_translatable_content[$field_name][$key]['fid-img'] = $item['fid'];
          }
        }
        break;

      default:
        smartling_watchdog('Smartling not processed content of field - @field_name', FALSE, array('@field_name' => $field_name), WATCHDOG_INFO);
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
  if ($field_name == 'taxonomy-term-title') {
    return 'taxonomy-term-title';
  }
  elseif ($field_name == 'taxonomy-term-description') {
    return 'taxonomy-term-description';
  }
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
function smartling_node_title_processing($entity_wrapper, $field_name, $xpath) {
  $node_title = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
  $string = (string) $node_title->nodeValue;
  $entity_wrapper->title = $string;
  return $entity_wrapper;
}

/**
 * Processing for term title ($term->name)
 */
function smartling_term_title_processing($entity_wrapper, $field_name, $xpath) {
  $term_title = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
  $string = (string) $term_title->nodeValue;
  $entity_wrapper->name = $string;
  return $entity_wrapper;
}

/**
 * Processing for term description ($term->description)
 */
function smartling_term_description_processing($entity_wrapper, $field_name, $xpath) {
  $term_description = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
  $string = (string) $term_description->nodeValue;
  $entity_wrapper->description = $string;
  return $entity_wrapper;
}

/**
 * Processing for single text field
 */
function smartling_simple_text_field_processing($entity_wrapper, $field_name, $xpath, $d_locale) {
  $simple_field = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
  $string = (string) $simple_field->nodeValue;
  $info = $entity_wrapper->language($d_locale)->{$field_name}->info();

  if (isset($info['property info'])) {
    $entity_wrapper->language($d_locale)->{$field_name}->value = $string;
  }
  else {
    $entity_wrapper->language($d_locale)->{$field_name}->set($string);
  }
  return $entity_wrapper;
}

/**
 * Processing for multiple text field
 */
function smartling_multiple_text_field_processing($entity_wrapper, $field_name, $xpath, $d_locale) {
  $simple_field_multiple = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
  $quantity = $simple_field_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $field = $xpath->query('//string[@id="' . $field_name . '-' . $i . '"][1]')->item(0);
    $string = (string) $field->nodeValue;
    $info = $entity_wrapper->language($d_locale)->{$field_name}[$i]->info();

    if (isset($info['property info'])) {
      $entity_wrapper->language($d_locale)->{$field_name}[$i]->value = $string;
    }
    else {
      $entity_wrapper->language($d_locale)->{$field_name}[$i]->set($string);
    }
  }
  return $entity_wrapper;
}

/**
 * Processing for single body/summary field
 */
function smartling_body_with_summary_field_processing($entity_wrapper, $field_name, $xpath, $d_locale) {
  $body_with_summary = $xpath->query('//string[@id="' . $field_name . '-body' . '"][1]')->item(0);

  $body = (string) $body_with_summary->nodeValue;
  $summary = $xpath->query('//string[@id="' . $field_name . '-summary' . '"][1]')->item(0);
  $summary = (string) $summary->nodeValue;
  $info = $entity_wrapper->language($d_locale)->{$field_name}->info();

  if (isset($info['property info'])) {
    $entity_wrapper->language($d_locale)->{$field_name}->value = $body;
    $entity_wrapper->language($d_locale)->{$field_name}->summary = $summary;
  }
  else {
    $entity_wrapper->language($d_locale)->{$field_name}->set(array('value' => $body, 'summary' => $summary));
  }
  return $entity_wrapper;
}

/**
 * Processing for multiple body/summary field
 */
function smartling_body_with_summary_multiple_field_processing($entity_wrapper, $field_name, $xpath, $d_locale) {
  $body_with_summary_multiple = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);
  $quantity = $body_with_summary_multiple->getAttribute('quantity');

  for ($i = 0; $i < $quantity; $i++) {
    $body_obj = $xpath->query('//string[@id="' . $field_name . '-body-' . $i . '"][1]')->item(0);
    $summary_obj = $xpath->query('//string[@id="' . $field_name . '-summary-' . $i . '"][1]')->item(0);
    $body_string = (string) $body_obj->nodeValue;
    $summary_string = (string) $summary_obj->nodeValue;
    $info = $entity_wrapper->language($d_locale)->{$field_name}[$i]->info();

    if (isset($info['property info'])) {
      $entity_wrapper->language($d_locale)->{$field_name}[$i]->value = $body_string;
      $entity_wrapper->language($d_locale)->{$field_name}[$i]->summary = $summary_string;
    }
    else {
      $entity_wrapper->language($d_locale)->{$field_name}[$i]->set(array('value' => $body_string, 'summary' => $summary_string));
    }
  }
  return $entity_wrapper;
}

/**
 * Processing for single image field
 */
function smartling_simple_image_field_processing($entity_wrapper, $field_name, $xpath, $d_locale) {
  $alt_img = $xpath->query('//string[@id="' . $field_name . '-alt-img' . '"][1]')->item(0);
  $fid = $alt_img->getAttribute('fid');
  $alt_img = (string) $alt_img->nodeValue;
  $title_img = $xpath->query('//string[@id="' . $field_name . '-title-img' . '"][1]')->item(0);
  $title_img = (string) $title_img->nodeValue;
  $file_img = file_load($fid);

  if ($file_img) {
    $entity_wrapper->language($d_locale)->{$field_name} = (array) $file_img + array('alt' => $alt_img, 'title' => $title_img);
  }
  return $entity_wrapper;
}

/**
 * Processing for multiple image field
 */
function smartling_multiple_image_field_processing($entity_wrapper, $field_name, $xpath, $d_locale) {
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
      $entity_wrapper->language($d_locale)->{$field_name}[$i] = (array) $file_img + array('alt' => $alt_string, 'title' => $title_string);
    }
  }
  return $entity_wrapper;
}

/**
 * Parsing the translated file and record the results in the field.
 *
 * @param type $nid
 * @param type $s_locale locale in Smartling format
 * @param bool $nodes_method
 * If TRUE - nodes method execute
 */
function smartling_update_translated_fields($rid, $s_locale, $nodes_method = FALSE, $bundle = NULL) {
  $entity_type = smartling_get_entity_type_by_bundle($bundle);

  $d_locale = smartling_convert_locale_drupal_to_smartling($s_locale, TRUE);
  smartling_watchdog('Smartling start update fields for entity type @entity_type, id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => $s_locale), WATCHDOG_INFO);

  $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'bundle' => $bundle, 'target_language' => $s_locale));
  $file_name = $entity_data->translated_file_name;
  $file_path = SMARTLING_DIRECTORY . '/' . $file_name;



  // Preproccess for entity type
  switch ($entity_type) {
    case 'node':
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
          $node->title = $original_node->title;
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
        $term = (object) array('vid' => $vocabulary->vid, 'name' => '');

        // Add language field or not depending on taxonomy mode.
        $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary->vid);
        switch ($vocabulary_mode) {
          case I18N_MODE_TRANSLATE:

            // If the term to be added will be a translation of a source term,
            // set the default value of the option list to the target language and
            // create a form element for storing the translation set of the source term.
            $source_term = taxonomy_term_load($rid);
            $target_language = i18n_language_object($d_locale);
            // Set context language to target language.
            i18n_language_context($target_language);

            $term->name = $source_term->name;
            $term->description = $source_term->description;
            $term->format = $source_term->format;
            $term->language = $target_language->language;
            $term->vocabulary_machine_name = $source_term->vocabulary_machine_name;
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

            $term_fields = field_info_instances('taxonomy_term', $source_term->vocabulary_machine_name);
            foreach ($term_fields as $field) {
              $term->{$field['field_name']}[LANGUAGE_NONE] = $source_term->{$field['field_name']}[LANGUAGE_NONE];
            }

            break;

          case I18N_MODE_LANGUAGE:
          case I18N_MODE_LOCALIZE:
          case I18N_MODE_NONE:
            smartling_watchdog('Translatable @entity_type with id - @rid FAIL. Vocabulary mode - @vocabulary_mode', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@vocabulary_mode' => $vocabulary_mode), WATCHDOG_INFO);
            return FALSE;
            break;
          default:
            smartling_watchdog('Translatable @entity_type with id - @rid FAIL', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid), WATCHDOG_INFO);
            return FALSE;
            break;
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

  // Get field names for entity
  switch ($entity_type) {
    case 'node':
      $fields = array();
      $smartling_translate_fields = variable_get('smartling_translate_fields');
      $fields = $smartling_translate_fields[$entity_data->bundle];
      break;

    case 'taxonomy_term':
      $fields = array();
      $fields[] = 'taxonomy-term-title';
      $fields[] = 'taxonomy-term-description';

      $fields = array_merge($fields, smartling_get_supported_field_names_by_type_and_bundle('taxonomy_term', $entity_data->bundle));
      break;

    case 'comment':
      $fields = smartling_get_supported_field_names_by_type_and_bundle('comment', $entity_data->bundle);
      break;

    case 'user':
      $fields = smartling_get_supported_field_names_by_type_and_bundle('user', $entity_data->bundle);
      break;

    default:
      $fields = array();
      break;
  }

  $xml = new DOMDocument();

  $xml->load($file_path);
  $xpath = new DomXpath($xml);

  $entity_wrapper = entity_metadata_wrapper($entity_type, $rid);
  if (!is_null($entity_wrapper->label())) {
    foreach ($fields as $field_name) {

      switch (smartling_get_update_field_processing_mode($field_name, $xpath)) {
        case 'taxonomy-term-title':
          $entity_wrapper = smartling_term_title_processing($entity_wrapper, $field_name, $xpath);
          break;

        case 'taxonomy-term-description':
          $entity_wrapper = smartling_term_description_processing($entity_wrapper, $field_name, $xpath);
          break;

        case 'node-title':
          $entity_wrapper = smartling_node_title_processing($entity_wrapper, $field_name, $xpath);
          break;

        case 'simple-text-field':
          $entity_wrapper = smartling_simple_text_field_processing($entity_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'multiple-text-field':
          $entity_wrapper = smartling_multiple_text_field_processing($entity_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'body-with-summary-field':
          $entity_wrapper = smartling_body_with_summary_field_processing($entity_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'body-with-summary-multiple-field':
          $entity_wrapper = smartling_body_with_summary_multiple_field_processing($entity_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'simple-image-field':
          $entity_wrapper = smartling_simple_image_field_processing($entity_wrapper, $field_name, $xpath, $d_locale);
          break;

        case 'multiple-image-field':
          $entity_wrapper = smartling_multiple_image_field_processing($entity_wrapper, $field_name, $xpath, $d_locale);
          break;

        default:
          smartling_watchdog('Smartling not processed field - @field_name', TRUE, array('@field_name' => $field_name), WATCHDOG_INFO);
          break;
      }
    }

    $entity_wrapper->save();

    $entity_data->download = 1;

    smartling_entity_data_save($entity_data);

    smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_UPDATE_FIELDS);

    $link_to_entity = smartling_get_link_to_entity($rid, $entity_type);
    smartling_watchdog('Smartling update all translated fields for @entity_type id - @rid. Locale: @locale', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid, '@locale' => $s_locale), WATCHDOG_INFO, $link_to_entity);

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
        $handler = entity_translation_get_handler($entity_type, $entity_load);
        $translations = $handler->getTranslations();

        // Initialize translations if they are empty.
        if (empty($translations->original)) {
          $handler->initTranslations();
          $handler->saveTranslations();
          $translations = $handler->getTranslations();
        }

        if (isset($translations->data[$d_locale])) {
          $entity_translation = array(
            'translate' => '0',
            'status' => $entity_load->status,
            'language' => $d_locale,
            'uid' => $entity_data->submitter,
            'changed' => $entity_data->submission_date,
          );
          $handler->setTranslation($entity_translation);
        }
        else {
          // Add the new translation.
          $entity_translation = array(
            'translate' => '0',
            'status' => $entity_load->status,
            'language' => $d_locale,
            'source' => $translations->original,
            'uid' => $entity_data->submitter,
            'created' => $entity_load->created,
            'changed' => $entity_data->submission_date,
          );
          $handler->setTranslation($entity_translation);
        }
        $handler->saveTranslations();
        break;

      default:
        break;
    }

    if (module_exists('rules') && ($entity_type == 'node')) {
      $node_event = smartling_node_load_default($rid);
      rules_invoke_event('smartling_translation_download_event', $node_event);
    }
  }
  else {
    smartling_watchdog('Translatable @entity_type with id - @rid not found in database', TRUE, array('@entity_type' => $entity_type, '@rid' => $rid), WATCHDOG_INFO);
  }
}

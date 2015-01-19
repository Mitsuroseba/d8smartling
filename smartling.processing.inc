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

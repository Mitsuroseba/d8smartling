<?php

/**
 * @file
 * Rules functions for Smartling module.
 */

/**
 * Implementation of hook_rules_event_info()
 */
function smartling_rules_event_info() {
  return array(
    'smartling_callback_call_event' => array(
      'label' => t('Smartling callback call'),
      'module' => 'smartling',
      'group' => 'smartling',
      'variables' => array(
        'locale' => array('type' => 'text', 'label' => t('Locale')),
        'file_uri' => array('type' => 'text', 'label' => t('File uri')),
      ),
    ),
    'smartling_translation_download_event' => array(
      'label' => t('Smartling translation download'),
      'module' => 'smartling',
      'group' => 'smartling',
      'variables' => array(
        'node' => array('type' => 'node', 'label' => t('Translated node')),
      ),
    ),
  );
}

/**
 * Implements hook_rules_action_info().
 */
function smartling_rules_action_info() {
  $actions = array(
    'smartling_get_translation_status_action' => array(
      'label' => t('Smartling get translation status'),
      'group' => t('Smartling actions'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Node'),
          'optional' => FALSE,
          'description' => t('There should be selected node object.'),
        ),
      ),
    ),
    'smartling_download_translation_action' => array(
      'label' => t('Smartling download translation'),
      'group' => t('Smartling actions'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Node'),
          'optional' => FALSE,
          'description' => t('There should be selected node object.'),
        ),
      ),
    ),
    'smartling_send_to_translate_action' => array(
      'label' => t('Smartling send to translate'),
      'group' => t('Smartling actions'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Node'),
          'optional' => FALSE,
          'description' => t('There should be selected node object.'),
        ),
        'translate_to_langs' => array(
          'type' => 'list<text>',
          'label' => t('Languages'),
          'optional' => FALSE,
          'options list' => 'smartling_language_list',
          'description' => t('Node will be translated into the selected language.'),
        ),
      ),
    ),
  );

  return $actions;
}

/**
 * Get translation status.
 * @param type $node
 */
function smartling_get_translation_status_action($node) {
  if (is_object($node)) {
    $result = smartling_entity_load_all_by_conditions(array('rid' => $node->nid));
    if ($result) {
      foreach ($result as $value) {
        if (!empty($value->rid) && !empty($value->target_language)) {
          $args = new stdClass;
          $args->rid = $value->rid;
          $args->s_locale = $value->target_language;
          smartling_watchdog('Smartling start check status action for node id - @nid (@s_locale).', TRUE, array('@nid' => $args->rid, '@s_locale' => $args->s_locale), WATCHDOG_INFO, l(t('Edit node'), 'node/' . $node->nid . '/edit'));
          smartling_queue_check_status_process($args);
          unset($args);
        }
      }
    }
  }
}

function smartling_download_translation_action($node) {
  if (is_object($node)) {
    $result = smartling_entity_load_all_by_conditions(array('rid' => $node->nid));
    if ($result) {
      foreach ($result as $entity_data) {
        smartling_watchdog('Smartling start download translation action for node id - @nid (@s_locale).', TRUE, array('@nid' => $entity_data->rid, '@s_locale' => $entity_data->target_language), WATCHDOG_INFO, l(t('Edit node'), 'node/' . $node->nid . '/edit'));
        smartling_queue_download_update_translated_item_process($entity_data);
      }
    }
  }
}

function smartling_send_to_translate_action($node, $translate_to_langs) {
  if (is_object($node) && !empty($translate_to_langs)) {
    global $user;

    foreach ($translate_to_langs as $language => $language_name) {
      $s_locale = smartling_convert_locale_drupal_to_smatrtling($language);
      if (smartling_supported_type($node->type) && smartling_translate_fields_is_set($node->type)) {
        if (smartling_nodes_method($node->type)) {
          $status = smartling_nodes_method_node_status($node);
          if ($status == SMARTLING_ORIGINAL_NODE || $status == SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE) {
            if ($node->tnid == '0') {
              $node->tnid = $node->nid;
              node_save($node);
            }
            if ($language != $node->language) {
              $obj = new stdClass;
              $obj->rid = $node->nid;
              $obj->type = $node->type;
              $obj->title = $node->title;
              $obj->original_language = smartling_convert_locale_drupal_to_smatrtling($node->language);
              $obj->s_locale = $s_locale;
              $obj->submitter = $user->uid;
              smartling_watchdog('Smartling start send to translate action for node id - @nid (@s_locale).', TRUE, array('@nid' => $obj->rid, '@s_locale' => $obj->s_locale), WATCHDOG_INFO, l(t('Edit node'), 'node/' . $node->nid . '/edit'));
              smartling_queue_send_to_translate_process($obj);
              unset($obj);
            }
          }
        }
        elseif (smartling_fields_method($node->type)) {
          if ($language != $node->language) {
            $obj = new stdClass;
            $obj->rid = $node->nid;
            $obj->type = $node->type;
            $obj->title = $node->title;
            $obj->original_language = smartling_convert_locale_drupal_to_smatrtling($node->language);
            $obj->s_locale = $s_locale;
            $obj->submitter = $user->uid;
            smartling_watchdog('Smartling start send to translate action for node id - @nid (@s_locale).', TRUE, array('@nid' => $obj->rid, '@s_locale' => $obj->s_locale), WATCHDOG_INFO, l(t('Edit node'), 'node/' . $node->nid . '/edit'));
            smartling_queue_send_to_translate_process($obj);
            unset($obj);
          }
        }
      }
    }
  }
}

function smartling_language_list() {
  $languages_list = array();
  $language_default = language_default();
  $languages = language_list('language');
  unset($languages[$language_default->language]);

  foreach ($languages as $langcode => $language) {
    if ($language->enabled != '0') {
      $languages_list[$langcode] = $language->name;
    }
  }
  return $languages_list;
}

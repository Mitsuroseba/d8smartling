<?php

/**
 * @file
 * Functions for node entity.
 */

/**
 * Implements hook_form_BASE_FORM_ID_alter().
 */
function smartling_form_node_form_alter(&$form, $form_state) {
  if (!smartling_supported_type($form['#node']->type)) {
    return;
  }

  if (smartling_nodes_method($form['#node']->type) && !isset($form['#node']->tnid)) {
    return;
  }

  if (smartling_fields_method($form['#node']->type) && !isset($form['#node']->tnid)) {
    return;
  }

  if (smartling_translate_fields_is_set($form['#node']->type)) {
    $form = array_merge($form, smartling_get_node_settings_form($form, $form_state));
  }
}

/**
 * Implements hook_node_presave().
 */
function smartling_node_presave($node) {
  // Make sure the title isn't overwritten with the translation when using the title module.
  if (module_exists('title') && array_key_exists('title_field', $node)) {
    if (isset($node->title_field[$node->language][0]['value'])) {
      $node->title = $node->title_field[$node->language][0]['value'];
    }
  }
}

/**
 * Implements hook_node_delete().
 */
function smartling_node_delete($node) {
  $entity_data = smartling_entity_load_by_conditions(array('rid' => $node->nid, 'bundle' => $node->type));
  if ($entity_data !== FALSE) {
    db_delete('smartling_entity_data')
        ->condition('rid', $entity_data->rid)
        ->execute();
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Enable Smartling translation on a node type
 */
function smartling_form_node_type_form_alter(&$form, &$form_state) {
  $details = t('It will be possible to use Smartling service for automatic content translation.');

  $form['workflow']['language_content_type']['#description'] .= ' ' . $details;
  $form['#submit'][] = 'smartling_multilanguage_support_node_type_submit';
  $form['#validate'][] = 'smartling_multilanguage_support_node_type_validate';
}

function smartling_multilanguage_support_node_type_validate(&$form, &$form_state) {
  if ($form_state['values']['language_content_type'] == SMARTLING_FIELDS_METHOD_KEY || $form_state['values']['language_content_type'] == SMARTLING_NODES_METHOD_KEY) {
    if (!smartling_is_configured()) {
      drupal_set_message(t('Please follow the page <a href="/admin/config/regional/smartling">"Smartling settings"</a> to setup Smartling configuration.'), 'warning');
      form_set_error('workflow');
    }
  }
}

function smartling_multilanguage_support_node_type_submit(&$form, &$form_state) {
  // Delete content type in smartling support types
  $smartling_translate_fields = variable_get('smartling_translate_fields', array());
  $type = $form_state['values']['type'];
  if ($form_state['values']['language_content_type'] != SMARTLING_FIELDS_METHOD_KEY || $form_state['values']['language_content_type'] != SMARTLING_NODES_METHOD_KEY) {
    if (isset($smartling_translate_fields[$type])) {
      unset($smartling_translate_fields[$type]);
      variable_set('smartling_translate_fields', $smartling_translate_fields);
    }
  }
}

function smartling_get_node_settings_form($form, &$form_state) {
  if (smartling_translate_fields_is_set($form['#node']->type)) {
    // Vertical Tab.
    $form['smartling'] = array(
      '#title' => t('Smartling management'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#group' => 'additional_settings',
      '#attributes' => array('id' => array('smartling_fieldset')),
      '#attached' => array(
        //'js' => array(
        //  drupal_get_path('module', 'smartling') . '/js/smartling_node_settings.js' => array(
        //    'type' => 'file',
        //  ),
        //),
        'css' => array(
          drupal_get_path('module', 'smartling') . '/css/smartling_entity_settings.css' => array(
            'type' => 'file',
          ),
        ),
      ),
      '#modal' => TRUE,
    );

    $form['smartling']['content'] = array(
      '#type' => 'container',
    );

    // This is node for fields method translate or original for nodes method
    if (($form['#node']->tnid == '0') || ($form['#node']->tnid == $form['#node']->nid)) {
      $languages = smartling_language_list();
    }
    elseif ($form['#node']->tnid != $form['#node']->nid) {
      // This is node for nodes method translate | not original
      $languages = smartling_language_list();
      $node_original = node_load($form['#node']->tnid);
      unset($languages[$node_original->language]);
    }

    $options = array();

    if (!is_null($form['nid']['#value'])) {
      $check = array();

      if (($form['#node']->tnid != '0') && ($form['#node']->tnid != $form['#node']->nid)) {
        // For not original node in nodes translate method
        $translations = translation_node_get_translations($form['#node']->tnid);
        $original_nid = FALSE;
        // Get original
        foreach ($translations as $langcode => $value) {
          if ($translations[$langcode]->nid == $form['#node']->tnid) {
            $original_nid = $translations[$langcode]->nid;
            break;
          }
        }

        foreach ($languages as $langcode => $language) {
          if ($language->enabled != '0') {
            $s_locale = smartling_convert_locale_drupal_to_smartling($langcode);

            $entity_data = smartling_entity_load_by_conditions(array('rid' => $original_nid, 'bundle' => $form['#bundle'], 'target_language' => $s_locale));

            if ($entity_data !== FALSE) {
              switch ($entity_data->status) {
                case SMARTLING_STATUS_IN_QUEUE:
                  $options[$langcode] = '<span title="' . t('Node is waiting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_IN_TRANSLATE:
                  $options[$langcode] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_TRANSLATED:
                  $options[$langcode] = '<span title="' . t('Node is translated') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_CHANGE:
                  $options[$langcode] = '<span title="' . t('Node was changed after submitting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                  break;
              }
            }
            else {
              $options[$langcode] = $language->name;
            }

            $check[] = ($entity_data) ? $langcode : FALSE;
          }
        }
      }
      elseif (($form['#node']->tnid != '0') && ($form['#node']->tnid == $form['#node']->nid)) {
        // For original node in nodes translate method
        $translations = translation_node_get_translations($form['#node']->tnid);
        $original_nid = FALSE;
        // Get original
        foreach ($translations as $langcode => $value) {
          if ($translations[$langcode]->nid == $form['#node']->tnid) {
            $original_nid = $translations[$langcode]->nid;
            break;
          }
        }

        foreach ($languages as $langcode => $language) {

          if ($form['language']['#default_value'] != $langcode && $language->enabled != '0') {
            $s_locale = smartling_convert_locale_drupal_to_smartling($langcode);

            $entity_data = smartling_entity_load_by_conditions(array('rid' => $original_nid, 'bundle' => $form['#bundle'], 'target_language' => $s_locale));

            if ($entity_data !== FALSE) {
              switch ($entity_data->status) {
                case SMARTLING_STATUS_IN_QUEUE:
                  $options[$langcode] = '<span title="' . t('Node is waiting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_IN_TRANSLATE:
                  $options[$langcode] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_TRANSLATED:
                  $options[$langcode] = '<span title="' . t('Node is translated') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_CHANGE:
                  $options[$langcode] = '<span title="' . t('Node was changed after submitting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                  break;
              }
            }
            else {
              $options[$langcode] = $language->name;
            }

            $check[] = ($entity_data) ? $langcode : FALSE;
          }
        }
      }
      else {
        // For fieds method
        foreach ($languages as $langcode => $language) {
          if ($form['language']['#default_value'] != $langcode && $language->enabled != '0') {
            $s_locale = smartling_convert_locale_drupal_to_smartling($langcode);
            $entity_data = smartling_entity_load_by_conditions(array('rid' => $form['nid']['#value'], 'bundle' => $form['#bundle'], 'target_language' => $s_locale));
            if ($entity_data !== FALSE) {
              switch ($entity_data->status) {
                case SMARTLING_STATUS_IN_QUEUE:
                  $options[$langcode] = '<span title="' . t('Node is waiting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_IN_TRANSLATE:
                  $options[$langcode] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_TRANSLATED:
                  $options[$langcode] = '<span title="' . t('Node is translated') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_CHANGE:
                  $options[$langcode] = '<span title="' . t('Node was changed after submitting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                  break;
              }
            }
            else {
              $options[$langcode] = $language->name;
            }
            $check[] = ($entity_data) ? $langcode : FALSE;
          }
        }
      }

      $form['smartling']['content']['target'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Target Locales',
        '#options' => $options,
        '#default_value' => $check,
      );
    }
    else {
      foreach ($languages as $langcode => $language) {
        $options[$langcode] = $language->name;
      }

      $form['smartling']['content']['target'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Target Locales',
        '#options' => $options,
      );
    }

    $form['smartling']['submit_to_translate'] = array(
      '#type' => 'submit',
      '#value' => 'Send to Smartling',
      '#submit' => array('smartling_get_node_settings_form_submit'),
      '#states' => array(
        'invisible' => array(
          // Hide the button if term is language neutral.
          'select[name=language]' => array('value' => LANGUAGE_NONE),
        ),
      ),
    );

    $form['smartling']['submit_to_download'] = array(
      '#type' => 'submit',
      '#value' => 'Download Translation',
      '#submit' => array('smartling_download_translate_form_submit'),
      '#states' => array(
        'invisible' => array(
          // Hide the button if term is language neutral.
          'select[name=language]' => array('value' => LANGUAGE_NONE),
        ),
      ),
    );

    return $form;
  }
}

/**
 * smartling_get_node_settings_form_submit
 */
function smartling_get_node_settings_form_submit($form, &$form_state) {
  $node = node_form_submit_build_node($form, $form_state);
  $nid = $node->nid;

  if (count(array_filter($form_state['values']['target'])) !== 0) {

    global $user;

    $smartling_queue = DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();
    $langs = array();

    if (smartling_nodes_method($node->type)) {
      $original_nid = $node->nid;
      $original_title = $node->title;
      // For nodes method
      $status = smartling_nodes_method_node_status($node);
      if ($status == SMARTLING_ORIGINAL_NODE || $status == SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE) {
        $node->tnid = $node->nid;
        $s_locale_origin = smartling_convert_locale_drupal_to_smartling($node->language);
      }

      if ($status == SMARTLING_TRANSLATE_NODE) {
        $translations = translation_node_get_translations($node->tnid);
        // Get original language
        foreach ($translations as $langcode => $value) {
          if ($translations[$langcode]->nid == $node->tnid) {
            $s_locale_origin = smartling_convert_locale_drupal_to_smartling($langcode);
            $original_nid = $translations[$langcode]->nid;
            $original_title = $translations[$langcode]->title;
            break;
          }
          else {
            $s_locale_origin = 'error';
          }
        }
      }

      foreach ($form_state['values']['target'] as $d_locale) {
        if ($d_locale !== 0) {
          $s_locale = smartling_convert_locale_drupal_to_smartling($d_locale);

          if (smartling_supported_type($form_state['values']['type'])) {

            $entity_data = smartling_entity_load_by_conditions(array('rid' => $original_nid, 'bundle' => $node->type, 'target_language' => $s_locale));

            if ($entity_data == FALSE) {
              $entity_data = smartling_entity_data_create();
              $entity_data->rid = $original_nid;
              $entity_data->bundle = $node->type;
              $entity_data->original_language = $s_locale_origin;
              $entity_data->target_language = $s_locale;
              $entity_data->progress = 0;
            }

            $entity_data->translated_file_name = FALSE;
            $entity_data->submitter = $user->uid;
            $entity_data->submission_date = time();
            smartling_entity_data_save($entity_data);

            smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
            $langs[] = $s_locale;
          }
        }
      }
      $obj = new stdClass;
      $obj->rid = $original_nid;
      $obj->bundle = $node->type;
      $obj->title = $original_title;
      $obj->original_language = $s_locale_origin;
      $obj->s_locales = $langs;
      $obj->submitter = $user->uid;
      // Create queue item
      $smartling_queue->createItem($obj);
      $title = $original_title;
      unset($obj);
      smartling_watchdog('Add smartling queue task for node id - @nid, locale - @locale', TRUE, array('@nid' => $original_nid, '@locale' => implode('; ', $langs)), WATCHDOG_INFO, l(t('Edit node'), 'node/' . $original_nid . '/edit'));
    }
    else {
      // For fields method

      foreach ($form_state['values']['target'] as $d_locale) {
        if ($d_locale !== 0 && ($d_locale !== $node->language)) {
          $s_locale = smartling_convert_locale_drupal_to_smartling($d_locale);
          if (smartling_supported_type($form_state['values']['type'])) {
            $s_locale_origin = smartling_convert_locale_drupal_to_smartling($node->language);

            $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'bundle' => $node->type, 'target_language' => $s_locale));

            if ($entity_data == FALSE) {
              $entity_data = smartling_entity_data_create();
              $entity_data->rid = $nid;
              $entity_data->bundle = $form_state['values']['type'];
              $entity_data->original_language = $s_locale_origin;
              $entity_data->target_language = $s_locale;
              $entity_data->progress = 0;
            }

            $entity_data->translated_file_name = FALSE;
            $entity_data->submitter = $user->uid;
            $entity_data->submission_date = time();
            smartling_entity_data_save($entity_data);

            smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
            $langs[] = $s_locale;
          }
        }
      }
      $obj = new stdClass;
      $obj->rid = $nid;
      $obj->bundle = $form_state['values']['type'];
      $obj->title = $form_state['values']['title'];
      $obj->original_language = $s_locale_origin;
      $obj->s_locales = $langs;
      $obj->submitter = $user->uid;
      // Create queue item
      $smartling_queue->createItem($obj);
      $title = $form_state['values']['title'];
      unset($obj);
      smartling_watchdog('Add smartling queue task for node id - @nid, locale - @locale', TRUE, array('@nid' => $nid, '@locale' => implode('; ', $langs)), WATCHDOG_INFO, l(t('Edit node'), 'node/' . $nid . '/edit'));
    }

    if (isset($_GET['destination'])) {
      unset($_GET['destination']);
    }

    $langs = implode(', ', $langs);
    drupal_set_message(t('The node "@title" has been sent to Smartling for translation to "@langs".', array('@title' => $title, '@langs' => $langs)));
  }
  node_save($node);
}
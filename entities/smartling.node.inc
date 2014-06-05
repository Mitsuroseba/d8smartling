<?php

/**
 * @file
 * Functions for node entity.
 */

/**
 * Implements hook_form_BASE_FORM_ID_alter().
 */
function smartling_form_node_form_alter(&$form, $form_state) {
  if (!smartling_supported_type($form['#entity_type'], $form['#node']->type)) {
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
  if (module_exists('title') && property_exists($node, 'title_field')) {
    $titles = field_get_items('node', $node, 'title_field');
    if (isset($titles[0]['value'])) {
      $node->title = $titles[0]['value'];
    }
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
      drupal_set_message(t('Please follow the page <a href="@url">"Smartling settings"</a> to setup Smartling configuration.', array('@url' => url('admin/config/regional/smartling'))), 'warning');
      form_set_error('workflow');
    }
  }
}

function smartling_multilanguage_support_node_type_submit(&$form, &$form_state) {
  // Delete content type in smartling support types
  $bundle = $form_state['values']['type'];
  if ($form_state['values']['language_content_type'] != SMARTLING_FIELDS_METHOD_KEY || $form_state['values']['language_content_type'] != SMARTLING_NODES_METHOD_KEY) {
    smartling_settings_get_handler()->deleteSingleBundleFromSettings('node', $bundle);
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

        foreach ($languages as $d_locale => $language) {
          if ($language->enabled != '0') {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $original_nid,
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale
            ));
            $language_name = check_plain($language->name);

            if ($entity_data !== FALSE) {
              switch ($entity_data->status) {
                case SMARTLING_STATUS_IN_QUEUE:
                  $options[$d_locale] = '<span title="' . t('Node is waiting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_IN_TRANSLATE:
                  $options[$d_locale] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_TRANSLATED:
                  $options[$d_locale] = '<span title="' . t('Node is translated') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_CHANGE:
                  $options[$d_locale] = '<span title="' . t('Node was changed after submitting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                  break;
              }
            }
            else {
              $options[$d_locale] = $language_name;
            }

            $check[] = ($entity_data) ? $d_locale : FALSE;
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

        foreach ($languages as $d_locale => $language) {

          if ($form['language']['#default_value'] != $d_locale && $language->enabled != '0') {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $original_nid,
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale
            ));
            $language_name = check_plain($language->name);

            if ($entity_data !== FALSE) {
              switch ($entity_data->status) {
                case SMARTLING_STATUS_IN_QUEUE:
                  $options[$d_locale] = '<span title="' . t('Node is waiting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_IN_TRANSLATE:
                  $options[$d_locale] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_TRANSLATED:
                  $options[$d_locale] = '<span title="' . t('Node is translated') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_CHANGE:
                  $options[$d_locale] = '<span title="' . t('Node was changed after submitting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                  break;
              }
            }
            else {
              $options[$d_locale] = $language_name;
            }

            $check[] = ($entity_data) ? $d_locale : FALSE;
          }
        }
      }
      else {
        // For fieds method
        foreach ($languages as $d_locale => $language) {
          if ($form['language']['#default_value'] != $d_locale && $language->enabled != '0') {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $form['nid']['#value'],
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale
            ));
            $language_name = check_plain($language->name);

            if ($entity_data !== FALSE) {
              switch ($entity_data->status) {
                case SMARTLING_STATUS_IN_QUEUE:
                  $options[$d_locale] = '<span title="' . t('Node is waiting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_IN_TRANSLATE:
                  $options[$d_locale] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_TRANSLATED:
                  $options[$d_locale] = '<span title="' . t('Node is translated') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                  break;
                case SMARTLING_STATUS_CHANGE:
                  $options[$d_locale] = '<span title="' . t('Node was changed after submitting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                  break;
              }
            }
            else {
              $options[$d_locale] = $language_name;
            }
            $check[] = ($entity_data) ? $d_locale : FALSE;
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
        $options[$langcode] = check_plain($language->name);
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
  $log = new SmartlingLog();
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
        $d_locale_origin = $node->language;
      }

      if ($status == SMARTLING_TRANSLATE_NODE) {
        $translations = translation_node_get_translations($node->tnid);
        // Get original language
        foreach ($translations as $langcode => $value) {
          if ($translations[$langcode]->nid == $node->tnid) {
            $d_locale_origin = $langcode;
            $original_nid = $translations[$langcode]->nid;
            $original_title = $translations[$langcode]->title;
            break;
          }
          else {
            $d_locale_origin = 'error';
          }
        }
      }

      foreach ($form_state['values']['target'] as $d_locale) {
        if ($d_locale !== 0) {

          if (smartling_supported_type($form['#entity_type'], $form_state['values']['type'])) {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $original_nid,
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale
            ));

            if ($entity_data == FALSE) {
              $entity_data = smartling_entity_data_create();
              $entity_data->rid = $original_nid;
              $entity_data->entity_type = $form['#entity_type'];
              $entity_data->bundle = $node->type;
              $entity_data->original_language = $d_locale_origin;
              $entity_data->target_language = $d_locale;
              $entity_data->progress = 0;
            }

            $entity_data->translated_file_name = FALSE;
            $entity_data->submitter = $user->uid;
            $entity_data->submission_date = time();
            smartling_entity_data_save($entity_data);

            smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
            $langs[] = $d_locale;
          }
        }
      }
      $obj = new stdClass;
      $obj->rid = $original_nid;
      $obj->entity_type = $form['#entity_type'];
      $obj->bundle = $node->type;
      $obj->title = $original_title;
      $obj->original_language = $d_locale_origin;
      $obj->d_locales = $langs;
      $obj->submitter = $user->uid;
      // Create queue item
      $smartling_queue->createItem($obj);
      $title = $original_title;
      unset($obj);
      $log->setMessage('Add smartling queue task for node id - @nid, locale - @locale')
        ->setVariables(array('@nid' => $original_nid, '@locale' => implode('; ', $langs)))
        ->setLink(l(t('Edit node'), 'node/' . $original_nid . '/edit'))
        ->execute();
    }
    else {
      // For fields method

      foreach ($form_state['values']['target'] as $d_locale) {
        if ($d_locale !== 0 && ($d_locale !== $node->language)) {

          if (smartling_supported_type($form['#entity_type'], $form_state['values']['type'])) {
            $d_locale_origin = $node->language;

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $nid,
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale
            ));

            if ($entity_data == FALSE) {
              $entity_data = smartling_entity_data_create();
              $entity_data->rid = $nid;
              $entity_data->entity_type = $form['#entity_type'];
              $entity_data->bundle = $form_state['values']['type'];
              $entity_data->original_language = $d_locale_origin;
              $entity_data->target_language = $d_locale;
              $entity_data->progress = 0;
            }

            $entity_data->translated_file_name = FALSE;
            $entity_data->submitter = $user->uid;
            $entity_data->submission_date = time();
            smartling_entity_data_save($entity_data);

            smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
            $langs[] = $d_locale;
          }
        }
      }
      $obj = new stdClass;
      $obj->rid = $nid;
      $obj->entity_type = $form['#entity_type'];
      $obj->bundle = $form_state['values']['type'];
      $obj->title = $form_state['values']['title'];
      $obj->original_language = $d_locale_origin;
      $obj->d_locales = $langs;
      $obj->submitter = $user->uid;
      // Create queue item
      $smartling_queue->createItem($obj);
      $title = $form_state['values']['title'];
      unset($obj);
      $log->setMessage('Add smartling queue task for node id - @nid, locale - @locale')
        ->setVariables(array('@nid' => $nid, '@locale' => implode('; ', $langs)))
        ->setLink(l(t('Edit node'), 'node/' . $nid . '/edit'))
        ->execute();
    }

    if (isset($_GET['destination'])) {
      unset($_GET['destination']);
    }

    $langs = implode(', ', $langs);
    drupal_set_message(t('The node "@title" has been sent to Smartling for translation to "@langs".', array(
      '@title' => $title,
      '@langs' => $langs
    )));
  }
  // For not change node status to red when send node and change content
  $node->send_to_smartling = TRUE;
  node_save($node);
}

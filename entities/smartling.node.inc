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

  if (smartling_translate_fields_is_set($form['#node']->type, 'node')) {
    $form = array_merge($form, smartling_get_node_settings_form($form, $form_state));
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

/**
 * Validate smartling multilingual support form for node types.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_multilanguage_support_node_type_validate(array &$form, array &$form_state) {
  if ($form_state['values']['language_content_type'] == SMARTLING_FIELDS_METHOD_KEY || $form_state['values']['language_content_type'] == SMARTLING_NODES_METHOD_KEY) {
    if (!smartling_is_configured()) {
      drupal_set_message(t('Please follow the page <a href="@url">"Smartling settings"</a> to setup Smartling configuration.', array('@url' => url('admin/config/regional/smartling'))), 'warning');
      form_set_error('workflow');
    }
  }
}

/**
 * Smartling multilingual support form for node types - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_multilanguage_support_node_type_submit(array &$form, array &$form_state) {
  // Delete content type in smartling support types.
  $bundle = $form_state['values']['type'];
  if ($form_state['values']['language_content_type'] != SMARTLING_FIELDS_METHOD_KEY || $form_state['values']['language_content_type'] != SMARTLING_NODES_METHOD_KEY) {
    smartling_settings_get_handler()->deleteSingleBundleFromSettings('node', $bundle);
  }
}

/**
 * Node smartling form.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for node smartling form.
 */
function smartling_get_node_settings_form(array $form, array &$form_state) {
  if (smartling_translate_fields_is_set($form['#node']->type, 'node')) {
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

    // This is node for fields method translate or original for nodes method.
    if (($form['#node']->tnid == '0') || ($form['#node']->tnid == $form['#node']->nid)) {
      $languages = smartling_language_list();
    }
    elseif ($form['#node']->tnid != $form['#node']->nid) {
      // This is node for nodes method translate | not original.
      $languages = smartling_language_list();
      $node_original = node_load($form['#node']->tnid);
      unset($languages[$node_original->language]);
    }

    $options = array();

    if (!is_null($form['nid']['#value'])) {
      $check = array();

      if (($form['#node']->tnid != '0') && ($form['#node']->tnid != $form['#node']->nid)) {
        // For not original node in nodes translate method.
        $translations = translation_node_get_translations($form['#node']->tnid);
        $original_nid = FALSE;
        // Get original.
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
              'target_language' => $d_locale,
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

                case SMARTLING_STATUS_FAILED:
                  $options[$d_locale] = '<span title="' . t('Request to service failed') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-failed">' . $entity_data->progress . '%</span></span>';
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
        // For original node in nodes translate method.
        $translations = translation_node_get_translations($form['#node']->tnid);
        $original_nid = FALSE;
        // Get original.
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
              'target_language' => $d_locale,
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

                case SMARTLING_STATUS_FAILED:
                  $options[$d_locale] = '<span title="' . t('Request to service failed') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-failed">' . $entity_data->progress . '%</span></span>';
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
        // For fieds method.
        foreach ($languages as $d_locale => $language) {
          if ($form['language']['#default_value'] != $d_locale && $language->enabled != '0') {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $form['nid']['#value'],
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale,
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

                case SMARTLING_STATUS_FAILED:
                  $options[$d_locale] = '<span title="' . t('Request to service failed') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-failed">' . $entity_data->progress . '%</span></span>';
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
 * Node smartling form - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_get_node_settings_form_submit(array $form, array &$form_state) {
  $log = smartling_log_get_handler();
  $node = node_form_submit_build_node($form, $form_state);

  $smartling_queue = DrupalQueue::get('smartling_upload');
  $smartling_queue->createQueue();

  /* @var $entity_wrapper \EntityDrupalWrapper */
  $entity_type = $form['#entity_type'];
  $entity_wrapper = entity_metadata_wrapper($entity_type, $node);

  foreach ($form_state['values']['target'] as $target_language) {
    if ($target_language) {
      $conditions = array(
        'rid' => $entity_wrapper->getIdentifier(),
        'entity_type' => $entity_type, //$entity_wrapper->getBundle(),
        'target_language' => $target_language,
      );
      $smartling_entity = smartling_entity_load_by_conditions($conditions);

      if (!$smartling_entity) {
        $smartling_entity = smartling_create_from_entity($node, $entity_wrapper->type(), $entity_wrapper->language->value(), $target_language);
      }

      $processor = smartling_get_entity_processor($smartling_entity);
      $processor->sendToUploadQueue();

      $smartling_queue->createItem($smartling_entity->eid);

      $log->setMessage('Add smartling queue task for node id - @nid, locale - @locale')
        ->setVariables(array(
          '@nid' => $entity_wrapper->getIdentifier(),
          '@locale' => $target_language,
        ))
        ->setLink(l(t('Edit node'), 'node/' . $entity_wrapper->getIdentifier() . '/edit'))
        ->execute();

      drupal_set_message(t('The node "@title" has been sent to Smartling for translation to "@lang".', array(
        '@title' => $entity_wrapper->title->value(),
        '@lang' => $target_language,
      )));
    }
  }

//  $entity_data = smartling_entity_load_by_conditions(array(
//    'rid' => $original_nid,
//    'entity_type' => $form['#entity_type'],
//    'target_language' => $d_locale,
//  ));
//
//  $nid = $node->nid;
//  if (count(array_filter($form_state['values']['target'])) !== 0) {
//
//    global $user;
//
//    $smartling_queue = DrupalQueue::get('smartling_upload');
//    $smartling_queue->createQueue();
//    $langs = array();
//
//    if (smartling_nodes_method($node->type)) {
//      $original_nid = $node->nid;
//      $original_title = $node->title;
//      // For nodes method.
//      $status = smartling_nodes_method_node_status($node);
//      if ($status == SMARTLING_ORIGINAL_NODE || $status == SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE) {
//        $node->tnid = $node->nid;
//        $d_locale_origin = $node->language;
//      }
//
//      if ($status == SMARTLING_TRANSLATE_NODE) {
//        $translations = translation_node_get_translations($node->tnid);
//        // Get original language.
//        foreach ($translations as $langcode => $value) {
//          if ($translations[$langcode]->nid == $node->tnid) {
//            $d_locale_origin = $langcode;
//            $original_nid = $translations[$langcode]->nid;
//            $original_title = $translations[$langcode]->title;
//            break;
//          }
//          else {
//            $d_locale_origin = 'error';
//          }
//        }
//      }
//
//      foreach ($form_state['values']['target'] as $d_locale) {
//        if ($d_locale !== 0) {
//
//          if (smartling_supported_type($form['#entity_type'], $form_state['values']['type'])) {
//
//            $entity_data = smartling_entity_load_by_conditions(array(
//              'rid' => $original_nid,
//              'entity_type' => $form['#entity_type'],
//              'target_language' => $d_locale,
//            ));
//
//            if ($entity_data == FALSE) {
//              // @todo: use smartling_create_from_entity().
//              $entity_data = smartling_entity_data_create();
//              $entity_data->rid = $original_nid;
//              $entity_data->entity_type = $form['#entity_type'];
//              $entity_data->bundle = $node->type;
//              $entity_data->original_language = $d_locale_origin;
//              $entity_data->target_language = $d_locale;
//              $entity_data->progress = 0;
//            }
//
//            $entity_data->translated_file_name = FALSE;
//            $entity_data->submitter = $user->uid;
//            $entity_data->submission_date = time();
//            smartling_entity_data_save($entity_data);
//
//            smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
//            $langs[] = $d_locale;
//          }
//        }
//      }
//
//      // Create queue item.
//      $smartling_queue->createItem($entity_data->eid);
//      $title = $original_title;
//      $log->setMessage('Add smartling queue task for node id - @nid, locale - @locale')
//        ->setVariables(array(
//          '@nid' => $original_nid,
//          '@locale' => implode('; ', $langs),
//        ))
//        ->setLink(l(t('Edit node'), 'node/' . $original_nid . '/edit'))
//        ->execute();
//    }
//    else {
//      // For fields method.
//      foreach ($form_state['values']['target'] as $d_locale) {
//        if ($d_locale !== 0 && ($d_locale !== $node->language)) {
//
//          if (smartling_supported_type($form['#entity_type'], $form_state['values']['type'])) {
//            $d_locale_origin = $node->language;
//
//            $entity_data = smartling_entity_load_by_conditions(array(
//              'rid' => $nid,
//              'entity_type' => $form['#entity_type'],
//              'target_language' => $d_locale,
//            ));
//
//            if ($entity_data == FALSE) {
//              $entity_data = smartling_entity_data_create();
//              $entity_data->rid = $nid;
//              $entity_data->entity_type = $form['#entity_type'];
//              $entity_data->bundle = $form_state['values']['type'];
//              $entity_data->original_language = $d_locale_origin;
//              $entity_data->target_language = $d_locale;
//              $entity_data->progress = 0;
//            }
//
//            $entity_data->translated_file_name = FALSE;
//            $entity_data->submitter = $user->uid;
//            $entity_data->submission_date = time();
//            smartling_entity_data_save($entity_data);
//
//            smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
//            $langs[] = $d_locale;
//          }
//        }
//      }
//
//      // Create queue item.
//      $smartling_queue->createItem($entity_data->eid);
//      $title = $form_state['values']['title'];
//      unset($obj);
//      $log->setMessage('Add smartling queue task for node id - @nid, locale - @locale')
//        ->setVariables(array(
//          '@nid' => $nid,
//          '@locale' => implode('; ', $langs),
//        ))
//        ->setLink(l(t('Edit node'), 'node/' . $nid . '/edit'))
//        ->execute();
//    }
//
//
//
//    $langs = implode(', ', $langs);
//    drupal_set_message(t('The node "@title" has been sent to Smartling for translation to "@langs".', array(
//      '@title' => $title,
//      '@langs' => $langs,
//    )));
//  }

  unset($_GET['destination']);
  // For not change node status to red when send node and change content.
  $node->send_to_smartling = TRUE;
  node_save($node);
}

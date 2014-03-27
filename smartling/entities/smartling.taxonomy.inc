<?php

/**
 * @file
 * Functions for taxonomy_term entity.
 */

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Disabled method on taxonomy vocabulary form
 */
function smartling_form_taxonomy_form_vocabulary_alter(&$form, &$form_state) {
  if (module_exists('i18n') && module_exists('i18n_taxonomy') && module_exists('translation')) {
    if (!isset($form_state['confirm_delete'])) {
      $form['i18n_translation']['i18n_mode']['#disabled'] = TRUE;
      $form['i18n_translation']['i18n_mode']['#default_value'] = I18N_MODE_TRANSLATE;
      $form['i18n_translation']['i18n_mode']['#description'] .= '<br>' . t('Should be disabled and set Translate when using Smartling.');
    }
  }
}

/**
 * Implements hook_form_BASE_FORM_ID_alter().
 */
function smartling_form_taxonomy_form_term_alter(&$form, $form_state) {
  $term = $form_state['term'];
  if (!empty($term->tid)) {
    if (smartling_supported_type($form['#vocabulary']->machine_name)) {
      $form = array_merge($form, smartling_get_term_settings_form($form, $form_state));
    }
    else {
      $link = '<a href="/admin/structure/taxonomy/' . $form['#vocabulary']->machine_name . '/edit">"edit vocabulary"</a>';
      drupal_set_message(t('Please follow the page !link to set Translation mode for Smartling.', array('!link' => $link)), 'warning');
    }
  }
  elseif (empty($term->tid) && !isset($_GET['translation']) && !isset($_GET['target'])) {
    $langcode = language_default()->language;
    $form['language']['#default_value'] = $langcode;
    unset($form['language']['#options']['und']);
  }
}

function smartling_get_term_settings_form($form, &$form_state) {
  if (!isset($form_state['confirm_delete']) || $form_state['confirm_delete'] !== TRUE) {
    $form['smartling'] = array(
      '#title' => t('Smartling management'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 100,
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

    $languages = smartling_language_list();

    $options = array();

    $check = array();
    if (!is_null($form['tid']['#value'])) {
      $language_default = language_default()->language;

      if ($form['#term']['language'] == $language_default) {
        $tid = $form['#term']['tid'];
      }
      else {
        $original_term = smartling_get_original_entity($form['#term']['tid'], $form['#term']['vocabulary_machine_name']);
        $tid = ($original_term) ? $original_term->tid : $original_term;
      }

      foreach ($languages as $langcode => $language) {
        if ($language->enabled != '0') {
          $s_locale = smartling_convert_locale_drupal_to_smartling($langcode);

          $entity_data = smartling_entity_load_by_conditions(array('rid' => $tid, 'bundle' => $form['#vocabulary']->machine_name, 'target_language' => $s_locale));

          if ($entity_data !== FALSE) {
            switch ($entity_data->status) {
              case SMARTLING_STATUS_IN_QUEUE:
                $options[$langcode] = '<span title="' . t('Term is waiting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                break;
              case SMARTLING_STATUS_IN_TRANSLATE:
                $options[$langcode] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                break;
              case SMARTLING_STATUS_TRANSLATED:
                $options[$langcode] = '<span title="' . t('Term is translated') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                break;
              case SMARTLING_STATUS_CHANGE:
                $options[$langcode] = '<span title="' . t('Term was changed after submitting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                break;
            }
          }
          else {
            $options[$langcode] = $language->name;
          }

          $check[] = ($entity_data) ? $langcode : FALSE;
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
      '#submit' => array('smartling_get_term_settings_form_submit'),
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
  }
  else {
    $form = array();
  }

  return $form;
}

/**
 * smartling_get_term_settings_form_submit
 */
function smartling_get_term_settings_form_submit($form, &$form_state) {
  $term = taxonomy_form_term_submit_build_taxonomy_term($form, $form_state);
  $language_default = language_default()->language;
  if ($term->language == $language_default) {
    $tid = $term->tid;
  }
  else {
    $original_term = smartling_get_original_entity($term->tid, $term->vocabulary_machine_name);
    $tid = $original_term->tid;
  }

  $langs = array();
  $link = smartling_get_link_to_entity($term->tid, $form['#entity_type']);

  if (count(array_filter($form_state['values']['target'])) !== 0) {

    global $user;

    $smartling_queue = DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();

    $s_locale_origin = smartling_convert_locale_drupal_to_smartling($language_default);

    // For fields method

    foreach ($form_state['values']['target'] as $d_locale) {
      if ($d_locale !== 0 && ($language_default !== $d_locale)) {
        $s_locale = smartling_convert_locale_drupal_to_smartling($d_locale);
        $entity_data = smartling_entity_load_by_conditions(array('rid' => $tid, 'bundle' => $term->vocabulary_machine_name, 'target_language' => $s_locale));

        if ($entity_data == FALSE) {
          $entity_data = smartling_entity_data_create();
          $entity_data->rid = $tid;
          $entity_data->bundle = $term->vocabulary_machine_name;
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
    $obj = new stdClass;
    $obj->rid = $tid;
    $obj->bundle = $term->vocabulary_machine_name;
    $obj->title = $term->name;
    $obj->original_language = $s_locale_origin;
    $obj->s_locales = $langs;
    $obj->submitter = $user->uid;
    // Create queue item
    $smartling_queue->createItem($obj);
    unset($obj);
    smartling_watchdog('Add smartling queue task for term tid - @tid, locale - @locale', TRUE, array('@tid' => $tid, '@locale' => implode('; ', $langs)), WATCHDOG_INFO, $link);

    $langs = implode(', ', $langs);
    drupal_set_message(t('The term "@title" has been sent to Smartling for translation to "@langs".', array('@title' => $term->name, '@langs' => $langs)));
  }

  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }

  $status = taxonomy_term_save($term);
  switch ($status) {
    case SAVED_UPDATED:
      smartling_watchdog('Updated term %term.', TRUE, array('%term' => $term->name), WATCHDOG_INFO, $link);
      // Clear the page and block caches to avoid stale data.
      cache_clear_all();
      break;
  }
}
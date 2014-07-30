<?php

/**
 * @file
 * Functions for taxonomy_term entity.
 */

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Add validate and submit
 */
function smartling_form_taxonomy_form_vocabulary_alter(&$form, &$form_state) {
  $form['#validate'][] = 'smartling_vocabulary_support_type_validate';
  $form['#submit'][] = 'smartling_vocabulary_support_type_submit';
}

/**
 * Smartling multilingual support form for vocabulary - Form Validate.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_vocabulary_support_type_validate(array &$form, array &$form_state) {
  // Set old i18n_mode.
  $vocabulary = taxonomy_vocabulary_machine_name_load($form_state['values']['machine_name']);
  if (is_object($vocabulary) && !empty($vocabulary->machine_name)) {
    $form_state['old_i18n_mode'] = i18n_taxonomy_vocabulary_mode($vocabulary);
  }

  if ($form_state['values']['i18n_mode'] == I18N_MODE_LOCALIZE || $form_state['values']['i18n_mode'] == I18N_MODE_TRANSLATE) {
    if (!smartling_is_configured()) {
      drupal_set_message(t('Please follow the page <a href="@url">"Smartling settings"</a> to setup Smartling configuration.', array('@url' => url('admin/config/regional/smartling'))), 'warning');
      form_set_error('i18n_translation');
    }
  }
}

/**
 * Smartling multilingual support form for vocabulary - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_vocabulary_support_type_submit(array &$form, array &$form_state) {
  // Delete content type in smartling support types.
  $bundle = $form_state['values']['machine_name'];
  if ($form_state['values']['i18n_mode'] != I18N_MODE_LOCALIZE && $form_state['values']['i18n_mode'] != I18N_MODE_TRANSLATE) {
    smartling_settings_get_handler()->deleteSingleBundleFromSettings('taxonomy_term', $bundle);
  }
  elseif ($form_state['values']['i18n_mode'] != $form_state['old_i18n_mode']) {
    smartling_settings_get_handler()->deleteSingleBundleFromSettings('taxonomy_term', $bundle);
  }
}

/**
 * Implements hook_form_BASE_FORM_ID_alter().
 */
function smartling_form_taxonomy_form_term_alter(&$form, $form_state) {
  $term = $form_state['term'];

  if (!empty($term->tid)) {
    if (smartling_supported_type('taxonomy_term', $form['#vocabulary']->machine_name)) {
      if (smartling_translate_fields_is_set($form['#vocabulary']->machine_name, 'taxonomy_term')) {
        $form = array_merge($form, smartling_get_term_settings_form($form, $form_state));
      }
    }
    else {
      drupal_set_message(t('Please follow the page <a href="/admin/structure/taxonomy/@machine_name/edit">"edit vocabulary"</a> to set Translation mode for Smartling.', array('@machine_name' => $form['#vocabulary']->machine_name)));
    }
  }
  elseif (empty($term->tid) && !isset($_GET['translation']) && !isset($_GET['target'])) {
    $langcode = language_default()->language;
    $form['language']['#default_value'] = $langcode;
    unset($form['language']['#options'][LANGUAGE_NONE]);
  }
}

/**
 * Taxonomy term smartling form.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for taxonomy term smartling form.
 */
function smartling_get_term_settings_form(array $form, array &$form_state) {
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
        $original_term = smartling_get_original_entity($form['#term']['tid'], $form['#entity_type']);
        $tid = ($original_term) ? $original_term->tid : $original_term;
      }

      foreach ($languages as $d_locale => $language) {
        if ($language->enabled != '0') {

          $entity_data = smartling_entity_load_by_conditions(array(
            'rid' => $tid,
            'entity_type' => $form['#entity_type'],
            'target_language' => $d_locale,
          ));
          $language_name = check_plain($language->name);

          if ($entity_data !== FALSE) {
            switch ($entity_data->status) {
              case SMARTLING_STATUS_IN_QUEUE:
                $options[$d_locale] = '<span title="' . t('Term is waiting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                break;

              case SMARTLING_STATUS_IN_TRANSLATE:
                $options[$d_locale] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                break;

              case SMARTLING_STATUS_TRANSLATED:
                $options[$d_locale] = '<span title="' . t('Term is translated') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                break;

              case SMARTLING_STATUS_CHANGE:
                $options[$d_locale] = '<span title="' . t('Term was changed after submitting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                break;
            }
          }
          else {
            $options[$d_locale] = $language_name;
          }

          $check[] = ($entity_data) ? $d_locale : FALSE;
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
 * Taxonomy term smartling form - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_get_term_settings_form_submit(array $form, array &$form_state) {
  $log = smartling_log_get_handler();
  $term = taxonomy_form_term_submit_build_taxonomy_term($form, $form_state);
  $language_default = language_default()->language;
  if ($term->language == $language_default) {
    $tid = $term->tid;
  }
  else {
    $original_term = smartling_get_original_entity($term->tid, $form['#entity_type']);
    $tid = $original_term->tid;
  }

  $langs = array();
  $link = smartling_get_link_to_entity($term->tid, $form['#entity_type']);

  if (count(array_filter($form_state['values']['target'])) !== 0) {

    global $user;

    $smartling_queue = DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();

    $d_locale_origin = $language_default;

    // For fields method.
    foreach ($form_state['values']['target'] as $d_locale) {
      if ($d_locale !== 0 && ($language_default !== $d_locale)) {

        $entity_data = smartling_entity_load_by_conditions(array(
          'rid' => $tid,
          'entity_type' => $form['#entity_type'],
          'target_language' => $d_locale,
        ));

        if ($entity_data == FALSE) {
          $entity_data = smartling_entity_data_create();
          $entity_data->rid = $tid;
          $entity_data->entity_type = $form['#entity_type'];
          $entity_data->bundle = $term->vocabulary_machine_name;
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
    $obj = new stdClass();
    $obj->rid = $tid;
    $obj->entity_type = $form['#entity_type'];
    $obj->bundle = $term->vocabulary_machine_name;
    $obj->title = $term->name;
    $obj->original_language = $d_locale_origin;
    $obj->d_locales = $langs;
    $obj->submitter = $user->uid;
    // Create queue item.
    $smartling_queue->createItem($obj);
    unset($obj);
    $log->setMessage('Add smartling queue task for term tid - @tid, locale - @locale')
      ->setVariables(array('@tid' => $tid, '@locale' => implode('; ', $langs)))
      ->setLink($link)
      ->execute();

    $langs = implode(', ', $langs);
    drupal_set_message(t('The term "@title" has been sent to Smartling for translation to "@langs".', array(
      '@title' => $term->name,
      '@langs' => $langs,
    )));
  }

  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  // For not change term status to red when send term and change content.
  $term->send_to_smartling = TRUE;
  $status = taxonomy_term_save($term);
  switch ($status) {
    case SAVED_UPDATED:
      $log->setMessage('Updated term %term.')
        ->setVariables(array('%term' => $term->name))
        ->setLink($link)
        ->execute();
      // Clear the page and block caches to avoid stale data.
      cache_clear_all();
      break;
  }
}

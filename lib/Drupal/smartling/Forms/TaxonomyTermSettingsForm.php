<?php

namespace Drupal\smartling\Forms;

class TaxonomyTermSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_get_term_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
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

      if (language_default('language') != field_valid_language(NULL, FALSE)) {
        //Otherwise if "title" module is enabled - it will spoil the title of the original node.
        $form['smartling']['error_default_language'] = array(
          '#type' => 'markup',
          '#markup' => '<p>Please switch to site\'s default language in order to execute the operations. The default language is: ' . language_default('language') . '</p>',
        );
        return $form;
      }

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
          $original_term = smartling_get_original_taxonomy_term($form['#term']['tid']);
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
              $options[$d_locale] = smartling_entity_status_message(t('Term'), $entity_data->status, $language_name, $entity_data->progress);
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
   * {@inheritdoc}
   */
  public function validateForm(array &$form, array &$form_state) {

  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
    $log = smartling_log_get_handler();
    $term = taxonomy_form_term_submit_build_taxonomy_term($form, $form_state);
    $language_default = language_default()->language;
    if ($term->language == $language_default) {
      $tid = $term->tid;
    }
    else {
      $original_term = smartling_get_original_taxonomy_term($term->tid);
      $tid = $original_term->tid;
    }

    if (intval($tid) == 0) {
      drupal_set_message(t('Original entity was not found. Please check if your current entity is "language neutral", that shouldn\'t be the case.'));
      return;
    }

    $langs = array();
    $link = smartling_get_link_to_entity($form['#entity_type'], $term);

    if (count(array_filter($form_state['values']['target'])) !== 0) {
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

          if (empty($entity_data)) {
            $entity_data = smartling_create_from_entity($term, $form['#entity_type'], $d_locale_origin, $d_locale);
          }

          smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
          $langs[] = $d_locale;
        }
      }

      // Create queue item.
      $smartling_queue->createItem($entity_data->eid);
      $log->setMessage('Add smartling queue task for term tid - @tid, locale - @locale')
        ->setVariables(array('@tid' => $tid, '@locale' => implode('; ', $langs)))
        ->setLink($link)
        ->execute();

      $langs = implode(', ', $langs);
      drupal_set_message(t('The term "@title" has been scheduled to be sent to Smartling for translation to "@langs".', array(
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
}
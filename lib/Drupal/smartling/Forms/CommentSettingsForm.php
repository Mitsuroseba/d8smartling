<?php

namespace Drupal\smartling\Forms;

class CommentSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_get_comment_settings_form';
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
      $comment = $form['#entity'];

      if (!is_null($comment->cid)) {
        foreach ($languages as $d_locale => $language) {
          if ($language->enabled != '0') {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $comment->cid,
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale,
            ));
            $language_name = check_plain($language->name);

            if ($entity_data !== FALSE) {
              $options[$d_locale] = smartling_entity_status_message(t('Comment'), $entity_data->status, $language_name, $entity_data->progress);
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
        foreach ($languages as $d_locale => $language) {
          $options[$d_locale] = check_plain($language->name);
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
        '#submit' => array('smartling_get_comment_settings_form_submit'),
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
    $comment = comment_form_submit_build_comment($form, $form_state);
    $d_locale_original = $comment->translations->original;
    $langs = array();
    $link = smartling_get_link_to_entity($form['#entity_type'], $comment);

    if (count(array_filter($form_state['values']['target'])) !== 0) {
      $smartling_queue = \DrupalQueue::get('smartling_upload');
      $smartling_queue->createQueue();

      $eids = array();
      foreach ($form_state['values']['target'] as $d_locale) {
        if ($d_locale !== 0 && ($d_locale_original !== $d_locale)) {
          $entity_data = smartling_entity_load_by_conditions(array(
            'rid' => $comment->cid,
            'entity_type' => $form['#entity_type'],
            'target_language' => $d_locale,
          ));

          if (empty($entity_data)) {
            $entity_data = smartling_create_from_entity($comment, $form['#entity_type'], $d_locale_original, $d_locale);
          }

          smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
          $langs[] = $d_locale;
          $eids[]  = $entity_data->eid;
        }
      }

      $smartling_queue->createItem($eids);
      $log->setMessage('Add smartling queue task for comment cid - @cid, locale - @locale')
        ->setVariables(array(
          '@cid' => $comment->cid,
          '@locale' => implode('; ', $langs),
        ))
        ->setLink($link)
        ->execute();

      $langs = implode(', ', $langs);
      drupal_set_message(t('The comment "@title" has been scheduled to be sent to Smartling for translation to "@langs".', array(
        '@title' => $comment->subject,
        '@langs' => $langs,
      )));
    }

    if (isset($_GET['destination'])) {
      unset($_GET['destination']);
    }
    // For not change comment status to red when send comment and change content.
    $comment->send_to_smartling = TRUE;
    comment_save($comment);
    $log->setMessage('Updated comment %comment.')
      ->setVariables(array('%comment' => $comment->subject))
      ->setLink($link)
      ->execute();
  }
}
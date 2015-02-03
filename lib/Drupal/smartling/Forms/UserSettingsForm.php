<?php

namespace Drupal\smartling\Forms;

class UserSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_get_user_settings_form';
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
      $user = $form['#user'];

      if (!is_null($user->uid)) {
        foreach ($languages as $d_locale => $language) {
          if ($language->enabled != '0') {

            $entity_data = smartling_entity_load_by_conditions(array(
              'rid' => $user->uid,
              'entity_type' => $form['#entity_type'],
              'target_language' => $d_locale,
            ));
            $language_name = check_plain($language->name);

            if ($entity_data !== FALSE) {
              $options[$d_locale] = smartling_entity_status_message(t('User'), $entity_data->status, $language_name, $entity_data->progress);
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
        '#submit' => array('smartling_get_user_settings_form_submit'),
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
    $account = $form_state['user'];
    $category = $form['#user_category'];
    // Remove unneeded values.
    form_state_values_clean($form_state);

    // Before updating the account entity, keep an unchanged copy for use with
    // user_save() later. This is necessary for modules implementing the user
    // hooks to be able to react on changes by comparing the values of $account
    // and $edit.
    $account_unchanged = clone $account;

    entity_form_submit_build_entity('user', $account, $form, $form_state);

    // Populate $edit with the properties of $account, which have been edited on
    // this form by taking over all values, which appear in the form values too.
    $edit = array_intersect_key((array) $account, $form_state['values']);

    $d_locale_origin = $account->translations->original;
    $langs = array();
    $link = smartling_get_link_to_entity($form['#entity_type'], $account);

    if (count(array_filter($form_state['values']['target'])) !== 0) {
      $smartling_queue = DrupalQueue::get('smartling_upload');
      $smartling_queue->createQueue();

      foreach ($form_state['values']['target'] as $d_locale) {
        if ($d_locale !== 0 && ($d_locale_origin !== $d_locale)) {

          $entity_data = smartling_entity_load_by_conditions(array(
            'rid' => $account->uid,
            'entity_type' => $form['#entity_type'],
            'target_language' => $d_locale,
          ));

          if (empty($entity_data)) {
            $entity_data = smartling_create_from_entity($account, $form['#entity_type'], $d_locale_origin, $d_locale);
          }

          smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
          $langs[] = $d_locale;
        }
      }

      // Create queue item.
      $smartling_queue->createItem($entity_data->eid);

      $log->setMessage('Add smartling queue task for user uid - @uid, locale - @locale')
        ->setVariables(array(
          '@uid' => $account->uid,
          '@locale' => implode('; ', $langs),
        ))
        ->setLink($link)
        ->execute();

      $langs = implode(', ', $langs);
      drupal_set_message(t('The user "@title" has been scheduled to be sent to Smartling for translation to "@langs".', array(
        '@title' => $account->name,
        '@langs' => $langs,
      )));
    }

    if (isset($_GET['destination'])) {
      unset($_GET['destination']);
    }
    // For not change account status to red when send account and change content.
    $account_unchanged->send_to_smartling = TRUE;
    user_save($account_unchanged, $edit, $category);
    $form_state['values']['uid'] = $account->uid;

    if ($category == 'account' && !empty($edit['pass'])) {
      // Remove the password reset tag since a new password was saved.
      unset($_SESSION['pass_reset_' . $account->uid]);
    }
    // Clear the page cache because pages can contain
    // usernames and/or profile information.
    cache_clear_all();

    $log->setMessage('Updated user %user.')
      ->setVariables(array('%user' => $account->name))
      ->setLink($link)
      ->execute();
  }
}
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
function smartling_form_user_profile_form_alter(&$form, &$form_state) {
  if (smartling_supported_type($form['#entity_type'], $form['#bundle'])) {
    $user = $form['#user'];

    if (!empty($user->uid)) {
      $form = array_merge($form, smartling_get_user_settings_form($form, $form_state));
    }
  }
  else {
    drupal_set_message(t('Add translatable field to user entity to use Smartling translate.'), 'info');
  }
}

/**
 * User smartling form.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for user smartling form.
 */
function smartling_get_user_settings_form(array $form, array &$form_state) {
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
            switch ($entity_data->status) {
              case SMARTLING_STATUS_IN_QUEUE:
                $options[$d_locale] = '<span title="' . t('User is waiting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                break;

              case SMARTLING_STATUS_IN_TRANSLATE:
                $options[$d_locale] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                break;

              case SMARTLING_STATUS_TRANSLATED:
                $options[$d_locale] = '<span title="' . t('User is translated') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                break;

              case SMARTLING_STATUS_CHANGE:
                $options[$d_locale] = '<span title="' . t('User was changed after submitting for translation') . '"><span class="lang-name">' . $language_name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
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
 * User smartling form - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_get_user_settings_form_submit(array $form, array &$form_state) {
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
  $link = smartling_get_link_to_entity($account->uid, $form['#entity_type']);

  if (count(array_filter($form_state['values']['target'])) !== 0) {
    global $user;

    $smartling_queue = DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();

    foreach ($form_state['values']['target'] as $d_locale) {
      if ($d_locale !== 0 && ($d_locale_origin !== $d_locale)) {

        $entity_data = smartling_entity_load_by_conditions(array(
          'rid' => $account->uid,
          'entity_type' => $form['#entity_type'],
          'target_language' => $d_locale,
        ));

        if ($entity_data == FALSE) {
          $entity_data = smartling_entity_data_create();
          $entity_data->rid = $account->uid;
          $entity_data->entity_type = $form['#entity_type'];
          $entity_data->bundle = $form['#bundle'];
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
    drupal_set_message(t('The user "@title" has been sent to Smartling for translation to "@langs".', array(
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

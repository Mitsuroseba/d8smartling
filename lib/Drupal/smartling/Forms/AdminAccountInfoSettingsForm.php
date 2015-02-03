<?php

namespace Drupal\smartling\Forms;

class AdminAccountInfoSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_admin_account_info_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $smartling_settings = smartling_settings_get_handler();

    $form['account_info'] = array(
      'actions' => array(
        '#type' => 'actions',
      ),
    );

    $form['account_info']['title'] = array(
      '#type' => 'item',
      '#title' => t('Account info'),
    );

    $form['account_info']['api_url'] = array(
      '#type' => 'textfield',
      '#title' => t('API URL'),
      '#default_value' => $smartling_settings->getApiUrl(),
      '#size' => 25,
      '#maxlength' => 255,
      '#required' => FALSE,
      '#description' => t('Set api url. Default: @api_url', array('@api_url' => SMARTLING_DEFAULT_API_URL)),
    );

    $form['account_info']['project_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Project Id'),
      '#default_value' => $smartling_settings->getProjectId(),
      '#size' => 25,
      '#maxlength' => 25,
      '#required' => TRUE,
    );

    $form['account_info']['smartling_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Key'),
      '#default_value' => '',
      '#description' => t('Current key: @key', array('@key' => smartling_hide_key($smartling_settings->getKey()))),
      '#size' => 40,
      '#maxlength' => 40,
      '#required' => FALSE,
    );

    $form['account_info']['production_retrieval_type'] = array(
      '#type' => 'radios',
      '#title' => t('Retrieval type'),
      '#default_value' => $smartling_settings->getRetrievalType(),
      '#options' => $smartling_settings->getRetrievalTypeOptions(),
      '#description' => t('Param for download translate.'),
    );

    $target_language_options_list = $smartling_settings->getTargetLanguageOptionsList();
    if (!empty($target_language_options_list)) {
      $form['account_info']['target_locales'] = array(
        '#type' => 'checkboxes',
        '#options' => $target_language_options_list,
        '#title' => t('Target Locales'),
        '#default_value' => $smartling_settings->getTargetLocales(),
        '#prefix' => '<div class="wrap-target-locales">',
      );

      $total = count($target_language_options_list);
      $counter = 0;
      $locales_convert_array = $smartling_settings->getLocalesConvertArray();
      foreach (array_keys($target_language_options_list) as $langcode) {
        $counter++;

        $form['account_info']['target_locales_text_key_' . $langcode] = array(
          '#type' => 'textfield',
          '#title' => '',
          '#title_display' => 'invisible',
          '#default_value' => (isset($locales_convert_array[$langcode]) && ($locales_convert_array[$langcode] != $langcode)) ? $locales_convert_array[$langcode] : '',
          '#size' => 6,
          '#maxlength' => 10,
          '#required' => FALSE,
          '#states' => array(
            'disabled' => array(
              ':input[name="target_locales[' . $langcode . ']"]' => array('checked' => FALSE),
            ),
          ),
        );

        if ($counter == 1) {
          $form['account_info']['target_locales_text_key_' . $langcode]['#prefix'] = '<div class="wrap-target-locales-text-key">';
        }

        if ($counter == $total) {
          $form['account_info']['target_locales_text_key_' . $langcode]['#suffix'] = '</div></div>';
        }
      }
    }
    else {
      $form['account_info']['target_locales'] = array(
        '#type' => 'checkboxes',
        '#options' => array(),
        '#title' => t('Target Locales'),
        '#default_value' => array(),
        '#description' => l(t('At least two languages must be enabled. Please change language settings.'), 'admin/config/regional/language'),
      );
    }

    $form['account_info']['default_language'] = array(
      '#type' => 'item',
      '#title' => t('Default language'),
    );

    $form['account_info']['default_language_markup'] = array(
      '#markup' => '<p>' . t('Site default language: @lang', array('@lang' => language_default()->name)) . '</p>',
      '#suffix' => '<p>' . l(t('Change default language'), 'admin/config/regional/language') . '</p>',
    );

    $form['account_info']['callback_info_title'] = array(
      '#type' => 'item',
      '#title' => t('Callback URL'),
    );

    $form['account_info']['callback_url_use'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use Smartling callback: /smartling/callback/%cron_key'),
      '#default_value' => $smartling_settings->getCallbackUrlUse(),
      '#required' => FALSE,
    );

    $form['account_info']['auto_authorize_content_title'] = array(
      '#type' => 'item',
      '#title' => t('Auto authorize'),
    );

    $form['account_info']['auto_authorize_content'] = array(
      '#type' => 'checkbox',
      '#title' => t('Auto authorize content'),
      '#default_value' => $smartling_settings->getAutoAuthorizeContent(),
      '#required' => FALSE,
    );

    $form['account_info']['actions']['submit'][] = array(
      '#type' => 'submit',
      '#name' => 'save',
      '#value' => t('Save'),
    );
    $form['account_info']['actions']['submit'][] = array(
      '#type' => 'submit',
      '#name' => 'test_connection',
      '#value' => t('Test connection'),
    );

    $form['#validate'][] = 'smartling_admin_account_info_settings_form_validate';
    $form['#submit'][] = 'smartling_admin_account_info_settings_form_submit';

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, array &$form_state) {
    $project_id = '';
    if (isset($form_state['values']['api_url']) && !empty($form_state['values']['api_url'])) {
      $api_url = check_plain($form_state['values']['api_url']);
      $status = valid_url($api_url, TRUE);
      if (!$status) {
        drupal_set_message(t('API URL has wrong format'), 'error');
        form_set_error('api_url');
      }
    }

    if (isset($form_state['values']['project_id']) && !empty($form_state['values']['project_id'])) {
      $project_id = trim($form_state['values']['project_id']);
      if (!smartling_project_id_check($project_id)) {
        drupal_set_message(t('Please enter valid Smartling Project Id.'), 'error');
        form_set_error('project_id');
      }
    }

    if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
      $smartling_key = trim($form_state['values']['smartling_key']);

      if (!smartling_api_key_check($smartling_key)) {
        drupal_set_message(t('Please enter valid Smartling key.'), 'error');
        form_set_error('smartling_key');
      }
    }


    // Target locales validate.
    if (count(array_filter($form_state['values']['target_locales'])) == 0) {
      drupal_set_message(t('At least one locale must be selected'), 'error');
      form_set_error('target_locales');
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
    // Add default private path.
    if (variable_get('file_private_path', '') == '') {
      $directory = 'sites/default/files/private';
      variable_set('file_private_path', $directory);

      if (!is_dir($directory) && !drupal_mkdir($directory, NULL, TRUE)) {
        // If the directory does not exists and cannot be created.
        drupal_set_message(t('The directory %directory does not exist and could not be created.', array('%directory' => $directory)));
        watchdog('file system', 'The directory %directory does not exist and could not be created.', array('%directory' => $directory), WATCHDOG_ERROR);
      }

      if (is_dir($directory) && !is_writable($directory) && !drupal_chmod($directory)) {
        // If the directory is not writable and cannot be made so.
        drupal_set_message(t('The directory %directory exists but is not writable and could not be made writable.', array('%directory' => $directory)));
        watchdog('file system', 'The directory %directory exists but is not writable and could not be made writable.', array('%directory' => $directory), WATCHDOG_ERROR);
      }
      elseif (is_dir($directory)) {
        // Create private .htaccess file.
        file_create_htaccess($directory);
      }
    }

    $smartling_settings = smartling_settings_get_handler();
    // Account settings.
    if (isset($form_state['values']['api_url'])) {
      $smartling_settings->setApiUrl(check_plain($form_state['values']['api_url']));
    }
    if (isset($form_state['values']['project_id'])) {
      $smartling_settings->setProjectId(check_plain($form_state['values']['project_id']));
    }
    if (isset($form_state['values']['smartling_key']) && !empty($form_state['values']['smartling_key'])) {
      $smartling_settings->setKey(check_plain(trim($form_state['values']['smartling_key'])));
    }
    // Retrieval type.
    if (isset($form_state['values']['production_retrieval_type']) && !empty($form_state['values']['production_retrieval_type'])) {
      $smartling_settings->setRetrievalType($form_state['values']['production_retrieval_type']);
      inject_flush_caches();
    }

    // Target locales.
    $smartling_settings->makeTargetLocales($form_state['values']['target_locales']);
    $smartling_settings->makeLocalesConvertArray($form_state['values']);

    // Callback.
    if (isset($form_state['values']['callback_url_use'])) {
      $smartling_settings->setCallbackUrlUse($form_state['values']['callback_url_use']);
    }

    // AutoAuthorizeContent.
    if (isset($form_state['values']['auto_authorize_content'])) {
      $smartling_settings->setAutoAuthorizeContent($form_state['values']['auto_authorize_content']);
    }

    drupal_set_message(t('Account settings saved.'));

    if ($form_state['triggering_element']['#name'] == 'save') {
      drupal_goto(current_path());
    }

    // Test.
    if ($form_state['triggering_element']['#name'] == 'test_connection') {
      $api = drupal_container()->get('smartling.api_wrapper');

      $connection = $api->testConnection($form_state['values']['target_locales']);
      foreach ($connection as $locale => $val) {
        if ($val) {
          drupal_set_message(t('Test connection for locale @s_locale is success.', array('@s_locale' => $locale)));
        }
        else {
          drupal_set_message(t('Test connection for locale @s_locale is fail.', array('@s_locale' => $locale)), 'error');
        }
      }

      drupal_goto(current_path());
    }
  }
}
<?php

namespace Drupal\smartling\Forms;

class AdminExpertSettingsForm implements FormInterface {

  protected $settings;
  protected $logger;

  public function __construct($settings, $logger) {
    $this->settings = $settings;
    $this->logger = $logger;
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_expert_info_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $settings = $this->settings;

    $form['log_info']['log_mode'] = array(
      '#type' => 'radios',
      '#title' => t('Smartling mode'),
      '#default_value' => $settings->getLogMode(),
      '#options' => $settings->getLogModeOptions(),
      '#description' => t('Log ON dy default.'),
    );

    $form['log_info']['async_mode'] = array(
      '#type' => 'checkbox',
      '#title' => t('Asynchronous mode'),
      '#description' => t('If you uncheck this, the Smartling Connector will attempt to submit content immediately to Smartling servers.'),
      '#default_value' => $settings->getAsyncMode(),
    );

    $form['log_info']['convert_entities_before_translation'] = array(
      '#type' => 'checkbox',
      '#title' => t('Convert entities before translation'),
      '#description' => t('If this is unchecked, then you should convert your content manually from "language-neutral" to default language (usually english) before sending content item for translation.'),
      '#default_value' => $settings->getConvertEntitiesBeforeTranslation(),
    );

    $form['log_info']['ui_translations_merge_mode'] = array(
      '#type' => 'checkbox',
      '#title' => t('UI translation mode'),
      '#description' => t('If checked: Translation import mode keeping existing translations and only inserting new strings, strings overwrite happens otherwise.'),
      '#default_value' => $settings->getUITranslationsMergeMode(),
    );

    $form['log_info']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );

    $form['#submit'][] = 'smartling_admin_expert_settings_form_submit';

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
    if (isset($form_state['values']['log_mode'])) {
      if ($form_state['values']['log_mode'] == FALSE) {
        $this->logger->info('Smartling log OFF', array(), TRUE);
      }
      elseif ($form_state['values']['log_mode'] == TRUE) {
        $this->logger->info('Smartling log ON', array(), TRUE);
      }
      $this->settings->setLogMode($form_state['values']['log_mode']);
    }

    $this->settings->setAsyncMode($form_state['values']['async_mode']);
    $this->settings->setConvertEntitiesBeforeTranslation($form_state['values']['convert_entities_before_translation']);
    $this->settings->getUITranslationsMergeMode($form_state['values']['ui_translations_merge_mode']);

    drupal_goto(current_path(), array('fragment' => 'smartling-expert-settings'));
  }
}
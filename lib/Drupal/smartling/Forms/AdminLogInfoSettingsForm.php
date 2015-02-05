<?php

namespace Drupal\smartling\Forms;

class AdminLogInfoSettingsForm implements FormInterface {

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
    return 'smartling_admin_log_info_settings_form';
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

    $form['log_info']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );

    $form['#submit'][] = 'smartling_admin_log_info_settings_form_submit';

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
        $this->logger->setMessage('Smartling log OFF')
          ->setConsiderLog(FALSE)
          ->execute();
      }
      elseif ($form_state['values']['log_mode'] == TRUE) {
        $this->logger->setMessage('Smartling log ON')
          ->setConsiderLog(FALSE)
          ->execute();
      }
      $this->settings->setLogMode($form_state['values']['log_mode']);
    }
    drupal_goto(current_path(), array('fragment' => 'smartling-smartling-log'));
  }
}
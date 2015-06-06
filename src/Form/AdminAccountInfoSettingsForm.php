<?php

/**
 * @file
 * Main settings form.
 */

namespace Drupal\smartling\Form;

use Drupal;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

class AdminAccountInfoSettingsForm extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'smartling.admin',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_admin_account_info_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $config = $this->config('smartling.settings');

//    $form['account_info'] = array(
//      'actions' => array(
//        '#type' => 'actions',
//      ),
//      '#attached' => array(
//        'js' => array(drupal_get_path('module', 'smartling') . '/js/smartling_check_all.js'),
//      ),
//    );
//    drupal_add_js(array('smartling' => array('checkAllId' => array('#edit-target-locales'))), 'setting');

    $form['account_info']['title'] = array(
      '#type' => 'item',
      '#title' => t('Account info'),
    );

    $form['account_info']['api_url'] = array(
      '#type' => 'textfield',
      '#title' => t('API URL'),
      '#default_value' => $config->get('smartling_api_url'),
      '#size' => 25,
      '#maxlength' => 255,
      '#required' => FALSE,
      '#description' => t('Set api url. Default: @api_url', array('@api_url' => $config->get('smartling_api_url'))),
    );

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    Drupal::service('config.factory')
      ->getEditable('smartling.settings')
      ->set('smartling_api_url', $form_state->getValue('api_url'))
      ->save();

    parent::submitForm($form, $form_state);
  }

}
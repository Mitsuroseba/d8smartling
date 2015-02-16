<?php

namespace Drupal\smartling_translate_page_popup\Forms;

class pageEntitiesTranslationForm implements \Drupal\smartling\Forms\FormInterface {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_page_entities_translation_form';
  }

  public function buildForm(array $form, array &$form_state) {
    $options = array();
    //print_r($_SESSION['smartling']['page_entities']);
    foreach ($_SESSION['smartling']['page_entities'] as $id => $title) {
      list($entity_id, $entity_type) = explode('_||_', $id);
      smartling_entity_load_by_conditions(array(
        'rid' => $entity_id,
        'entity_type' => $entity_type,
        //'target_language' => $entity_data->target_language,
      ));

      $options [$id] = $title;
    }
    unset($_SESSION['smartling']['page_entities']);

    $form['smartling']['items'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,//$_SESSION['smartling']['page_entities'], //drupal_map_assoc(array(t('SAT'), t('ACT'))),
      '#title' => t('What items would you like to translate?'),
    );

    $form['smartling']['languages'] = array(
      '#type' => 'checkboxes',
      '#options' => smartling_language_options_list(),//$_SESSION['smartling']['page_entities'], //drupal_map_assoc(array(t('SAT'), t('ACT'))),
      '#title' => t('Languages'),
    );

    $form['smartling']['submit'] = array(
      '#type' => 'submit',
      '#ajax' => array(
        'callback' => 'smartling_translate_page_popup_form_submit',
      ),
      '#value' => t('Translate'),
    );

    return $form;
  }

  public function validateForm(array &$form, array &$form_state) {

  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
    //print_r($form_state['input']);
    //die('hi');
    //$elem
    $commands[] = ajax_command_replace('#smartling-translate-page-popup-form', print_r($form_state['input'], TRUE));
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
  }
}
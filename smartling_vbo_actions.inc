<?php

/**
 * Smartling do translate action.
 *
 * @param object $entity
 *   Action entity object.
 * @param array $context
 *   Context info.
 *
 * @global object $user
 *   Drupal user object.
 */
function smartling_do_translate_action($entity, array $context) {
  return drupal_container()->get('smartling.queue_managers.upload')->addRawEntity($context['entity_type'], $entity, $context['translate_to_langs']);
}

/**
 * Smartling do translate action form.
 *
 * @param array $context
 *   Context info array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return FAPI array for smartling do translate action form.
 */
function smartling_do_translate_action_form(array $context, array &$form_state) {
  $form['do_translate_action']['languages'] = array(
    '#type' => 'checkboxes',
    '#options' => smartling_language_options_list(),
    '#title' => t('Check language'),
  );
  return $form;
}

/**
 * Smartling do translate action form - Form Validate.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 */
function smartling_do_translate_action_validate(array $form, array &$form_state) {
  $select_languages = array();
  if (!empty($form_state['input']['languages'])) {
    foreach ($form_state['input']['languages'] as $d_locale) {
      if (!empty($d_locale)) {
        $select_languages[] = $d_locale;
      }
    }
  }

  if (count($select_languages) > 0) {
    $form_state['translate_to_langs'] = $select_languages;
  }
  else {
    form_set_error('languages', t('Check language, please.'));
  }
}

/**
 * Smartling do translate action form - Form Submit.
 *
 * @param array $form
 *   FAPI array.
 * @param array $form_state
 *   FAPI array.
 *
 * @return array
 *   Return array with languages to translate.
 */
function smartling_do_translate_action_submit(array $form, array $form_state) {
  if (!smartling_is_configured()) {
    return;
  }

  $count_op = count($form_state['selection']);
  $log = smartling_log_get_handler();
  $log->setMessage(format_plural($count_op, 'Smartling Bulk Submit - add 1 entity to queue', 'Smartling Bulk Submit - add @count entities to queue'))
    ->execute();
  return array('translate_to_langs' => $form_state['translate_to_langs']);
}






function smartling_do_download_translation_action_form() {
  $form = array();

  $form['confirm'] = array(
    '#type' => 'checkbox',
    '#title' => t('Download translation of the selected items.'),
  );

  return $form;
}

function smartling_do_download_translation_action_validate($form, $form_state) {
  if (!@$form['confirm']['#value']) {
    form_set_error('confirm', t('Please confirm explicitly if you want to continue.'));
  }
}

function smartling_do_download_translation_action_submit($form, $form_state) {
  return array();
}

function smartling_do_download_translation_action(&$entity, $context) {
  smartling_queue_download_update_translated_item_process($entity->eid);
}






function smartling_do_resend_translation_action_form() {
  $form = array();

  $form['confirm'] = array(
    '#type' => 'checkbox',
    '#title' => t('Resend the selected items for translation.'),
  );

  return $form;
}

function smartling_do_resend_translation_action_validate($form, $form_state) {
  if (!@$form['confirm']['#value']) {
    form_set_error('confirm', t('Please confirm explicitly if you want to continue.'));
  }
}

function smartling_do_resend_translation_action_submit($form, $form_state) {
  return array();
}

function smartling_do_resend_translation_action(&$entity, $context) {
  drupal_container()->get('smartling.wrappers.entity_data_wrapper')->setEntity($entity)->setStatusByEvent(SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE)->save();
  drupal_container()->get('smartling.queue_managers.upload')->add(array($entity->eid));

  drupal_set_message(t('Entity "@title" has been successfully enqueued for resubmission.', array('@title' => $entity->title)));
}
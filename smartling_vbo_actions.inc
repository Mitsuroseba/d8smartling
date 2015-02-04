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
  $entity_type = $context['entity_type'];
  $log = smartling_log_get_handler();

  $wrapper = entity_metadata_wrapper($entity_type, $entity);
  $id      = $wrapper->getIdentifier();
  $bundle  = $wrapper->getBundle();

  if (!smartling_supported_type($entity_type, $bundle) || !smartling_translate_fields_is_set($bundle, $entity_type)) {
    $link = smartling_get_link_to_entity($entity_type, $entity);

    drupal_set_message(t("Type '@type' is not supported or it's not configured in Smartling.", array('@type' => $bundle)), 'warning');
    $log->setMessage("Type '@type' is not supported or it's not configured in Smartling.")
      ->setVariables(array('@type' => $bundle))
      ->setConsiderLog(FALSE)
      ->setSeverity(WATCHDOG_ERROR)
      ->setLink($link)
      ->execute();

    return;
  }

  $smartling_queue = DrupalQueue::get('smartling_upload');
  $smartling_queue->createQueue();

  $queued_eids = array();
  switch ($entity_type) {
    case 'node':
      $node_status = NULL;
      if (smartling_nodes_method($entity->type)) {
        $node_status = smartling_nodes_method_node_status($entity);

        if (($entity->tnid == '0') && in_array($node_status, array(SMARTLING_ORIGINAL_NODE, SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE))) {
          $entity->tnid = $entity->nid;
          node_save($entity);
        }
      }
      break;

    case 'taxonomy_term':
      $language_default = language_default()->language;

      if (entity_language($entity_type, $entity) == $language_default) {
        $id = $entity->tid;
      }
      else {
        $original_term = smartling_get_original_taxonomy_term($entity);
        $id = $original_term->tid;
      }

      if (intval($id) == 0) {
        drupal_set_message(t('Original entity was not found. Please check if your current entity is "language neutral", that shouldn\'t be the case.'));
        return;
      }
      break;
  }

  // $d_locale_original = language_default()->language;
  // $d_locale_original = $entity->translations->original;
  $d_locale_original = entity_language($entity_type, $entity);
  foreach ($context['translate_to_langs'] as $d_locale) {
    if ($d_locale == $d_locale_original) {
      continue;
    }

    if (!((smartling_nodes_method($entity->type) && in_array($node_status, array(SMARTLING_ORIGINAL_NODE, SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE)))
        || (smartling_fields_method($entity->type))) && ($entity_type == 'node')) {
      continue;
    }

    $smartling_data = smartling_entity_load_by_conditions(array(
      'rid' => $id,
      'entity_type' => $entity_type,
      'target_language' => $d_locale,
    ));

    if ($smartling_data == FALSE) {
      $smartling_data = smartling_create_from_entity($entity, $entity_type, $d_locale_original, $d_locale);
    }

    smartling_set_translation_status($smartling_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);

    $queued_eids[] = $smartling_data->eid;
  }

  if (!empty($queued_eids)) {
    $smartling_queue->createItem($queued_eids);
  }
  // Create content hash (Fake entity update).
  smartling_entity_update($entity, $entity_type);
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
  smartling_set_translation_status($entity, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);

  $smartling_queue = DrupalQueue::get('smartling_upload');
  $smartling_queue->createQueue();
  $smartling_queue->createItem(array($entity->eid));

  drupal_set_message(t('Entity "@title" has been successfully enqueued for resubmission.', array('@title' => $entity->title)));
}
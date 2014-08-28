<?php

function smartling_manual_management_form($form, $form_state) {
  $form = array();

  $form['entity_type'] = array(
    '#type' => 'select',
    '#title' => t('Entity types'),
    '#options' => array(
      'node' => 'Node',
      'comment' => 'Comment',
      'taxonomy_term' => 'Taxonomy term',
      'user' => 'User',
    ),
    '#required' => TRUE,
    '#default_value' => 'node',
  );

  $form['entity_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity ID'),
    '#required' => TRUE,
  );

  $form['language'] = array(
    '#type' => 'select',
    '#title' => t('Language'),
    '#required' => TRUE,
    '#options' => locale_language_list(),
  );

  $form['operation'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#options' => array(
      'upload' => t('Upload content'),
      'download' => t('Download transation'),
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function smartling_manual_management_form_submit($form, $form_state) {
  $log = smartling_log_get_handler();

  $entity = entity_load_single($form_state['values']['entity_type'], $form_state['values']['entity_id']);
  $wrapper = entity_metadata_wrapper($form_state['values']['entity_type'], $entity);
  $entity_type = $form_state['values']['entity_type'];
  $target_language = $form_state['values']['language'];

  if ($form_state['values']['operation'] == 'upload') {
    $smartling_data = smartling_entity_load_by_conditions(array(
      'rid' => $wrapper->getIdentifier(),
      'target_language' => $form_state['values']['language'],
    ));

    if (!$smartling_data) {
      // @todo verify that entity has language property.
      $smartling_data = smartling_create_from_entity($entity, $entity_type, $entity->language, $target_language);
    }

    $log->setMessage('Smartling start send to translate action for node id - @nid (@d_locales).')
      ->setVariables(array(
        '@nid' => $wrapper->getIdentifier(),
        '@s_locales' => implode('; ', array($target_language)),
      ))
      // @TODO remove this ugly hardcode.
      ->setLink(l(t('Edit node'), 'node/' . $wrapper->getIdentifier() . '/edit'))
      ->execute();

    smartling_queue_send_to_translate_process($smartling_data->eid);
  }
  elseif ($form_state['values']['operation'] == 'download') {
    $smartling_data = smartling_entity_load_by_conditions(array(
      'rid' => $wrapper->getIdentifier(),
      'target_language' => $form_state['values']['language'],
    ));

    smartling_queue_download_update_translated_item_process($smartling_data->eid);
  }
}

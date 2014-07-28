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
  global $user;
  $log = smartling_log_get_handler();

  if ($form_state['values']['operation'] == 'upload') {
    /* var $entity Entity */
    $entity = entity_load_single($form_state['values']['entity_type'], $form_state['values']['entity_id']);
    /* var $entity EntityDrupalWrapper */
    $wrapper = entity_metadata_wrapper($form_state['values']['entity_type'], $entity);
    $obj = new stdClass();
    $obj->rid = $wrapper->getIdentifier();
    $obj->entity_type = $wrapper->type();
    $obj->bundle = $wrapper->getBundle();
    $obj->title = $wrapper->label();
    $obj->original_language = entity_language($wrapper->type(), $entity);
    $obj->d_locales = array($form_state['values']['language']);
    $obj->submitter = $user->uid;
    $log->setMessage('Smartling start send to translate action for node id - @nid (@d_locales).')
      ->setVariables(array(
        '@nid' => $obj->rid,
        '@s_locales' => implode('; ', array($form_state['values']['language'])),
      ))
      // @TODO remove this ugly hardcode.
      ->setLink(l(t('Edit node'), 'node/' . $wrapper->getIdentifier() . '/edit'))
      ->execute();
    smartling_queue_send_to_translate_process($obj);
    unset($obj);
  }
}
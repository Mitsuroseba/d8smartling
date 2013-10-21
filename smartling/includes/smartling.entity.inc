<?php

/**
 * @file
 */

/**
 * Implements hook_entity_info().
 */
function smartling_entity_info() {
  $info['smartling_entity_data'] = array(
    // A human readable label to identify our entity.
    'label' => t('Smartling Entity Data'),
    'entity class' => 'SmartlingEntityData',
    'controller class' => 'SmartlingEntityDataController',
    'views controller class' => 'EntityDefaultViewsController',
    // The controller for our Entity, extending the Drupal core controller.
    //'controller class' => 'SmartlingEntityDataController',
    // The table for this entity defined in hook_schema()
    'base table' => 'smartling_entity_data',
    // Returns the uri elements of an entity
    'uri callback' => 'smartling_entity_data_uri',
    // IF fieldable == FALSE, we can't attach fields.
    'fieldable' => FALSE,
    // entity_keys tells the controller what database fields are used for key
    // functions. It is not required if we don't have bundles or revisions.
    // Here we do not support a revision, so that entity key is omitted.
    'entity keys' => array(
      'id' => 'eid', // The 'id' (basic_id here) is the unique id.
    //'bundle' => 'bundle_type' // Bundle will be determined by the 'bundle_type' field
    ),
    //'bundle keys' => array(
    //  'bundle' => 'bundle_type',
    //),
    // FALSE disables caching. Caching functionality is handled by Drupal core.
    'static cache' => FALSE,
    // Bundles are alternative groups of fields or configuration
    // associated with a base entity type.
    /*
      'bundles' => array(
      'first_example_bundle' => array(
      'label' => 'First example bundle',
      // 'admin' key is used by the Field UI to provide field and
      // display UI pages.
      'admin' => array(
      'path' => 'admin/structure/entity_example_basic/manage',
      'access arguments' => array('administer entity_example_basic entities'),
      ),
      ),
      ),
     *
     */
    // View modes allow entities to be displayed differently based on context.
    // As a demonstration we'll support "Tweaky", but we could have and support
    // multiple display modes.
    'view modes' => array(
      'full' => array(
        'label' => t('Default'),
        'custom settings' => FALSE,
      ),
    )
  );

  return $info;
}

/**
 * Fetch a basic object.
 */
function smartling_entity_data_load($entity_id = NULL, $reset = FALSE) {
  $entity_ids = (isset($entity_id) ? array($entity_id) : array());
  $basic = smartling_entity_data_load_multiple($entity_ids, array(), $reset);
  return $basic ? reset($basic) : FALSE;
}

/**
 * Loads multiple basic entities.
 */
function smartling_entity_data_load_multiple($entity_ids = FALSE, $conditions = array(), $reset = FALSE) {
  return entity_load('smartling_entity_data', $entity_ids, $conditions, $reset);
}

/**
 * We create the entity by calling the controller.
 */
function smartling_entity_data_create() {
  return entity_get_controller('smartling_entity_data')->create();
}

/**
 * We save the entity by calling the controller.
 */
function smartling_entity_data_save(&$entity) {
  return entity_get_controller('smartling_entity_data')->save($entity);
}

/**
 * Use the controller to delete the entity.
 */
function smartling_entity_data_delete($entity) {
  entity_get_controller('smartling_entity_data')->delete($entity);
}

/**
 * Use the controller to multiple delete the entities.
 */
function smartling_entity_data_delete_multiple($entity_ids) {
  entity_delete_multiple('smartling_entity_data', $entity_ids);
}

/**
 * Implements the uri callback.
 */
function smartling_entity_data_uri($basic) {
  return array(
    'path' => 'admin/config/regional/smartling/entity-data/' . $basic->eid,
  );
}

/**
 * Basic information for the page.
 */
function smartling_entity_data_info_page() {
  $content['preface'] = array(
    '#type' => 'item',
    '#markup' => t('The smartling_entity_data provides a simple smartling_entity_data.')
  );
  if (user_access('administer smartling_entity_data entities')) {
    $content['preface']['#markup'] = t('You can administer these and add fields and change the view !link.', array('!link' => l(t('here'), 'admin/config/regional/smartling/entity-data/manage'))
    );
  }
  $content['table'] = smartling_entity_data_list_entities();

  return $content;
}

/**
 * Returns a render array with all smartling_entity_data entities.
 */
function smartling_entity_data_list_entities() {
  $content = array();
  // Load all of our entities.
  $entities = smartling_entity_data_load_multiple();
  if (!empty($entities)) {
    foreach ($entities as $entity) {
      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'id' => $entity->eid,
          'page_name' => l($entity->page_name, 'admin/config/regional/smartling/entity-data/' . $entity->eid),
          'locale_name' => $entity->locale_name,
          'progress' => $entity->progress,
          'submitter' => $entity->submitter,
          'submission_date' => $entity->submission_date,
          'download' => $entity->download,
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('page_name'), t('locale_name'), t('progress'), t('submitter'), t('submission_date'), t('download')),
    );
  }
  else {
    // There were no entities. Tell the user.
    $content[] = array(
      '#type' => 'item',
      '#markup' => t('No smartling_entity_data entities currently exist.'),
    );
  }
  return $content;
}

/**
 * Callback for a page title when this entity is displayed.
 */
function smartling_entity_data_title($entity) {
  return t('smartling_entity_data (page_name=page_name)', array('@page_name' => $entity->page_name));
}

/**
 * Menu callback to display an entity.
 *
 * As we load the entity for display, we're responsible for invoking a number
 * of hooks in their proper order.
 *
 * @see hook_entity_prepare_view()
 * @see hook_entity_view()
 * @see hook_entity_view_alter()
 */
function smartling_entity_data_view($entity, $view_mode = 'full') {
  // Our entity type, for convenience.
  $entity_type = 'smartling_entity_data';
  // Start setting up the content.
  $entity->content = array(
    '#view_mode' => $view_mode,
  );
  // Build fields content - this is where the Field API really comes in to play.
  // The task has very little code here because it all gets taken care of by
  // field module.
  // field_attach_prepare_view() lets the fields load any data they need
  // before viewing.
  //field_attach_prepare_view($entity_type, array($entity->basic_id => $entity), $view_mode);
  // We call entity_prepare_view() so it can invoke hook_entity_prepare_view()
  // for us.
  //entity_prepare_view($entity_type, array($entity->basic_id => $entity));
  // Now field_attach_view() generates the content for the fields.
  //$entity->content += field_attach_view($entity_type, $entity, $view_mode);
  // OK, Field API done, now we can set up some of our own data.
  $entity->content['page_name'] = array(
    '#type' => 'item',
    '#title' => t('page_name'),
    '#markup' => $entity->page_name,
  );

  // Now to invoke some hooks. We need the language code for
  // hook_entity_view(), so let's get that.
  global $language;
  $langcode = $language->language;
  // And now invoke hook_entity_view().
  module_invoke_all('entity_view', $entity, $entity_type, $view_mode, $langcode);
  // Now invoke hook_entity_view_alter().
  drupal_alter(array('smartling_entity_data_view', 'entity_view'), $entity->content, $entity_type);

  // And finally return the content.
  return $entity->content;
}

/**
 * Provides a wrapper on the edit form to add a new entity.
 */
function smartling_entity_data_add() {
  // Create a basic entity structure to be used and passed to the validation
  // and submission functions.
  $entity = entity_get_controller('smartling_entity_data')->create();
  return drupal_get_form('smartling_entity_data_form', $entity);
}

/**
 * Form function to create an smartling_entity_data entity.
 */
function smartling_entity_data_form($form, &$form_state, $entity) {
  $form['eid'] = array(
    '#type' => 'textfield',
    '#title' => t('Item eid'),
    '#required' => TRUE,
    '#default_value' => $entity->eid,
  );
  $form['page_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Item page_name'),
    '#required' => TRUE,
    '#default_value' => $entity->page_name,
  );
  $form['locale_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Item locale_name'),
    '#required' => TRUE,
    '#default_value' => $entity->locale_name,
  );
  $form['progress'] = array(
    '#type' => 'textfield',
    '#title' => t('Item progress'),
    '#required' => TRUE,
    '#default_value' => $entity->progress,
  );
  $form['submitter'] = array(
    '#type' => 'textfield',
    '#title' => t('Item submitter'),
    '#required' => TRUE,
    '#default_value' => $entity->submitter,
  );
  $form['submission_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Item submission_date'),
    '#required' => TRUE,
    '#default_value' => (empty($entity->submission_date)) ? time() : $entity->submission_date,
  );
  $form['download'] = array(
    '#type' => 'textfield',
    '#title' => t('Item download'),
    '#required' => TRUE,
    '#default_value' => $entity->download,
  );

  $form['basic_entity'] = array(
    '#type' => 'value',
    '#value' => $entity,
  );

  //field_attach_form('smartling_entity_data', $entity, $form, $form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100,
  );
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => array('smartling_entity_data_edit_delete'),
    '#weight' => 200,
  );

  return $form;
}

/**
 * Validation handler for smartling_entity_data_add_form form.
 */
function smartling_entity_data_validate($form, &$form_state) {
  field_attach_form_validate('smartling_entity_data', $form_state['values']['basic_entity'], $form, $form_state);
}

/**
 * Form submit handler: submits basic_add_form information
 */
function smartling_entity_data_form_submit($form, &$form_state) {
  $entity = $form_state['values']['basic_entity'];
  $entity->eid = $form_state['values']['eid'];
  $entity->page_name = $form_state['values']['page_name'];
  $entity->locale_name = $form_state['values']['locale_name'];
  $entity->progress = $form_state['values']['progress'];
  $entity->submitter = $form_state['values']['submitter'];
  $entity->submission_date = $form_state['values']['submission_date'];
  $entity->download = $form_state['values']['download'];
  field_attach_submit('smartling_entity_data', $entity, $form, $form_state);
  $entity = smartling_entity_data_save($entity);
  $form_state['redirect'] = 'admin/config/regional/smartling/entity-data/manage';
}

/**
 * Form deletion handler.
 */
function smartling_entity_data_edit_delete($form, &$form_state) {
  $entity = $form_state['values']['basic_entity'];
  smartling_entity_data_delete($entity);
  drupal_set_message(t('The entity %page_name (ID %id) has been deleted', array('%page_name' => $entity->page_name, '%id' => $entity->eid))
  );
  $form_state['redirect'] = 'admin/config/regional/smartling/entity-data/';
}

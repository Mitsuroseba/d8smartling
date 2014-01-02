<?php

/**
 * @file
 */

/**
 * SmartlingEntityData class.
 */
class SmartlingEntityData extends Entity {

  protected function defaultLabel() {
    return $this->title;
  }

  protected function defaultUri() {
    return array('path' => 'admin/config/regional/smartling/entity-data/' . $this->identifier());
  }

}

/**
 * SmartlingEntityDataController class.
 */
class SmartlingEntityDataController extends EntityAPIController {

  public function create(array $values = array()) {
    $values += array(
      'rid' => '',
      'bundle' => '',
      'original_language' => '',
      'target_language' => '',
      'file_name' => '',
      'translated_file_name' => '',
      'progress' => '',
      'submitter' => '',
      'submission_date' => '',
      'download' => '',
      'status' => '',
    );

    return parent::create($values);
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $wrapper = entity_metadata_wrapper('smartling_entity_data', $entity);
    $content['author'] = array('#markup' => t('Created by: !author', array('!author' => $wrapper->uid->name->value(array('sanitize' => TRUE)))));

    // Make submitter themed like default fields.
    $content['submitter'] = array(
      '#theme' => 'field',
      '#weight' => 0,
      '#title' => t('Submitter'),
      '#access' => TRUE,
      '#label_display' => 'above',
      '#view_mode' => 'full',
      '#language' => LANGUAGE_NONE,
      '#field_name' => 'field_fake_submitter',
      '#field_type' => 'text',
      '#entity_type' => 'smartling_entity_data',
      '#items' => array(array('value' => $entity->submitter)),
      '#formatter' => 'text_default',
      0 => array('#markup' => check_plain($entity->submitter))
    );

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }

}

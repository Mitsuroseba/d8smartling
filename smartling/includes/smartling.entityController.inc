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

}

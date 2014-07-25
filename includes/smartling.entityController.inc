<?php

/**
 * @file
 * Smartling entity controller.
 */

/**
 * SmartlingEntityData class.
 */
class SmartlingEntityData extends Entity {

  protected $processor;

  /**
   * Get default label.
   *
   * @return string
   *   Return label.
   */
  protected function defaultLabel() {
    return $this->title;
  }

  /**
   * Get default uri string for smartling entity.
   *
   * @return string
   *   Return default uri string.
   */
  protected function defaultUri() {
    return array('path' => 'admin/config/regional/smartling/entity-data/' . $this->identifier());
  }

  protected function getProcessor() {
    if ($this->processor === NULL) {
      $log = smartling_log_get_handler();

      switch ($this->bundle) {
        case 'node':
          $this->processor = new \Drupal\smartling\Processors\NodeProcessor($this, $this->entity->language, $log, $this->rid);
          break;

        case 'taxonomy_term':
          $this->processor = new \Drupal\smartling\Processors\TaxonomyTermProcessor($this, $this->entity->language, $log, $this->rid);
          break;

        default:
          $this->processor = new \Drupal\smartling\Processors\BaseEntityProcessor($this, $this->entity->language, $log, $this->rid)
          break;
      }
    }

    return $this->processor;
  }

  public function importDrupalEntity($entity) {

  }

  public function exportToTranslation() {
    return $this->getProcessor()->exportContentToTranslation();
  }

  public function importTranslation($source, $type) {

    if (is_array($source)) {
      $this->getProcessor()->importSmartlingTranslation($source);
    }
  }

  public function syncTranslation($entityId = NULL) {

  }
}

/**
 * SmartlingEntityDataController class.
 */
class SmartlingEntityDataController extends EntityAPIController {

  /**
   * Create smartling entity object.
   *
   * @param array $values
   *   Entity values.
   *
   * @return object
   *   Return smartling entity object.
   */
  public function create(array $values = array()) {
    $values += array(
      'rid' => '',
      'entity_type' => '',
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
      'content_hash' => '',
    );

    return parent::create($values);
  }

}

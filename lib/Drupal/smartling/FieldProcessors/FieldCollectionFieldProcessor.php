<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TitlePropertyFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class FieldCollectionFieldProcessor extends BaseFieldProcessor {


  /**
   * Wrapper for Smartling settings storage.
   *
   * @todo avoid procedural code and inject storage to keep DI pattern.
   *
   * @return array()
   */
  protected function getTransletableFields() {
    //return smartling_settings_get_handler()->getFieldsSettings($this->entity->entity_type, $this->entity->bundle);
    return array('field_text1', 'field_some2');
  }

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();

    //return $entity_current_translatable_content;
    if (!empty($this->entity->{$this->fieldName}[$this->sourceLanguage])) {
      foreach ($this->entity->{$this->fieldName}[$this->sourceLanguage] as $delta => $value) {
        $fid = (int)$value['value'];
        $entity = field_collection_item_load($fid);

        foreach ($this->getTransletableFields() as $field_name) {
          /* @var $fieldProcessor \Drupal\smartling\FieldProcessors\BaseFieldProcessor */
          $fieldProcessor = drupal_container()->get('smartling.field_processor_factory')->getProcessor($field_name, $entity, 'field_collection_item', $this->smartling_entity, $this->targetLanguage);

          if ($fieldProcessor) {
            $data[$fid][$field_name] = $fieldProcessor->getSmartlingContent();
          }
        }
      }
    }

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function getDrupalContent() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartling_entity[$this->fieldName][$this->sourceLanguage] as $delta => $value) {
      $data = $value;
    }

    return $data;
  }

  public function fetchDataFromXML(\DomXpath $xpath) {

  }

}
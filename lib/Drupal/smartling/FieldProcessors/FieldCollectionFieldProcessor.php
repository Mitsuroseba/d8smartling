<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TitlePropertyFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class FieldCollectionFieldProcessor extends BaseFieldProcessor {

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();


    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $fid = (int)$value['value'];
        $entity = field_collection_item_load($fid);

        $wrapper = entity_metadata_wrapper('field_collection', $entity);
        $smartling_entity = smartling_entity_load_by_conditions(array(
          'rid' => $wrapper->getIdentifier(),
          'target_language' => $this->language,
        ));

        if (!$smartling_entity) {
          // @todo verify that entity has language property.
          $smartling_entity = smartling_create_from_entity($entity, 'field_collection', $entity->langcode, $this->language);
        }
        //$smartling_entity = smartling_create_from_entity($entity, $entity->type, $entity->language, $this->language);
        $processor = smartling_get_entity_processor($smartling_entity);
        $data[$delta] = $processor->exportContentToTranslation();


      }
    }

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function getDrupalContent() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartling_entity[$this->fieldName][$this->language] as $delta => $value) {
      $data = $value;
    }

    return $data;
  }

  public function fetchDataFromXML(\DomXpath $xpath) {

  }

}
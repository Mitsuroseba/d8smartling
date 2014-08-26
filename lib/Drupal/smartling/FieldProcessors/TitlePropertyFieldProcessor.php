<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TitlePropertyFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class TitlePropertyFieldProcessor extends BaseFieldProcessor {

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
//    $data = array();

//    if (!empty($this->entity->title)) {
//      $data[0] = $this->entity->label();
//    }

    return array(entity_label($this->entityType, $this->entity));
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
    $data = array();
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-0' . '"][1]')
      ->item(0);
    $quantity = $quantity_value->getAttribute('quantity');

    for ($i = 0; $i < $quantity; $i++) {
      $field = $xpath->query('//string[@id="' . $this->fieldName . '-' . $i . '"][1]')
        ->item(0);
      $data[$this->language][$i]['value'] = $this->processXMLContent((string) $field->nodeValue);
    }

    return $data;
  }

}
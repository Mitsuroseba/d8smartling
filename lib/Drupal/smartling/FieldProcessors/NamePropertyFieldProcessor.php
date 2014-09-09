<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TitlePropertyFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class NamePropertyFieldProcessor extends TitlePropertyFieldProcessor {

  protected $propertyName = 'name';

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    return $this->entity->{$this->propertyName};
  }

  /**
   * {@inheritdoc}
   */
  public function getDrupalContent() {
    $data = $this->entity->{$this->propertyName};

    foreach ($this->smartling_entity[$this->fieldName][$this->language] as $delta => $value) {
      $data = $value;
    }

    return $data;
  }

  public function fetchDataFromXML(\DomXpath $xpath) {
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-0' . '"][1]')
      ->item(0);

    if (!$quantity_value) {
      return NULL;
    }

    $field = $xpath->query('//string[@id="' . $this->fieldName . '-0"][1]')
      ->item(0);
    return $this->processXMLContent((string) $field->nodeValue);
  }

  public function setDrupalContentFromXML($xpath) {
    $this->entity->{$this->propertyName} = $this->fetchDataFromXML($xpath);
  }

}
<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TextFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class TextFieldProcessor extends BaseFieldProcessor {

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->sourceLanguage])) {
      foreach ($this->entity->{$this->fieldName}[$this->sourceLanguage] as $delta => $value) {
        $data[$delta] = $value['value'];
      }
    }

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function fetchDataFromXML(\DomXpath $xpath) {
    //@todo fetch format from xml as well.
    $data = array();
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-0' . '"][1]')
      ->item(0);

    if (!$quantity_value) {
      return NULL;
    }

    $quantity = $quantity_value->getAttribute('quantity');

    for ($i = 0; $i < $quantity; $i++) {
      $field = $xpath->query('//string[@id="' . $this->fieldName . '-' . $i . '"][1]')
        ->item(0);
      $data[$i]['value'] = $this->processXMLContent((string) $field->nodeValue);
    }

    return $data;
  }

}
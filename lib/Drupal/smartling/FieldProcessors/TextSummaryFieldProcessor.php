<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TextSummaryFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class TextSummaryFieldProcessor extends BaseFieldProcessor {

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $data[$delta]['body'] = $value['value'];
        $data[$delta]['summary'] = $value['summary'];
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
      $data[$this->language][$delta]['value'] = $value['body'];
      $data[$this->language][$delta]['summary'] = $value['summary'];
    }

    return $data;
  }

  //@todo fetch format from xml as well.
  /**
   * {@inheritdoc}
   */
  public function fetchDataFromXML(\DomXpath $xpath) {
    $data = array();
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-body-0' . '"][1]')
      ->item(0);
    $quantity = $quantity_value->getAttribute('quantity');

    for ($i = 0; $i < $quantity; $i++) {
      $bodyField = $xpath->query('//string[@id="' . $this->fieldName . '-body-' . $i . '"][1]')->item(0);
      $summaryField = $xpath->query('//string[@id="' . $this->fieldName . '-summary-' . $i . '"][1]')->item(0);

      $data[$this->language][$i]['value'] = $this->processXMLContent((string) $bodyField->nodeValue);
      $data[$this->language][$i]['summary'] = $this->processXMLContent((string) $summaryField->nodeValue);
    }

    return $data;
  }
}
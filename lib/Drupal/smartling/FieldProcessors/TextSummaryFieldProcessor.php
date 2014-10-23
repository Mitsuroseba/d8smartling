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

    if (!empty($this->entity->{$this->fieldName}[$this->sourceLanguage])) {
      foreach ($this->entity->{$this->fieldName}[$this->sourceLanguage] as $delta => $value) {
        $data[$delta]['body'] = $value['value'];
        $data[$delta]['summary'] = $value['summary'];
        $data[$delta]['format'] = $value['format'];
      }
    }

    return $data;
  }


  /**
   * {@inheritdoc}
   */
  public function fetchDataFromXML(\DomXpath $xpath) {
    $data = array();
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-body-0' . '"][1]')
      ->item(0);

    if (!$quantity_value) {
      return NULL;
    }

    $quantity = $quantity_value->getAttribute('quantity');
    $format = $quantity_value->getAttribute('format');

    for ($i = 0; $i < $quantity; $i++) {
      $bodyField = $xpath->query('//string[@id="' . $this->fieldName . '-body-' . $i . '"][1]')->item(0);
      $summaryField = $xpath->query('//string[@id="' . $this->fieldName . '-summary-' . $i . '"][1]')->item(0);

      $data[$i]['value'] = $this->processXMLContent((string) $bodyField->nodeValue);
      $data[$i]['summary'] = $this->processXMLContent((string) $summaryField->nodeValue);
      $data[$i]['format'] = $format;
      // @todo Copy fromat from the original field while xml file doesn't contain format
      // Otherwise you will get bug imediatelly with FullHtml fields
    }

    return $data;
  }
}
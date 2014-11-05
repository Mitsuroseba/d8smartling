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
      // @todo Copy fromat from the original field while xml file doesn't contain format
      // Otherwise you will get bug imediatelly with FullHtml fields
    }

    return $data;
  }

  public function putDataToXML($xml, $localize, $data) {
    // Field text.
    $quantity = count($data);
    foreach ($data as $key => $value) {
      $string = $this->buildXMLString($xml, $this->fieldName . '-' . $key, $key, $quantity, $value);
      $localize->appendChild($string);
    }
  }

  public function setDrupalContentFromXML($fieldValue) {
    if (is_array($fieldValue)) {
      $elem = current($fieldValue);
      if (isset($elem['value'])) {
        $this->entity->{$this->fieldName}[$this->targetLanguage] = $fieldValue;
      }
      else {
        foreach ($fieldValue as $delta => $val) {
          $this->entity->{$this->fieldName}[$this->targetLanguage][$delta] = array('value' => $val);
        }
      }
    }
    else {
      $this->entity->{$this->fieldName}[$this->targetLanguage] = array(array('value' => $fieldValue));
    }

  }
}
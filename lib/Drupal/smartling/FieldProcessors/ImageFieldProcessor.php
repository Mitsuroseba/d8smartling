<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\ImageFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class ImageFieldProcessor extends BaseFieldProcessor {

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->sourceLanguage])) {
      foreach ($this->entity->{$this->fieldName}[$this->sourceLanguage] as $delta => $value) {
        $data[$delta]['alt-img'] = $value['alt'];
        $data[$delta]['title-img'] = $value['title'];
        $data[$delta]['fid-img'] = $value['fid'];
      }
    }

    return $data;
  }

  //@todo fetch format from xml as well.
  /**
   * {@inheritdoc}
   */
  public function fetchDataFromXML(\DomXpath $xpath) {
    $data = array();
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-alt-img-0' . '"][1]')
      ->item(0);

    if (!$quantity_value) {
      return NULL;
    }

    $quantity = $quantity_value->getAttribute('quantity');

    for ($i = 0; $i < $quantity; $i++) {
      $altField = $xpath->query('//string[@id="' . $this->fieldName . '-alt-img-' . $i . '"][1]')->item(0);
      $titleField = $xpath->query('//string[@id="' . $this->fieldName . '-title-img-' . $i . '"][1]')->item(0);


      $fid = $altField->getAttribute('fid');
      $file_img = file_load($fid);

      if ($file_img) {
        $data[$i] = (array) $file_img;
        $data[$i]['alt'] = $this->processXMLContent((string) $altField->nodeValue);
        $data[$i]['title'] = $this->processXMLContent((string) $titleField->nodeValue);
      }
    }

    return $data;
  }

  public function putDataToXML($xml, $localize, $data) {
    $quantity = count($data);
    foreach ($data as $key => $value) {
      $string = $xml->createElement('string');
      $string_val = $xml->createTextNode($value['alt-img']);
      $string_attr = $xml->createAttribute('id');
      $string_attr->value = $this->fieldName . '-alt-img-' . $key;
      $string->appendChild($string_attr);
      $string->appendChild($string_val);
      // Set quantity.
      $string_attr = $xml->createAttribute('quantity');
      $string_attr->value = $quantity;
      $string->appendChild($string_attr);
      // Set image fid.
      $string_attr = $xml->createAttribute('fid');
      $string_attr->value = $value['fid-img'];
      $string->appendChild($string_attr);
      $localize->appendChild($string);

      $string = $xml->createElement('string');
      $string_val = $xml->createTextNode($value['title-img']);
      $string_attr = $xml->createAttribute('id');
      $string_attr->value = $this->fieldName . '-title-img-' . $key;
      $string->appendChild($string_attr);
      $string->appendChild($string_val);
      // Set quantity.
      $string_attr = $xml->createAttribute('quantity');
      $string_attr->value = $quantity;
      $string->appendChild($string_attr);
      // Set image fid.
      $string_attr = $xml->createAttribute('fid');
      $string_attr->value = $value['fid-img'];
      $string->appendChild($string_attr);
      $localize->appendChild($string);
    }
  }

}

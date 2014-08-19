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
  public function getSmartlingFormat() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $data[$delta]['alt-img'] = $value['alt'];
        $data[$delta]['title-img'] = $value['title'];
        $data[$delta]['fid-img'] = $value['fid'];
      }
    }

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function getDrupalFormat() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartlingData[$this->fieldName][$this->language] as $delta => $value) {
      $data[$this->language][$delta]['alt'] = $value['alt-img'];
      $data[$this->language][$delta]['title'] = $value['title-img'];
      $data[$this->language][$delta]['fid'] = $value['fid-img'];
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
    $quantity = $quantity_value->getAttribute('quantity');

    for ($i = 0; $i < $quantity; $i++) {
      $altField = $xpath->query('//string[@id="' . $this->fieldName . '-alt-img-' . $i . '"][1]')->item(0);
      $titleField = $xpath->query('//string[@id="' . $this->fieldName . '-title-img-' . $i . '"][1]')->item(0);


      $fid = $altField->getAttribute('fid');
      $file_img = file_load($fid);

      if ($file_img) {
        $data[$this->language][$i] = (array) $file_img;
        $data[$this->language][$i]['alt'] = $this->processXMLContent((string) $altField->nodeValue);
        $data[$this->language][$i]['title'] = $this->processXMLContent((string) $titleField->nodeValue);
      }
    }

    return $data;
  }

}

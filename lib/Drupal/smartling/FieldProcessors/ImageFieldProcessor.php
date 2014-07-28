<?php

namespace Drupal\smartling\FieldProcessors;

class ImageFieldProcessor extends BaseFieldProcessor {
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

  public function getDrupalFormat() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartlingData[$this->fieldName][$this->language] as $delta => $value) {
      $data[$this->language][$delta]['alt'] = $value['alt-img'];
      $data[$this->language][$delta]['title'] = $value['title-img'];
      $data[$this->language][$delta]['fid'] = $value['fid-img'];
    }

    return $data;
  }
}

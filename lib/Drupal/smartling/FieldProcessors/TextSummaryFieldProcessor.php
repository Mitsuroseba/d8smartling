<?php

namespace Drupal\smartling\FieldProcessors;

class TextSummaryFieldProcessor extends BaseFieldProcessor {
  public function getSmartlingFormat() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $data[$delta]['body'] = $value['value'];
        $data[$delta]['summary'] = $value['summary'];
      }
    }

    return $data;
  }

  public function getDrupalFormat() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartlingData[$this->fieldName][$this->language] as $delta => $value) {
      $data[$this->language][$delta]['value'] = $value['body'];
      $data[$this->language][$delta]['summary'] = $value['summary'];
    }

    return $data;
  }
}
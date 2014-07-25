<?php

namespace Drupal\smartling\Processors;

abstract class BaseFieldProcessor {

  protected $entity;
  protected $language;
  protected $fieldName;

  protected $smartlingData;

  public function __construct($entity, $language, $field_name, $smartling_data = NULL) {
    $this->entity = $entity;
    $this->language = $language;
    $this->fieldName = $field_name;
    $this->smartlingData = $smartling_data;

    return $this;
  }

  public function setSmartlingData($smartling_data) {
    $this->smartlingData = $smartling_data;

    return $this;
  }

  abstract public function getSmartlingFormat();
  abstract public function getDrupalFormat();

}

class TextFieldProcessor extends BaseFieldProcessor {
  public function getSmartlingFormat() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $data[$this->language][$delta] = $value['value'];
      }
    }

    return $data;
  }

  public function getDrupalFormat() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartlingData[$this->fieldName][$this->language] as $delta => $value) {
      $data[$this->language][$delta]['value'] = $value;
    }

    return $data;
  }
}

class TextSummaryFieldProcessor extends BaseFieldProcessor {
  public function getSmartlingFormat() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $data[$this->language][$delta]['body'] = $value['value'];
        $data[$this->language][$delta]['summary'] = $value['summary'];
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

class ImageFieldProcessor extends BaseFieldProcessor {
  public function getSmartlingFormat() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->language])) {
      foreach ($this->entity->{$this->fieldName}[$this->language] as $delta => $value) {
        $data[$this->language][$delta]['alt-img'] = $value['alt'];
        $data[$this->language][$delta]['title-img'] = $value['title'];
        $data[$this->language][$delta]['fid-img'] = $value['fid'];
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

class TitlePropertyFieldProcessor extends BaseFieldProcessor {
  public function getSmartlingFormat() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName})) {
      $data[0] = $this->entity->label();
    }

    return $data;
  }

  public function getDrupalFormat() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartlingData[$this->fieldName][$this->language] as $delta => $value) {
      $data = $value;
    }

    return $data;
  }
}
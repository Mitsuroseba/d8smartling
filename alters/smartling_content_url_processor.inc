<?php
require_once(dirname(__FILE__) . '/smartling_content_processor_interface.inc');

class SmartlingUrlProcessor implements ISmartlingContentProcessor {
  protected $field_name;
  protected $entity;

  public function __construct($field_name, $entity) {
    $this->field_name = $field_name;
    $this->entity = $entity;
  }

  public function process(&$item, $context, $field_name, $entity) {
    $item[2] = str_replace('a', 'b', $item[2]);
  }
}
<?php
require_once(dirname(__FILE__) . '/smartling_content_processor_interface.inc');

/*
 * Demo url processor. No real value here for now.
 */
class SmartlingUrlProcessor implements ISmartlingContentProcessor {
  protected $field_name;
  protected $entity;

  public function process(&$item, $context, $lang, $field_name, $entity) {
    $item[2] = str_replace('a', 'b', $item[2]);
  }
}
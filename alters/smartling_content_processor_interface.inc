<?php

interface ISmartlingContentProcessor {
  public function process(&$item, $context, $field_name, $entity);
}
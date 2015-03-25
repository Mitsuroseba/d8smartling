<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Wrappers;

/**
 * Class EntityAPIWrapper.
 */
class FieldAPIWrapper {
  public function fieldLanguage($entity_type, $entity, $field_name = NULL, $langcode = NULL) {
    return field_language($entity_type, $entity, $field_name, $langcode);
  }

  public function fieldGetItems($entity_type, $entity, $field_name, $langcode = NULL) {
    return field_get_items($entity_type, $entity, $field_name, $langcode);
  }

  public function fieldInfoField($field_name) {
    return field_info_field($field_name);
  }
}

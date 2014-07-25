<?php

namespace Drupal\smartling\Processors;

class FieldProcessorFactory {
  protected static $fields_mapping = array(
    'text' => 'TextFieldProcessor',
    'text_long' => 'TextFieldProcessor',

    'text_with_summary' => 'TextSummaryFieldProcessor',

    'image' => 'ImageFieldProcessor',

    'title_property' => 'TitlePropertyFieldProcessor',
  );

  public static function getFieldsProcessorsMapping() {
    return self::$fields_mapping;
  }

  /**
   * @param $field_name
   * @param $entity
   * @return FieldProcessor
   */
  public static function getProcessor($field_name, $entity) {
    $field_info = field_info_field($field_name);
    $language = (smartling_field_is_translatable_by_field_name($field_name, $entity)) ? entity_language($entity->entityType(), $entity) : LANGUAGE_NONE;

    if (empty(self::$fields_mapping[$field_info['type']])) {
      $log = smartling_log_get_handler();
      $log->setMessage("Smartling didn't process content of field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    return new self::$fields_mapping[$field_info['type']]($entity, $language, $field_name);
  }
}
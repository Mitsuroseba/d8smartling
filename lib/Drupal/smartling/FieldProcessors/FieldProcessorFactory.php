<?php

namespace Drupal\smartling\FieldProcessors;

class FieldProcessorFactory {
  // @TODO remove this hardcode and figure out how to pass namespace to factory.
  protected static $namespace = 'Drupal\smartling\FieldProcessors\\';

  protected static $fields_mapping = array(
    'text' => 'TextFieldProcessor',
    'text_long' => 'TextFieldProcessor',

    'text_with_summary' => 'TextSummaryFieldProcessor',

    'image' => 'ImageFieldProcessor',

    'title_property' => 'TitlePropertyFieldProcessor',
    'title_property_field' => 'TitlePropertyFieldProcessor',
  );

  protected static $fake_fields = array(
    'title_property_field',
    'name_property_field',
    'description_property_field',
  );

  public static function getFieldsProcessorsMapping() {
    return self::$fields_mapping;
  }

  /**
   * @param $field_name
   * @param $entity
   * @return BaseFieldProcessor
   */
  public static function getProcessor($field_name, $entity_type, $entity) {
    $field_info = field_info_field($field_name);

    if ($field_info) {
      $type = $field_info['type'];
    }
    elseif (in_array($field_name, self::$fake_fields)) {
      $type = $field_name;
    }
    else {
      $log = smartling_log_get_handler();
      $log->setMessage("Smartling found unexisted field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    if (empty(self::$fields_mapping[$type])) {
      $log = smartling_log_get_handler();
      $log->setMessage("Smartling didn't process content of field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    $language = (smartling_field_is_translatable_by_field_name($field_name, $entity)) ? entity_language($entity->entityType(), $entity) : LANGUAGE_NONE;

    $class_name = self::$namespace . self::$fields_mapping[$type];

    return new $class_name($entity, $entity_type, $language, $field_name);
  }
}
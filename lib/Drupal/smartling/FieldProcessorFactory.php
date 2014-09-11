<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessor\FieldProcessorFactory.
 *
 * @todo move to Drupal\smartling namespace.
 */

namespace Drupal\smartling;

use Drupal\smartling\FieldProcessors\BaseFieldProcessor;

/**
 * Factory that creates field processor instances and contains mapping.
 *
 * @package Drupal\smartling\FieldProcessors
 */
class FieldProcessorFactory {

  protected $fieldMapping;

  public function __construct($field_mapping_new) {
    $this->fieldMapping = $field_mapping_new;
  }

  /**
   * Factory method for FieldProcessor instances.
   *
   * @param string$field_name
   * @param \stdClass $entity
   * @param string $entity_type
   * @param \stdClass $smartling_entity
   * @param string $target_language
   *
   * @return BaseFieldProcessor
   */
  public function getProcessor($field_name, $entity, $entity_type, $smartling_entity, $target_language) {
    $field_info = field_info_field($field_name);

    if ($field_info) {
      $type = $field_info['type'];
      // @todo we could get notice about invalid key here.
      $class_name = $this->fieldMapping['real'][$type];
    }
    elseif (isset($this->fieldMapping['fake'][$field_name])) {
      $type = $field_name;
      $class_name = $this->fieldMapping['fake'][$type];
    }
    else {
      $log = smartling_log_get_handler();
      $log->setMessage("Smartling found unexisted field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    if (!$class_name) {
      $log = smartling_log_get_handler();
      $log->setMessage("Smartling didn't process content of field - @field_name")
        ->setVariables(array('@field_name' => $field_name))
        ->setConsiderLog(FALSE)
        ->execute();

      return FALSE;
    }

    $source_language = (smartling_field_is_translatable_by_field_name($field_name, $entity_type)) ? entity_language($entity_type, $entity) : LANGUAGE_NONE;

    return new $class_name(
      $entity,
      $entity_type,
      $field_name,
      $smartling_entity,
      $source_language,
      $target_language
    );
  }

//  /**
//   * Mapping field_type => FieldProcessor.
//   *
//   * @var array
//   */
//  protected static $fields_mapping = array(
//    'text' => 'TextFieldProcessor',
//    'text_long' => 'TextFieldProcessor',
//
//    'text_with_summary' => 'TextSummaryFieldProcessor',
//
//    'image' => 'ImageFieldProcessor',
//
//    'title_property' => 'TitlePropertyFieldProcessor',
//    'title_property_field' => 'TitlePropertyFieldProcessor',
//    'description_property_field' => 'DescriptionPropertyFieldProcessor',
//    'name_property_field' => 'NamePropertyFieldProcessor',
//
//    'field_collection' => 'FieldCollectionFieldProcessor',
//  );
//
//  /**
//   * List of fake fields which should be processed in the separate way.
//   *
//   * @var array
//   */
//  protected static $fake_fields = array(
//    'title_property_field',
//    'name_property_field',
//    'description_property_field',
//  );
//
//  /**
//   * @param $field_name string
//   *   Field's machine name.
//   * @param $entity \stdClass
//   *   Drupal content entity.
//   *
//   * @return BaseFieldProcessor
//   *
//   * @todo remove procedural code or at least put into the separate method
//   * to allow unit testing.
//   */
//  public static function getProcessor($field_name, $entity, $entity_type, $smartling_entity, $target_language) {
//    $field_info = field_info_field($field_name);
//
//    if ($field_info) {
//      $type = $field_info['type'];
//    }
//    elseif (in_array($field_name, self::$fake_fields)) {
//      $type = $field_name;
//    }
//    else {
//      $log = smartling_log_get_handler();
//      $log->setMessage("Smartling found unexisted field - @field_name")
//        ->setVariables(array('@field_name' => $field_name))
//        ->setConsiderLog(FALSE)
//        ->execute();
//
//      return FALSE;
//    }
//
//    if (empty(self::$fields_mapping[$type])) {
//      $log = smartling_log_get_handler();
//      $log->setMessage("Smartling didn't process content of field - @field_name")
//        ->setVariables(array('@field_name' => $field_name))
//        ->setConsiderLog(FALSE)
//        ->execute();
//
//      return FALSE;
//    }
//
//    $source_language = (smartling_field_is_translatable_by_field_name($field_name, $entity_type)) ? entity_language($entity_type, $entity) : LANGUAGE_NONE;
//
//    $class_name = self::$namespace . self::$fields_mapping[$type];
//
//    return new $class_name(
//      $entity,
//      $entity_type,
//      $field_name,
//      $smartling_entity,
//      $source_language,
//      $target_language
//    );
//  }
}
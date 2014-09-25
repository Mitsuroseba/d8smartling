<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\BaseFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

/**
 * Handle business logic for different drupal field types.
 *
 * @package Drupal\smartling\FieldProcessors
 */
abstract class BaseFieldProcessor {

  protected $entityType;
  protected $entity;
  protected $sourceLanguage;
  protected $targetLanguage;
  protected $fieldName;

  protected $smartling_entity;

  public function __construct($entity, $entity_type, $field_name, $smartling_data, $source_language, $target_language) {
    $this->entity = $entity;
    $this->entityType = $entity_type;
    $this->sourceLanguage = $source_language;
    $this->targetLanguage = $target_language;
    $this->fieldName = $field_name;
    $this->smartling_entity = $smartling_data;

    return $this;
  }

  public function setSmartlingEntity($smartling_data) {
    $this->smartling_entity = $smartling_data;

    return $this;
  }

  /**
   * Runs specific smartling alters.
   *
   * @param $value string
   * @param bool $reset
   *
   * @see \Drupal\smartling\Alters\*
   *
   * @return string
   */
  public function processXMLContent($value, $reset = FALSE) {
    $handlers = & drupal_static(__FUNCTION__);
    if (!isset($actions) || $reset) {
      $handlers = module_invoke_all('smartling_data_processor_info');
      drupal_alter('smartling_data_processor_info', $handlers);
    }

    foreach ($handlers as $parser => $processors) {
      if (!class_exists($parser)) {
        continue;
      }

      $processors_objs = array();
      foreach ($processors as $proc) {
        if (class_exists($proc) && in_array('SmartlingContentProcessorInterface', class_implements($proc))) {
          $processors_objs[] = new $proc();
        }
      }

      if (!empty($processors_objs)) {
        $parser = new $parser($processors_objs);
        $value = $parser->parse($value, $this->sourceLanguage, $this->fieldName, $this->entity);
      }
    }

    return $value;
  }

  /**
   * Converts drupal field format to smartling data.
   *
   * @return array
   *   Drupal field structure under language key ready to be put into drupal content entity.
   */
  abstract public function getSmartlingContent();

  /**
   * Converts smartling data field format to drupal.
   *
   * @deprecated will be remove as unused method.
   *
   * @return array
   *   Drupal field structure under language key ready to be put into smartling entity.
   */
  abstract public function getDrupalContent();

  /**
   * Fetch translation data from xml based on structure for particular field.
   *
   * @param \DomXpath $xpath
   *
   * @return array
   *   Drupal field structure under language key ready to be put into smartling entity.
   */
  abstract public function fetchDataFromXML(\DomXpath $xpath);

  public function setDrupalContentFromXML($xpath) {
    $this->entity->{$this->fieldName}[$this->targetLanguage] = $this->fetchDataFromXML($xpath);
  }

  /**
   * Prepare default field data for translatable field before applying new translation.
   *
   * @param array $fieldData
   *   Field data under language key.
   *   Array(
   *     $delta => array(
   *       'value' => $value,
   *     )
   *   )
   *
   * @return array
   */
  public function prepareBeforeDownload(array $fieldData) {
    return $fieldData;
  }

  public function cleanBeforeClone($field_name, $entity) {
    return NULL;
  }
}

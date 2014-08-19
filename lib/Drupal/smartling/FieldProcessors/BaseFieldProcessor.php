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
  protected $language;
  protected $fieldName;

  protected $smartlingData;

  public function __construct($entity, $entity_type, $language, $field_name, $smartling_data = NULL) {
    $this->entity = $entity;
    $this->entityType = $entity_type;
    $this->language = $language;
    $this->fieldName = $field_name;
    $this->smartlingData = $smartling_data;

    return $this;
  }

  public function setSmartlingData($smartling_data) {
    $this->smartlingData = $smartling_data;

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
        $value = $parser->parse($value, $this->language, $this->fieldName, $this->entity);
      }
    }

    return $value;
  }

  /**
   * Converts drupal field format to smartling data.
   *
   * @return array
   *   Drupal field structure ready to be put into drupal content entity.
   */
  abstract public function getSmartlingFormat();

  /**
   * Converts smartling data field format to drupal.
   *
   * @return array
   *   Drupal field structure ready to be put into smartling entity.
   */
  abstract public function getDrupalFormat();

  /**
   * Fetch translation data from xml based on structure for particular field.
   *
   * @param \DomXpath $xpath
   *
   * @return array
   *   Drupal field structure ready to be put into smartling entity.
   */
  public function fetchDataFromXML(\DomXpath $xpath) {
    //@todo fetch format from xml as well.
    $data = array();
    $quantity_value = $xpath->query('//string[@id="' . $this->fieldName . '-0' . '"][1]')
      ->item(0);
    $quantity = $quantity_value->getAttribute('quantity');

    for ($i = 0; $i < $quantity; $i++) {
      $field = $xpath->query('//string[@id="' . $this->fieldName . '-' . $i . '"][1]')
        ->item(0);
      $data[$this->language][$i]['value'] = $this->processXMLContent((string) $field->nodeValue);
    }

    return $data;
  }

}

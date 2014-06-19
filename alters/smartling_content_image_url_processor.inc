<?php

/**
 * @file
 * Class SmartlingUrlProcessor.
 */

require_once dirname(__FILE__) . '/smartling_content_processor_interface.inc';

/**
 * Demo url processor. No real value here for now.
 */
class SmartlingUrlProcessor implements SmartlingIContentProcessorInterface {

  protected $fieldName;
  protected $entity;

  /**
   * Process.
   *
   * @param array $item
   *   Item.
   * @param array $context
   *   Context.
   * @param string $lang
   *   Locale in drupal format (ru, en).
   * @param string $field_name
   *   Field name.
   * @param object $entity
   *   Entity object.
   */
  public function process(array &$item, array $context, $lang, $field_name, $entity) {
    $item[2] = str_replace('a', 'b', $item[2]);
  }

}

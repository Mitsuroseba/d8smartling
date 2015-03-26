<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Wrappers;

/**
 * Class SmartlingUtils.
 */
class SmartlingUtils {
  protected $entity_api_wrapper;
  protected $field_api_wrapper;

  public function __construct($entity_api_wrapper, $field_api_wrapper) {
    $this->entity_api_wrapper = $entity_api_wrapper;
    $this->field_api_wrapper = $field_api_wrapper;
  }

  public function nodesMethod($bundle) {
    return smartling_nodes_method($bundle);
  }

  /**
   * Checks translatable field by field name.
   *
   * @param string $field_name
   *   Field name.
   *
   * @param string $entity_type
   *   Entity type machine name.
   * @return bool
   *   Return TRUE if field is translatable.
   */
  public function fieldIsTranslatable($field_name, $entity_type) {
    $field = $this->field_api_wrapper->fieldInfoField($field_name);
    return $this->field_api_wrapper->fieldIsTranslatable($entity_type, $field);
  }
}

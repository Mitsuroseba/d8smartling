<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Wrappers;

/**
 * Class DrupalAPIWrapper.
 */
class DrupalAPIWrapper {
  public function getDefaultLanguage() {
    return language_default()->language;
  }

  /*
   * A wrapper for Drupal drupal_alter function
   */
  public function alter($hook_name, &$handlers) {
    drupal_alter($hook_name, $handlers);
  }

  public function &drupalStatic($name, $default_value = NULL, $reset = FALSE) {
    return drupal_static($name, $default_value, $reset);
  }
}

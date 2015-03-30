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

  public function moduleInvokeAll($hook) {
    return module_invoke_all($hook);
  }

  public function &drupalStatic($name, $default_value = NULL, $reset = FALSE) {
    return drupal_static($name, $default_value, $reset);
  }

  public function drupalRealpath($uri) {
    return drupal_realpath($uri);
  }

  public function fileLoad($fid) {
    return file_load($fid);
  }

  public function elementChildren(&$elements, $sort = FALSE) {
    return element_children($elements, $sort);
  }
}

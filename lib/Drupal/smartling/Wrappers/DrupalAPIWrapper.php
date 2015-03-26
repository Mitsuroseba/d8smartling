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
  protected function alter($hook_name, &$handlers) {
    drupal_alter($hook_name, $handlers);
  }
}

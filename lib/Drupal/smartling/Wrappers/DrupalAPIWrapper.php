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
}

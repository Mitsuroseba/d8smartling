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
  public function nodesMethod($bundle) {
    return smartling_nodes_method($bundle);
  }
}

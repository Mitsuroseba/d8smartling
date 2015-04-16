<?php

/**
 * @file
 * Contains Drupal\smartling\Processors\EntityProcessorInterface.
 */

namespace Drupal\smartling\Processors;

interface EntityProcessorInterface {
  public function downloadTranslation();
  public function updateEntityFromXML();
  public function exportContentForTranslation();
  public function getTranslatableContent();
}
<?php

namespace Drupal\smartling\Convectors;

interface ConverterInterface {
  public function import();
  public function export();
}
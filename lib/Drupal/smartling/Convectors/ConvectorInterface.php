<?php

namespace Drupal\smartling\Convectors;

interface ConvectorInterface {
  public function import();
  public function export();
}
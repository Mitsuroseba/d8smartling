<?php

/**
 * @file
 * Definition of smartling handler filter entity status.
 */

/**
 * Filter by smartling status.
 *
 * @ingroup views_filter_handlers
 */
class SmartlingHandlerFilterEntityStatus extends views_handler_filter_in_operator {

  /**
   * Get value options.
   */
  public function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Smartling status');
      $options = array(
        0 => 'In queue',
        1 => 'In translate',
        2 => 'Translated',
        3 => 'Changed',
        4 => 'Failed',
      );
      $this->value_options = $options;
    }
  }

}

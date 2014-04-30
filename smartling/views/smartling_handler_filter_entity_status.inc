<?php

/**
 * @file
 * Definition of smartling_handler_filter_entity_status.
 */

/**
 * Filter by smartling status.
 *
 * @ingroup views_filter_handlers
 */
class smartling_handler_filter_entity_status extends views_handler_filter_in_operator {

  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Smartling status');
      $options = array(
        0 => 'In queue',
        1 => 'In translate',
        2 => 'Translated',
        3 => 'Chaged',
      );
      $this->value_options = $options;
    }
  }

}

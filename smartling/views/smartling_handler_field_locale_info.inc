<?php

/**
 * @file
 * Handler code.
 */

/**
 * Handler for field locale_info.
 */
class smartling_handler_field_locale_info extends views_handler_field {

  function render($values) {
    $result = smartling_entity_load_all_by_conditions(array('rid' => $values->nid, 'bundle' => $values->node_type));
    if ($result) {
      foreach ($result as $value) {
        $langs[] = smartling_get_full_language_name($value->target_language);
      }
      return implode(', ', $langs);
    }
    else {
      return '';
    }
  }

}

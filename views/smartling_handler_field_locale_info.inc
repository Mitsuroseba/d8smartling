<?php

/**
 * @file
 * Handler code.
 */

/**
 * Handler for field locale_info.
 */
class SmartlingHandlerFieldLocaleInfo extends views_handler_field {

  /**
   * Render.
   *
   * @param object $values
   *   Entity.
   *
   * @return string
   *   Return locale value.
   */
  public function render($values) {
    if (isset($values->nid)) {
      $result = smartling_entity_load_all_by_conditions(array('rid' => $values->nid, 'entity_type' => 'node'));
    }
    elseif (isset($values->tid)) {
      $result = smartling_entity_load_all_by_conditions(array('rid' => $values->tid, 'entity_type' => 'taxonomy_term'));
    }
    elseif (isset($values->cid)) {
      $result = smartling_entity_load_all_by_conditions(array('rid' => $values->cid, 'entity_type' => 'comment'));
    }
    elseif (isset($values->uid)) {
      $result = smartling_entity_load_all_by_conditions(array('rid' => $values->uid, 'entity_type' => 'user'));
    }
    elseif (isset($values->smartling_interface_entity_textgroup)) {
      $result = smartling_entity_load_all_by_conditions(array('entity_type' => 'smartling_interface_entity', 'bundle' => $values->smartling_interface_entity_textgroup));
    }
    else {
      $result = FALSE;
    }
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

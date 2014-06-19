<?php

/**
 * @file
 * Handler code.
 */

/**
 * Handler for field target_language.
 */
class SmartlingHandlerFieldTargetLanguage extends views_handler_field {

  /**
   * Render.
   *
   * @param object $values
   *   Smartling entity.
   *
   * @return string
   *   Return target locale.
   */
  public function render($values) {
    $result = '';

    $lang_name = smartling_get_full_language_name($values->smartling_entity_data_target_language);

    if (is_null($lang_name)) {
      $result = t('Undefined name');
    }
    else {
      $result = $lang_name;
    }

    return $result;
  }

}

<?php

/**
 * @file
 * Handler code.
 */

/**
 * Handler for field locale_info.
 */
class smartling_handler_field_smartling_operations extends views_handler_field {

  function render($values) {
    $output = '';

    $entity_type = smartling_get_entity_type_by_bundle($values->smartling_entity_data_bundle);
    $rid = $values->smartling_entity_data_rid;
    $eid = $values->eid;
    $d_locale = smartling_convert_locale_drupal_to_smartling($values->smartling_entity_data_target_language, TRUE);
    $destination = 'admin/content/smartling';
    if (arg(3) == 'report-in-progress') {
      $destination = $destination . '/report-in-progress';
    }
    $options = array('query' => array('destination' => $destination));

    switch ($entity_type) {
      case 'node':
        $output = l(t('Edit'), $d_locale . '/node/' . $rid . '/edit/' . $d_locale) . ' | '
            . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      case 'taxonomy_term':
        $output = l(t('Edit'), $d_locale . '/taxonomy/term/' . $rid . '/edit/' . $d_locale) . ' | '
            . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      case 'comment':
        $output = l(t('Edit'), $d_locale . '/comment/' . $rid . '/edit/' . $d_locale) . ' | '
            . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      case 'user':
        $output = l(t('Edit'), $d_locale . '/user/' . $rid . '/edit/' . $d_locale) . ' | '
            . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      default:
        $output = '-';
        break;
    }
    return $output;
  }

}

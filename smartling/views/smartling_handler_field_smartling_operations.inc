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

    $entity_type = $values->smartling_entity_data_entity_type;
    $rid = $values->smartling_entity_data_rid;
    $eid = $values->eid;
    $d_locale = $values->smartling_entity_data_target_language;
    $destination = 'admin/content/smartling';
    if (arg(3) == 'report-in-progress') {
      $destination = $destination . '/report-in-progress';
    }
    $options = array('query' => array('destination' => $destination));

    switch ($entity_type) {
      case 'node':
        $output = l(t('Edit'), 'node/' . $rid . '/edit/' . $d_locale) . ' | '
          . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      case 'taxonomy_term':
        $output = l(t('Edit'), 'taxonomy/term/' . $rid . '/edit/' . $d_locale) . ' | '
          . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      case 'comment':
        $output = l(t('Edit'), 'comment/' . $rid . '/edit/' . $d_locale) . ' | '
          . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      case 'user':
        $output = l(t('Edit'), 'user/' . $rid . '/edit/' . $d_locale) . ' | '
          . l(t('Download'), 'smartling/download/' . $rid . '/' . $eid, $options);
        break;

      default:
        $output = '-';
        break;
    }
    return $output;
  }

}

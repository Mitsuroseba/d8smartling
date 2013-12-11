<?php

/**
 * @file
 * Utils functions for Smartling module.
 */

/**
 * Returns whether the given node type has support for Smartling translations.
 *
 * @return
 *   Boolean value.
 */
function smartling_supported_type($type) {
  $smartling_supported_explicitly = variable_get('language_content_type_' . $type, NULL);

  if ($smartling_supported_explicitly == 2 || $smartling_supported_explicitly == SMARTLING_KEY) {
    $smartling_supported_explicitly = TRUE;
  }
  else {
    $smartling_supported_explicitly = FALSE;
  }

  return $smartling_supported_explicitly;
}

function smartling_nodes_method($type) {
  $nodes_method = variable_get('language_content_type_' . $type, NULL) == 2;
  return $nodes_method;
}

function smartling_fields_method($type) {
  $fields_method = variable_get('language_content_type_' . $type, NULL) == SMARTLING_KEY;
  return $fields_method;
}

/**
 * Returns node status for nodes method by node id.
 */
function smartling_nodes_method_node_status($nid) {
  $node = smartling_node_load_default($nid);
  if (isset($node->tnid) && $node->tnid != '0' && ($node->tnid != $node->nid)) {
    return SMARTLING_TRANSLATE_NODE;
  }
  elseif (isset($node->tnid) && $node->tnid != '0' && ($node->tnid == $node->nid)) {
    return SMARTLING_ORIGINAL_NODE;
  }
  elseif (isset($node->tnid) && $node->tnid == '0') {
    return SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE;
  }
}

/**
 * Smartling custom wrapper function for node_load
 *
 * This is needed because the Smartling module should translate only the most current revision of any node
 *
 * @param $nid entity_id of node to load
 *
 * @return $node most current revision of the node at $nid
 */
function smartling_node_load_default($nid, $vid = NULL, $reset = FALSE) {
  if ($vid == NULL) {
    $query = db_select('node_revision', 'nr')
        ->condition('nid', $nid, '=');
    $query->addExpression('MAX(vid)', 'max_vid');
    $vid = $query->execute()->fetchField();
  }

  $node = node_load($nid, $vid, $reset);
  return $node;
}

/**
 * Checks any required configuration parameters are missing.
 *
 * @return string|boolean
 */
function smartling_is_configured() {
  $required_variables = array(
    'smartling_key',
    'smartling_project_id',
    'smartling_mode',
  );
  foreach ($required_variables as $required_variable) {
    $val = variable_get($required_variable, NULL);
    if (empty($val)) {
      if ($required_variable == 'smartling_key') {
        return FALSE;
      }
      elseif ($required_variable == 'smartling_project_id') {
        return FALSE;
      }
      elseif ($required_variable == 'smartling_mode') {
        return FALSE;
      }
    }
  }
  return TRUE; // all required configuration variables are set
}

/**
 * Checks smartling_translate_fields variable and check settings for content type.
 */
function smartling_translate_fields_is_set($type = FALSE) {
  $val = variable_get('smartling_translate_fields', NULL);

  if (is_null($val) || empty($val)) {
    $result = FALSE;
  }
  else {
    if ($type !== FALSE) {
      if (isset($val[$type])) {
        $result = TRUE;
      }
      else {
        $result = FALSE;
      }
    }
    else {
      $result = TRUE;
    }
  }
  if (!$result) {
    drupal_set_message('Please follow the page <a href="/admin/config/regional/smartling">"Smartling settings"</a> to setup Smartling configuration.', 'warning');
  }

  return $result;
}

function smartling_get_translatable_field_types() {
  // What types of fields DO we translate?
  $included_fields = array('text', 'text_long', 'text_textfield', 'text_textarea', 'text_textarea_with_summary');
  /* uncomment after more tests
    $included_fields = array('text', 'text_long', 'text_textfield', 'text_textarea', 'text_textarea_with_summary', 'field_collection_embed');
    if (module_exists('link')) {
    $included_fields[] = 'link_field';
    }
   */
  return $included_fields;
}

/**
 * Content node types linked to 'translatable' fields.
 */
function smartling_translatable_node_types() {
  $types = array();

  $fields = smartling_translatable_node_field_details();
  foreach ($fields as $field) {
    foreach ($field['bundles'] as $bundle) {
      $types[$bundle] = $bundle;
    }
  }

  if (count($types) > 0) {
    $types = array_keys($types);
  }

  return $types;
}

/**
 * Goes though ALL the fields in the system and gets the details about the ones that are marked 'translatable'.
 */
function smartling_translatable_node_field_details() {
  $fields = field_info_fields();
  $translatable_fields = array();

  foreach ($fields as $field_id => $field) {
    foreach ($field['bundles'] as $type => $instance) {
      if (field_is_translatable($type, $field)) {
        //echo '<br>Translatable: YES!' ;
        $field_db_table = array_keys($field['storage']['details']['sql']['FIELD_LOAD_CURRENT']);
        $field_db_table = array_shift($field_db_table);
        $translatable_fields[] = array(
          'entity_type' => $type,
          'machine_name' => $field['field_name'],
          'db_table' => $field_db_table,
          'bundles' => $field['bundles'][$type],
        );
      }
    }
  }

  return $translatable_fields;
}

/**
 *
 * @param type $nid
 * @return type array
 */
function smartling_node_content_to_translate($nid, $nodes_method = FALSE) {

  $node_current_translatable_content = array();
  $node = smartling_node_load_default($nid);

  $translate = variable_get('smartling_translate_fields', array());

  if (!empty($translate)) {
    foreach ($translate as $type => $field_names) {

      if ($node->type === $type) {
        foreach ($field_names as $field_name) {
          if ($field_name == 'node_title') {
            $node_current_translatable_content[$field_name] = $node->title;
          }
          else {

            if (isset($node->{$field_name}[$node->language][0]['value'])) {

              if (count($node->{$field_name}[$node->language]) == 1) {

                if (!isset($node->{$field_name}[$node->language][0]['summary'])) {
                  $node_current_translatable_content[$field_name] = $node->{$field_name}[$node->language][0]['value'];
                }
                else {
                  $node_current_translatable_content[$field_name]['body'] = $node->{$field_name}[$node->language][0]['value'];
                  $node_current_translatable_content[$field_name]['summary'] = $node->{$field_name}[$node->language][0]['summary'];
                }
              }
              elseif (count($node->{$field_name}[$node->language]) > 1) {

                if (!isset($node->{$field_name}[$node->language][0]['summary'])) {
                  foreach ($node->{$field_name}[$node->language] as $key => $item) {
                    $node_current_translatable_content[$field_name][$key] = $item['value'];
                  }
                }
                else {
                  foreach ($node->{$field_name}[$node->language] as $key => $item) {
                    $node_current_translatable_content[$field_name][$key]['body'] = $item['value'];
                    $node_current_translatable_content[$field_name][$key]['summary'] = $item['summary'];
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return $node_current_translatable_content;
}

/**
 *
 * @param type $file_name
 * @param type $nid
 */
function smartling_entity_data_filename_update($file_name, $nid, $s_locale, $translated_file = FALSE) {
  if (!$translated_file) {
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'target_language' => $s_locale));
    $entity_data->file_name = $file_name;
    smartling_entity_data_save($entity_data);
  }
  else {
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $nid, 'target_language' => $s_locale));
    $entity_data->translated_file_name = $file_name;
    smartling_entity_data_save($entity_data);
  }
}

function smartling_get_full_language_name($s_locale) {
  $locales = array(
    'en' => t('English'),
    'es' => t('Spanish (International)'),
    'nl-NL' => t('Dutch (Netherlands)'),
    'ru-RU' => t('Russian (Russia)'),
    'zh-CN' => t('Chinese (Simplified)'),
  );

  if (isset($locales[$s_locale])) {
    return $locales[$s_locale];
  }
  else {
    return NULL;
  }
}

function smartling_convert_locale_drupal_to_smatrtling($locale, $reverse = FALSE) {
  $locales = array(
    'aa' => 'aa-DJ',
    'ab' => 'ab-GE',
    'af' => 'af-ZA',
    'ak' => 'ak-GH',
    'am' => 'am-ET',
    'apa' => 'apa-US',
    'ar' => 'ar',
    'as' => 'as-IN',
    'ast' => 'ast-ES',
    'ay' => 'ay-BO',
    'az' => 'az-AZ',
    'ba' => 'ba-RU',
    'be' => 'be-BY',
    'bg' => 'bg-BG',
    'bi' => 'bi-VU',
    'bik' => 'bik-PH',
    'bm' => 'bm-ML',
    'bn' => 'bn-BD',
    'bo' => 'bo-CN',
    'br' => 'br-FR',
    'bs' => 'bs-BA',
    'ca' => 'ca-ES',
    'ce' => 'ce-RU',
    'ceb' => 'ceb-PH',
    'ch' => 'ch-GU',
    'chr' => 'chr-US',
    'chy' => 'chy-US',
    'co' => 'co-FR',
    'cpe' => 'cpe-US',
    'cpf' => 'cpf-MU',
    'cpp' => 'cpp-BR',
    'cs' => 'cs-CZ',
    'cy' => 'cy-GB',
    'da' => 'da-DK',
    'de' => 'de-DE',
    'dik' => 'dik-SD',
    'dv' => 'dv-MV',
    'dz' => 'dz-BT',
    'ee' => 'ee-GH',
    'efi' => 'efi-NG',
    'el' => 'el-GR',
    'en' => 'en-US',
    'en-gb' => 'en-GB',
    'eo' => 'eo-FR',
    'es' => 'es-ES',
    'es-cl' => 'es-CL',
    'es-co' => 'es-CO',
    'es-ar' => 'es-AR',
    'es-ve' => 'es-VE',
    'et' => 'et-EE',
    'eu' => 'eu-ES',
    'fa' => 'fa-IR',
    'fat' => 'fat-GH',
    'fi' => 'fi-FI',
    'fj' => 'fj-FJ',
    'fon' => 'fon-BJ',
    'fr' => 'fr-FR',
    'ga' => 'ga-IE',
    'gaa' => 'gaa-GH',
    'gbz' => 'gbz-IR',
    'gd' => 'gd-GB',
    'gil' => 'gil-KI',
    'gl' => 'gl-ES',
    'gn' => 'gn-BO',
    'gu' => 'gu-IN',
    'ha' => 'ha-NG',
    'haw' => 'haw-US',
    'he' => 'he-IL',
    'hi' => 'hi-IN',
    'hil' => 'hil-PH',
    'hmn' => 'hmn-LA',
    'hr' => 'hr-HR',
    'ht' => 'ht-HT',
    'hu' => 'hu-HU',
    'hy' => 'hy-AM',
    'id' => 'id-ID',
    'ig' => 'ig-NG',
    'Ilo' => 'Ilo-PH',
    'is' => 'is-IS',
    'it' => 'it-IT',
    'ja' => 'ja-JP',
    'jv' => 'jv-ID',
    'ka' => 'ka-GE',
    'kek' => 'kek-GT',
    'kg' => 'kg-CD',
    'ki' => 'kik-KE',
    'kin' => 'kin-RW',
    'kj' => 'kj-AO',
    'kk' => 'kk-KZ',
    'km' => 'km-KH',
    'kn' => 'kn-IN',
    'ko' => 'ko-KR',
    'kos' => 'kos-FM',
    'ks' => 'ks-IN',
    'ku' => 'ku-IQ',
    'kw' => 'kw-GB',
    'ky' => 'ky-KG',
    'la' => 'la-VA',
    'lb' => 'lb-LU',
    'lg' => 'lg-UG',
    'ln' => 'ln-CD',
    'lo' => 'lo-LA',
    'lt' => 'lt-LT',
    'lu' => 'lu-CD',
    'lv' => 'lv-LV',
    'mg' => 'mg-MG',
    'mh' => 'mh-MH',
    'mi' => 'mi-NZ',
    'mk' => 'mk-MK',
    'ml' => 'ml-IN',
    'mn' => 'mn-MN',
    'mo' => 'mo-MD',
    'mr' => 'mr-IN',
    'ms' => 'ms-MY',
    'mt' => 'mt-MT',
    'my' => 'my-MM',
    'na' => 'na-NR',
    'nb' => 'nb-NO',
    'nd' => 'nd-ZW',
    'ne' => 'ne-NP',
    'ng' => 'ng-NA',
    'niu' => 'niu-NU',
    'nl' => 'nl-NL',
    'nn' => 'nn-NO',
    'no' => 'no-NO',
    'nr' => 'nr-ZA',
    'nso' => 'nso-ZA',
    'nv' => 'nv-US',
    'ny' => 'ny-MW',
    'om' => 'om-ET',
    'or' => 'or-IN',
    'pa' => 'pa-IN',
    'pag' => 'pag-PH',
    'pap' => 'pap-AN',
    'pau' => 'pau-PW',
    'pl' => 'pl-PL',
    'ps' => 'ps-AF',
    //'pt' => 'pt-PT',
    'pt-pt' => 'pt-PT',
    'pt-br' => 'pt-BR',
    'qu' => 'qu-BO',
    'rar' => 'rar-CK',
    'rn' => 'rn-BI',
    'ro' => 'ro-RO',
    'ru' => 'ru-RU',
    'sa' => 'sa-IN',
    'sc' => 'sc-IT',
    'scn' => 'scn-IT',
    'sd' => 'sd-PK',
    'sg' => 'sg-CF',
    'si' => 'si-LK',
    'sk' => 'sk-SK',
    'sl' => 'sl-SI',
    'sm' => 'sm-WS',
    'sn' => 'sn-ZW',
    'so' => 'so-SO',
    'sq' => 'sq-SQ',
    'sr' => 'sr-CS',
    'ss' => 'ss-SZ',
    'st' => 'st-LS',
    'su' => 'su-ID',
    'sv' => 'sv-SE',
    'sw' => 'sw-TZ',
    'ta' => 'ta-IN',
    'te' => 'te-IN',
    'tg' => 'tg-TJ',
    'th' => 'th-TH',
    'ti' => 'ti-ER',
    'tk' => 'tk-TM',
    'fil' => 'tl-PH',
    //'tl' => 'tl-PH', // philipines
    'tn' => 'tn-BW',
    'to' => 'to-TO',
    'tpi' => 'tpi-PG',
    'tr' => 'tr-TR',
    'ts' => 'ts-ZA',
    'tum' => 'tum-MW',
    'tvl' => 'tvl-TV',
    'tw' => 'tw-GH',
    'ty' => 'ty-PF',
    'ug' => 'ug-CN',
    'uk' => 'uk-UA',
    'um' => 'um-AO',
    'ur' => 'ur-PK',
    'uz' => 'uz-UZ',
    've' => 've-ZA',
    'vi' => 'vi-VN',
    'war' => 'war-PH',
    'wo' => 'wo-SN',
    'xh' => 'xh-ZA',
    'yap' => 'yap-FM',
    'yi' => 'yi-IL',
    'yo' => 'yo-NG',
    //'zh' => 'zh-TW',
    'zh-hans' => 'zh-CN',
    'zh-hant' => 'zh-TW',
    'zu' => 'zu-ZA',
  );
  if (!$reverse) {
    if (isset($locales[$locale])) {
      return $locales[$locale];
    }
    else {
      return NULL;
    }
  }
  else {
    foreach ($locales as $key => $loc) {
      if ($locale == $loc) {
        return $key;
      }
    }
  }
}

/**
 *
 * @param type $xml_doc
 */
function smartling_save_xml($xml_doc, $nid = NULL, $s_locale = FALSE, $file_name = NULL, $translated_file = FALSE) {
  $directory = SMARTLING_DIRECTORY;
  if (is_null($file_name)) {
    $file_name = 'smartling_translate_' . $nid . '.xml';
  }
  $file_name = file_munge_filename(preg_replace('@^.*/@', '', $file_name), '', TRUE);

  if ($translated_file !== FALSE) {
    $translated_file = file_munge_filename(preg_replace('@^.*/@', '', $translated_file), '', TRUE);
  }

  $path = $directory . '/' . $file_name;

  if (is_dir($directory) && is_writable($directory)) {
    $xml_doc->save($path);

    if ($translated_file == FALSE) {
      smartling_watchdog('Smartling save original xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
    }
    if ($translated_file == TRUE) {
      smartling_watchdog('Smartling save translated xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
    }

    if (!is_null($nid)) {
      smartling_entity_data_filename_update($file_name, $nid, $s_locale, $translated_file);
    }
    return $xml_doc;
  }
  elseif (drupal_mkdir($directory) && drupal_chmod($directory)) {
    $xml_doc->save($path);

    if ($translated_file == FALSE) {
      smartling_watchdog('Smartling save original xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
    }
    if ($translated_file == TRUE) {
      smartling_watchdog('Smartling save translated xml file for node id - @nid. Locale: @locale', TRUE, array('@nid' => $nid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View file'), $path));
    }

    if (!is_null($nid)) {
      smartling_entity_data_filename_update($file_name, $nid, $s_locale, $translated_file);
    }
    return $xml_doc;
  }
  else {
    smartling_watchdog('Smartling file is not be saved because of the some errors. Filename: @file_name, related entity - @nid', FALSE, array('@file_name' => $file_name, '@nid' => $nid), WATCHDOG_ERROR);
    drupal_set_message(t('File is not be saved because of the some errors.'), 'error');
    return NULL;
  }
}

/**
 *
 * @param type $nid
 * @return boolean
 */
function smartling_build_xml_by_node_id($rid = NULL, $s_locale, $title = '', $nodes_method = FALSE) {
  if (!is_null($rid)) {
    $node_content_to_translate = smartling_node_content_to_translate($rid, $nodes_method);
    $xml = new DOMDocument('1.0', 'UTF-8');

    $xml->appendChild($xml->createComment(' smartling.translate_paths = data/localize/string '));
    $xml->appendChild($xml->createComment(' smartling.string_format_paths = html : data/localize/string '));
    $xml->appendChild($xml->createComment(' smartling.placeholder_format_custom = \$\{.*?\} '));

    $data = $xml->createElement('data');

    $data->appendChild(smartling_build_localize($xml, $rid, $node_content_to_translate));
    $xml->appendChild($data);

    $file_name = strtolower(trim(preg_replace('#\W+#', '_', $title), '_')) . '_' . $rid . '.xml';

    $xml = smartling_save_xml($xml, $rid, $s_locale, $file_name, FALSE);
    if (!is_null($xml)) {
      return $file_name;
    }
    else {
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

/**
 *
 * @param object $dom
 * @param string $title
 * This param set value of attribute title
 * @param array $strings
 * This is array of strings to be create
 * @param string $index
 * This is index of localize or nolocalize tag. Such as localize2 or nolocalize3.
 * Is empty string by default.
 * @param bool $no
 * If TRUE tag -> nolocalize, if FALSE tag -> localize. Tag localize use by default.
 * @return type
 */
function smartling_build_localize($dom, $title = '', $strings = array(), $index = '', $no = FALSE) {
  $element = ($no !== FALSE) ? 'nolocalize' . $index : 'localize' . $index;

  $localize = $dom->createElement($element);
  $localize_attr = $dom->createAttribute('title');
  $localize_attr->value = $title;
  $localize->appendChild($localize_attr);

  foreach ($strings as $field_mane => $field_value) {
    if (is_string($field_value)) {
      //simple field
      $string = $dom->createElement('string');
      $string_val = $dom->createTextNode($field_value);
      $string_attr = $dom->createAttribute('id');
      $string_attr->value = $field_mane;
      $string->appendChild($string_attr);
      $string->appendChild($string_val);
      $localize->appendChild($string);
    }
    elseif (is_array($field_value)) {
      if (count($field_value) > 1) {
        //fields body-summary or multifields
        if (isset($field_value['body'])) {
          //field body-summary
          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($field_value['body']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_mane . '-body';
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          $localize->appendChild($string);

          $string = $dom->createElement('string');
          $string_val = $dom->createTextNode($field_value['summary']);
          $string_attr = $dom->createAttribute('id');
          $string_attr->value = $field_mane . '-summary';
          $string->appendChild($string_attr);
          $string->appendChild($string_val);
          $localize->appendChild($string);
        }
        else {
          if (!is_string($field_value[0]) && isset($field_value[0]['body'])) {
            //multifields body-summary
            $quantity = count($field_value);
            foreach ($field_value as $key => $value) {
              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value['body']);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_mane . '-body' . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              $localize->appendChild($string);
              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value['summary']);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_mane . '-summary' . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              $localize->appendChild($string);
            }
          }
          elseif (is_string($field_value[0])) {
            //multifields
            $quantity = count($field_value);
            foreach ($field_value as $key => $value) {
              $string = $dom->createElement('string');
              $string_val = $dom->createTextNode($value);
              $string_attr = $dom->createAttribute('id');
              $string_attr->value = $field_mane . '-' . $key;
              $string->appendChild($string_attr);
              $string->appendChild($string_val);
              // set quantity of multifields
              $string_attr = $dom->createAttribute('quantity');
              $string_attr->value = $quantity;
              $string->appendChild($string_attr);
              $localize->appendChild($string);
            }
          }
        }
      }
    }
  }

  return $localize;
}

/**
 *
 * @param type $rid
 * @param type $s_locale
 * @param type $entity_data
 * @return int
 */
function smartling_check_translate($rid, $s_locale = '', $entity_data = NULL) {
  smartling_is_configured();

  if (is_null($entity_data)) {
    $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
  }
  $file_name = $entity_data->file_name;
  $file_name_unic = $entity_data->file_name;
  $file_uri = file_create_url('public://smartling/' . $file_name);

  $key = variable_get('smartling_key');
  $project_id = variable_get('smartling_project_id');
  $mode = variable_get('smartling_mode');
  //init api object
  $api = new SmartlingAPI($key, $project_id, $mode);

  //try to retrieve file status
  $status_result = $api->getStatus($file_name_unic, $s_locale);
  $status_result = json_decode($status_result);

  //This is a get status
  if ($status_result->response->code == 'SUCCESS') {

    if ($status_result->response->data->approvedStringCount !== 0) {
      $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);
    }
    else {
      $progress = 100;
    }
    smartling_watchdog('Smartling progress status for node id - @nid (@s_locale). approvedString = @as, completedString = @cs', TRUE, array(
      '@nid' => $rid,
      '@s_locale' => $s_locale,
      '@as' => $status_result->response->data->approvedStringCount,
      '@cs' => $status_result->response->data->completedStringCount
        ), WATCHDOG_INFO, l(t('View original file'), $file_uri));

    return $progress;
  }
  else {
    smartling_watchdog('Smartling try to check progress file: response code - @code, message - @message', FALSE, array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $rid));
    drupal_set_message(t('Smartling try to check progress file: response code - @code, message - @message', array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0])), 'error');
    drupal_goto(current_path());
  }
}

/**
 *
 * @param type $rid
 * @param type $s_locale
 * @param type $entity_data
 * @return int
 */
function smartling_download_translate($rid, $s_locale = '', $entity_data = NULL, $nodes_method = FALSE) {
  smartling_is_configured();

  if (is_null($entity_data)) {
    if (is_numeric($s_locale)) { // use in views report (Download)
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'eid' => $s_locale));
      $s_locale = $entity_data->target_language;
    }
    else {
      $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
    }
  }

  $nodes_method = smartling_nodes_method($entity_data->bundle);

  $file_name = $entity_data->file_name;
  $file_name_unic = $entity_data->file_name;
  $file_uri = file_create_url('public://smartling/' . $file_name);
  //$new_file_uri = SMARTLING_DIRECTORY . '/new_' . $file_name;
  $file_name = substr($file_name, 0, strlen($file_name) - 4);

  $translated_filename = $file_name . '_' . $s_locale . '.xml';

  $key = variable_get('smartling_key');
  $project_id = variable_get('smartling_project_id');
  $mode = variable_get('smartling_mode');
  //init api object
  $api = new SmartlingAPI($key, $project_id, $mode);

  $retrieval_type = variable_get('smartling_retrieval_type');
  $download_param = array(
    'retrievalType' => $retrieval_type,
  );

  //try to retrieve file status
  $status_result = $api->getStatus($file_name_unic, $s_locale);
  $status_result = json_decode($status_result);

  //This is a get status
  if ($status_result->response->code == 'SUCCESS') {
    //try to download file
    $download_result = $api->downloadFile($file_name_unic, $s_locale, $download_param);
    //This is a $download_result
    $xml = new DOMDocument();
    $xml->loadXML($download_result);

    $save = smartling_save_xml($xml, $rid, $s_locale, $translated_filename, TRUE, $nodes_method);
    if (is_object($save)) {
      if ($status_result->response->data->approvedStringCount !== 0) {
        $progress = (int) (($status_result->response->data->completedStringCount / $status_result->response->data->approvedStringCount) * 100);
      }
      else {
        $progress = 100;
      }
      smartling_watchdog('Smartling progress status for node id - @nid (@s_locale). approvedString = @as, completedString = @cs', TRUE, array(
        '@nid' => $rid,
        '@s_locale' => $s_locale,
        '@as' => $status_result->response->data->approvedStringCount,
        '@cs' => $status_result->response->data->completedStringCount
          ), WATCHDOG_INFO, l(t('View original file'), $file_uri));

      smartling_update_translated_fields($rid, $s_locale, $nodes_method);

      if (isset($_GET['destination'])) {
        $entity_data->progress = $progress;
        smartling_entity_data_save($entity_data);
        drupal_set_message(t('Uploaded for language translation @language', array('@language' => $s_locale)));
        drupal_goto($_GET['destination']);
      }
      return $progress;
    }
    else {
      if (isset($_GET['destination'])) {
        $entity_data->progress = $progress;
        smartling_entity_data_save($entity_data);
        drupal_set_message(t('Uploaded for language translation @language', array('@language' => $s_locale)));
        drupal_goto($_GET['destination']);
      }
      return $progress = 0;
    }
  }
  else {
    smartling_watchdog('Smartling try to download file: response code - @code, message - @message', FALSE, array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0]), WATCHDOG_ERROR, l(t('Related entity'), 'node/' . $rid));
    drupal_set_message(t('Smartling try to download file: response code - @code, message - @message', array('@code' => $status_result->response->code, '@message' => $status_result->response->messages[0])), 'error');
    drupal_goto(current_path());
  }
}

/**
 * Parsing the translated file and record the results in the field.
 *
 * @param type $nid
 * @param type $s_locale locale in Smartling format
 * @param bool $nodes_method
 * If TRUE - nodes method execute
 */
function smartling_update_translated_fields($rid, $s_locale, $nodes_method = FALSE) {
  $d_locale = smartling_convert_locale_drupal_to_smatrtling($s_locale, TRUE);
  smartling_watchdog('Smartling start update fields for node id - @nid. Locale: @locale', TRUE, array('@nid' => $rid, '@locale' => $s_locale), WATCHDOG_INFO);

  $entity_data = smartling_entity_load_by_conditions(array('rid' => $rid, 'target_language' => $s_locale));
  $file_name = $entity_data->translated_file_name;
  $file_path = SMARTLING_DIRECTORY . '/' . $file_name;

  if ($nodes_method) {
    $original_node = smartling_node_load_default($rid);
    $translations = translation_node_get_translations($original_node->tnid);
    if (isset($translations[$d_locale])) {
      $rid = $translations[$d_locale]->nid;
    }
    else {
      //if node not exist, need create new node
      $node = new stdClass();
      $node->type = $original_node->type;
      node_object_prepare($node);
      $node->title = $original_node->title;
      $node->language = $d_locale;
      $node->uid = $entity_data->submitter;
      $node->tnid = $original_node->nid;

      node_save($node);

      $rid = $node->nid;
    }
  }
  $smartling_translate_fields = variable_get('smartling_translate_fields');

  $fields = $smartling_translate_fields[$entity_data->bundle];

  $xml = new DOMDocument();

  $xml->load($file_path);
  $xpath = new DomXpath($xml);

  $node_wrapper = entity_metadata_wrapper('node', $rid);

  foreach ($fields as $field_name) {
    if ($field_name == 'node_title') {
      $simple_field = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
      if (is_object($simple_field)) { // simple title
        if ($simple_field instanceof DomElement) {
          $string = (string) $simple_field->nodeValue;
          $node_wrapper->title = $string;
        }
      }
    }
    else {
      // check field variant
      $simple_field = $xpath->query('//string[@id="' . $field_name . '"][1]')->item(0);
      $simple_field_multiple = $xpath->query('//string[@id="' . $field_name . '-0' . '"][1]')->item(0);
      $body_with_summary = $xpath->query('//string[@id="' . $field_name . '-body' . '"][1]')->item(0);
      $body_with_summary_multiple = $xpath->query('//string[@id="' . $field_name . '-body-0' . '"][1]')->item(0);

      if (is_object($simple_field)) { // if this field is a simple field
        if ($simple_field instanceof DomElement) {
          $string = (string) $simple_field->nodeValue;
          $info = $node_wrapper->language($d_locale)->{$field_name}->info();

          if (isset($info['property info'])) {
            $node_wrapper->language($d_locale)->{$field_name}->value = $string;
          }
          else {
            $node_wrapper->language($d_locale)->{$field_name}->set($string);
          }
        }
      }
      elseif (is_object($simple_field_multiple)) { // if this field is a simple multiple field
        $quantity = $simple_field_multiple->getAttribute('quantity');

        for ($i = 0; $i < $quantity; $i++) {
          $field = $xpath->query('//string[@id="' . $field_name . '-' . $i . '"][1]')->item(0);

          if ($field instanceof DomElement) {
            $string = (string) $field->nodeValue;
            $info = $node_wrapper->language($d_locale)->{$field_name}[$i]->info();

            if (isset($info['property info'])) {
              $node_wrapper->language($d_locale)->{$field_name}[$i]->value = $string;
            }
            else {
              $node_wrapper->language($d_locale)->{$field_name}[$i]->set($string);
            }
          }
        }
      }
      elseif (is_object($body_with_summary)) { // if this field is a simple body/summary field
        if ($body_with_summary instanceof DomElement) {
          $body = (string) $body_with_summary->nodeValue;
          $summary = $xpath->query('//string[@id="' . $field_name . '-summary' . '"][1]')->item(0);
          $summary = (string) $summary->nodeValue;

          $info = $node_wrapper->language($d_locale)->{$field_name}->info();

          if (isset($info['property info'])) {
            $node_wrapper->language($d_locale)->{$field_name}->value = $body;
            $node_wrapper->language($d_locale)->{$field_name}->summary = $summary;
          }
          else {
            $node_wrapper->language($d_locale)->{$field_name}->set(array('value' => $body, 'summary' => $summary));
          }
        }
      }
      elseif (is_object($body_with_summary_multiple)) { //if this field is a multiple body/summary
        $quantity = $body_with_summary_multiple->getAttribute('quantity');

        for ($i = 0; $i < $quantity; $i++) {
          $body_obj = $xpath->query('//string[@id="' . $field_name . '-body-' . $i . '"][1]')->item(0);
          $summary_obj = $xpath->query('//string[@id="' . $field_name . '-summary-' . $i . '"][1]')->item(0);

          if ($body_obj instanceof DomElement && $summary_obj instanceof DomElement) {
            $body_string = (string) $body_obj->nodeValue;
            $summary_string = (string) $summary_obj->nodeValue;

            $info = $node_wrapper->language($d_locale)->{$field_name}[$i]->info();

            if (isset($info['property info'])) {
              $node_wrapper->language($d_locale)->{$field_name}[$i]->value = $body_string;
              $node_wrapper->language($d_locale)->{$field_name}[$i]->summary = $summary_string;
            }
            else {
              $node_wrapper->language($d_locale)->{$field_name}[$i]->set(array('value' => $body_string, 'summary' => $summary_string));
            }
          }
        }
      }
    }
  }

  $node_wrapper->save();

  $entity_data->download = 1;
  smartling_entity_data_save($entity_data);

  smartling_watchdog('Smartling update all translated fields for node id - @nid. Locale: @locale', TRUE, array('@nid' => $rid, '@locale' => $s_locale), WATCHDOG_INFO, l(t('View translated node'), $d_locale . '/node/' . $rid));
}

/**
 * Duplicate the function node_admin_content() for the introduction of mass smartling action.
 */
function smartling_node_admin_content($form, $form_state) {
  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    return node_multiple_delete_confirm($form, $form_state, array_filter($form_state['values']['nodes']));
  }

  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'smartling_translate') {
    if (smartling_translate_fields_is_set()) {
      return smartling_node_mass_translate_confirm($form, $form_state, array_filter($form_state['values']['nodes']));
    }
  }

  $form['filter'] = node_filter_form();
  $form['#submit'][] = 'node_filter_form_submit';
  $form['admin'] = node_admin_nodes();

  return $form;
}

/**
 * Confirmation form for mass translation.
 */
function smartling_node_mass_translate_confirm($form, &$form_state, $nodes) {
  $languages = language_list();
  $options = array();
  foreach ($languages as $langcode => $language) {
    if ($language->enabled != '0') {
      $options[$language->language] = $language->name;
    }
  }

  $nodes = node_load_multiple($nodes);
  $form_state['data_nodes'] = ((empty($nodes)) ? array() : $nodes);
  $form['node']['languages'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#title' => t('Check language'),
  );


  foreach ($nodes as $key => $node) {
    if (smartling_supported_type($node->type)) {
      if (smartling_fields_method($node->type)) {
        $data_item = array(
          'title' => $node->title,
          'status' => t('translate'),
        );

        $rows[] = array(
          'data' => $data_item,
          'class' => array('ok'),
        );
      }
      elseif (smartling_nodes_method($node->type)) {
        $status = smartling_nodes_method_node_status($node->nid);
        if ($status == SMARTLING_ORIGINAL_NODE || $status == SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE) {
          $data_item = array(
            'title' => $node->title,
            'status' => t('original node for nodes method'),
          );

          $rows[] = array(
            'data' => $data_item,
            'class' => array('ok'),
          );
        }
        else {
          $data_item = array(
            'title' => $node->title,
            'status' => t('not supported'),
          );

          $rows[] = array(
            'data' => $data_item,
          );
        }
      }
    }
    else {
      $data_item = array(
        'title' => $node->title,
        'status' => t('not supported'),
      );

      $rows[] = array(
        'data' => $data_item,
      );
    }
  }

  $form['node']['nodes_table'] = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => array(t('Title'), t('Status')),
  );

  $form['operation'] = array('#type' => 'hidden', '#value' => 'smartling_translate');
  $form['#submit'][] = 'smartling_node_mass_translate_confirm_submit';

  $confirm_question = format_plural(count($nodes), 'Are you sure you want to translate this item?', 'Are you sure you want to translate these items?');
  return confirm_form($form, $confirm_question, 'admin/content', t('This action cannot be undone.'), t('Translate'), t('Cancel'));
}

function smartling_node_mass_translate_confirm_submit($form, &$form_state) {
  smartling_watchdog('Smartling add mass node translate operations.', TRUE);

  if ($form_state['values']['confirm']) {
    global $user;
    $smartling_queue = DrupalQueue::get('smartling_send_to_translate');
    $smartling_queue->createQueue();
    $count_op = 0;

    foreach ($form_state['values']['languages'] as $language => $check) {
      if ($check) {
        $s_locale = smartling_convert_locale_drupal_to_smatrtling($language);
        foreach ($form_state['data_nodes'] as $node) {
          if (smartling_supported_type($node->type)) {
            if (smartling_nodes_method($node->type)) {
              $status = smartling_nodes_method_node_status($node->nid);
              if ($status == SMARTLING_ORIGINAL_NODE || $status == SMARTLING_ORIGINAL_WITHOUT_TRANSLATION_NODE) {
                if ($node->tnid == '0') {
                  $node->tnid = $node->nid;
                  node_save($node);
                }
                if ($language != $node->language) {
                  $obj = new stdClass;
                  $obj->rid = $node->nid;
                  $obj->type = $node->type;
                  $obj->title = $node->title;
                  $obj->original_language = smartling_convert_locale_drupal_to_smatrtling($node->language);
                  $obj->s_locale = $s_locale;
                  $obj->submitter = $user->uid;
                  // create queue item
                  $smartling_queue->createItem($obj);
                  $count_op++;
                  unset($obj);
                }
              }
            }
            elseif (smartling_fields_method($node->type)) {
              if ($language != $node->language) {
                $obj = new stdClass;
                $obj->rid = $node->nid;
                $obj->type = $node->type;
                $obj->title = $node->title;
                $obj->original_language = smartling_convert_locale_drupal_to_smatrtling($node->language);
                $obj->s_locale = $s_locale;
                $obj->submitter = $user->uid;
                // create queue item
                $smartling_queue->createItem($obj);
                $count_op++;
                unset($obj);
              }
            }
          }
        }
      }
    }

    drupal_set_message(t('Smartling add @count operations', array('@count' => $count_op)));
  }

  $form_state['redirect'] = 'admin/content';
}

/**
 * Smartling watchdog function.
 *
 * @param string $message
 * @param bool $consider_log
 * If FALSE, smartling not take into account the setting of logging. TRUE by default.
 * @param array $variables
 * @param int $severity
 * WATCHDOG_INFO by default.
 * @param string $link
 */
function smartling_watchdog($message, $consider_log = TRUE, $variables = array(), $severity = WATCHDOG_INFO, $link = NULL) {
  if ($consider_log) {
    if (SMARTLING_LOG) {
      watchdog('smartling', $message, $variables, $severity, $link);
    }
  }
  else {
    watchdog('smartling', $message, $variables, $severity, $link);
  }
}


<?php
/**
 * @file
 * smartling_demo_rules.rules_defaults.inc
 */

/**
 * Implements hook_default_rules_configuration().
 */
function smartling_demo_rules_default_rules_configuration() {
  $items = array();
  $items['rules_smartling_call'] = entity_import('rules_config', '{ "rules_smartling_call" : {
      "LABEL" : "smartling call",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "php", "rules", "smartling" ],
      "ON" : { "smartling_callback_call_event" : [] },
      "DO" : [
        { "php_eval" : { "code" : "drupal_set_message(t(\\u0027Event: Smartling callback call\\u0027));" } }
      ]
    }
  }');
  $items['rules_smartling_download_translate'] = entity_import('rules_config', '{ "rules_smartling_download_translate" : {
      "LABEL" : "smartling download translate",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "php", "rules", "smartling" ],
      "ON" : { "smartling_translation_download_event" : [] },
      "DO" : [
        { "php_eval" : { "code" : "drupal_set_message(t(\\u0027Event: Smartling download translate\\u0027));" } }
      ]
    }
  }');
  return $items;
}

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
      "LABEL" : "Smartling call",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "smartling" ],
      "ON" : { "smartling_callback_call_event" : [] },
      "DO" : [
        { "drupal_message" : { "message" : "Event: After handling callback from Smartling" } }
      ]
    }
  }');
  $items['rules_smartling_download_translate'] = entity_import('rules_config', '{ "rules_smartling_download_translate" : {
      "LABEL" : "Smartling download translate",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "smartling" ],
      "ON" : { "smartling_translation_download_event" : [] },
      "DO" : [
        { "drupal_message" : { "message" : "Event: After downloading translation from Smartling" } }
      ]
    }
  }');
  $items['rules_smartling_upload_translate'] = entity_import('rules_config', '{ "rules_smartling_upload_translate" : {
      "LABEL" : "Smartling upload translate",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "smartling" ],
      "ON" : { "smartling_uploading_original_to_smartling_event" : [] },
      "DO" : [
        { "drupal_message" : { "message" : "Event: After uploading original content to Smartling" } }
      ]
    }
  }');
  return $items;
}

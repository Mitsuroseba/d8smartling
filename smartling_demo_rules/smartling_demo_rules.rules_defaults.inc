<?php
/**
 * @file
 * Smartling demo rules defaults inc.
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
      "REQUIRES" : [ "smartling" ],
      "ON" : { "smartling_callback_call_event" : [] },
      "DO" : []
    }
  }');
  $items['rules_smartling_download_translate'] = entity_import('rules_config', '{ "rules_smartling_download_translate" : {
      "LABEL" : "Smartling download translate",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "smartling" ],
      "ON" : { "smartling_translation_download_event" : [] },
      "DO" : []
    }
  }');
  $items['rules_smartling_upload_translate'] = entity_import('rules_config', '{ "rules_smartling_upload_translate" : {
      "LABEL" : "Smartling upload translate",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "smartling" ],
      "ON" : { "smartling_uploading_original_to_smartling_event" : [] },
      "DO" : []
    }
  }');
  return $items;
}

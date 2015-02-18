<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class DownloadQueueManager implements QueueManagerInterface {
  /**
   * @inheritdoc
   */
  public function add($entity_type, $entity, $langs) {
  }

  /**
   * @inheritdoc
   */
  public function execute($eids) {
    if (language_default('language') != field_valid_language(NULL, FALSE)) {
      drupal_set_message('The download failed. Please switch to the site\'s default language: ' . language_default('language'), 'error');
      return FALSE;
    }

    if (!is_array($eids)) {
      $eids = array($eids);
    }

    $global_status = TRUE;
    foreach ($eids as $eid) {
      $status = FALSE;

      $smartling_entity = entity_load_single('smartling_entity_data', $eid);
      if ($smartling_entity && smartling_is_configured() && smartling_translate_fields_configured($smartling_entity->bundle, $smartling_entity->entity_type)) {
        $processor = smartling_get_entity_processor($smartling_entity);
        if ($processor->downloadTranslation()) {
          $status = $processor->updateEntityFromXML();
        }

      }
      $global_status = $global_status & $status;
    }

    return $global_status;
  }
}

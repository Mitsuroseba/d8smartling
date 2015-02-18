<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class CheckStatusQueueManager implements QueueManagerInterface {
  /**
   * @inheritdoc
   */
  public function add($entity_type, $entity, $langs) {
  }

  /**
   * @inheritdoc
   */
  public function execute($eids) {
    if (!is_array($eids)) {
      $eids = array($eids);
    }

    foreach($eids as $eid) {
      $smartling_entity = entity_load_single('smartling_entity_data', $eid);

      if (smartling_is_configured()) {
        $api = drupal_container()->get('smartling.api_wrapper');
        $result = $api->getStatus($smartling_entity);

        if (!empty($result)) {
          if (($result['response_data']->approvedStringCount == $result['response_data']->completedStringCount) && ($smartling_entity->entity_type != 'smartling_interface_entity')) {
            $smartling_queue = \DrupalQueue::get('smartling_download');
            $smartling_queue->createQueue();
            $smartling_queue->createItem($eid);
          }

          smartling_entity_data_save($result['entity_data']);
        }
      }
    }
  }
}

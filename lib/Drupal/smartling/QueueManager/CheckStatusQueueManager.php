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
  public function add($eids) {
    $log = smartling_log_get_handler();

    $smartling_entities = smartling_entity_data_load_multiple($eids);

    $smartling_queue = \DrupalQueue::get('smartling_check_status');
    $smartling_queue->createQueue();
    foreach ($smartling_entities as $eid => $queue_item) {
      if (!empty($queue_item->file_name)) {
        $smartling_queue->createItem($eid);
        $log->setMessage('Add item to "smartling_check_status" queue. Smartling entity data id - @eid, related entity id - @rid, entity type - @entity_type')
          ->setVariables(array(
            '@eid' => $queue_item->eid,
            '@rid' => $queue_item->rid,
            '@entity_type' => $queue_item->entity_type,
          ))
          ->execute();
      }
      elseif ($queue_item->status != 0) {
        $log->setMessage('Original file name is empty. Smartling entity data id - @eid, related entity id - @rid, entity type - @entity_type')
          ->setVariables(array(
            '@eid' => $queue_item->eid,
            '@rid' => $queue_item->rid,
            '@entity_type' => $queue_item->entity_type,
          ))
          ->setSeverity(WATCHDOG_WARNING)
          ->execute();
      }
    }
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
            drupal_container()->get('smartling.queue_managers.download')->add($eid);
          }

          smartling_entity_data_save($result['entity_data']);
        }
      }
    }
  }
}

<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class CheckStatusQueueManager implements QueueManagerInterface {

  protected $api_wrapper;
  protected $smartling_submission_wrapper;
  protected $queue_download;
  protected $log;
  protected $smartling_utils;
  protected $submissions_collection;

  public function __construct($api_wrapper, $smartling_submission_wrapper, $submissions_collection, $queue_download, $log, $smartling_utils) {
    $this->api_wrapper = $api_wrapper;
    $this->smartling_submission_wrapper = $smartling_submission_wrapper;
    $this->submissions_collection = $submissions_collection;
    $this->queue_download = $queue_download;
    $this->log = $log;
    $this->smartling_utils = $smartling_utils;
  }

  /**
   * @inheritdoc
   */
  public function add($eids) {
    //$smartling_entities = smartling_entity_data_load_multiple($eids);
    $smartling_entities = $this->submissions_collection->loadByIDs($eids)->getCollection();

    $smartling_queue = \DrupalQueue::get('smartling_check_status');
    $smartling_queue->createQueue();
    foreach ($smartling_entities as $eid => $queue_item) {
      if (!empty($queue_item->file_name)) {
        $smartling_queue->createItem($eid);
        $this->log->info('Add item to "smartling_check_status" queue. Smartling entity data id - @eid, related entity id - @rid, entity type - @entity_type',
          array('@eid' => $queue_item->eid, '@rid' => $queue_item->rid, '@entity_type' => $queue_item->entity_type));
      }
      elseif ($queue_item->status != 0) {
        $this->log->warning('Original file name is empty. Smartling entity data id - @eid, related entity id - @rid, entity type - @entity_type',
          array('@eid' => $queue_item->eid, '@rid' => $queue_item->rid, '@entity_type' => $queue_item->entity_type));
      }
    }
  }

  /**
   * @inheritdoc
   */
  public function execute($eids) {
    if (!$this->smartling_utils->isConfigured()) {
      throw new \Drupal\smartling\SmartlingExceptions\SmartlingNotConfigured(t('Smartling module is not configured. Please follow the page <a href="@link">"Smartling settings"</a> to setup Smartling configuration.', array('@link' => url('admin/config/regional/smartling'))));
    }

    if (!is_array($eids)) {
      $eids = array($eids);
    }

    foreach($eids as $eid) {
      $smartling_submission = $this->smartling_submission_wrapper->loadByID($eid)->getEntity();

      $result = $this->api_wrapper->getStatus($smartling_submission);
      if (!empty($result)) {

        if (($result['response_data']->approvedStringCount == $result['response_data']->completedStringCount)
             && ($smartling_submission->entity_type != 'smartling_interface_entity')) {
          $this->queue_download->add($eid);
        }

        //smartling_entity_data_save($result['entity_data']);
        $this->smartling_submission_wrapper->setEntity($result['entity_data'])->save();
      }
    }
  }
}

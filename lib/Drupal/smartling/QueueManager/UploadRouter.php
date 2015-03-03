<?php

/**
 * @file
 * Contains Drupal\smartling\Forms.
 */

namespace Drupal\smartling\QueueManager;

class UploadRouter {
  protected $entity_wrapper_collection;
  protected $upload_manager;
  protected $log;

  public function __construct($entity_wrapper_collection, $upload_manager, $log) {
    $this->entity_wrapper_collection = $entity_wrapper_collection;
    $this->upload_manager = $upload_manager;
    $this->log = $log;
  }

  public function routeUploadRequest($entity_type, $entity, $languages, $async_mode = TRUE) {
    $this->entity_wrapper_collection->createForLanguages($entity_type, $entity, $languages);
    if ($async_mode) {
      $this->upload_manager->add($this->entity_wrapper_collection->getIDs());

      $collection = $this->entity_wrapper_collection->getCollection();
      $smartling_wrapper = reset($collection);

      // Create content hash (Fake entity update).
      smartling_entity_update($entity, $entity_type);

      $langs = implode(', ', $languages);
      $this->log->setMessage('Smartling queue task was created for entity id - @id, locale - @locale, type - @entity_type')
        ->setVariables(array(
          '@id' => $smartling_wrapper->getRID(),
          '@locale' => $langs,
          '@entity_type' => $entity_type,
        ))
        ->execute();

      drupal_set_message(t('The @entity_type "@title" has been scheduled to be sent to Smartling for translation to "@langs".', array(
        '@entity_type' => $entity_type,
        '@title' => $smartling_wrapper->getTitle(),
        '@langs' => $langs,
      )));
    }
    else {
      $this->upload_manager->execute($this->entity_wrapper_collection->getIDs());

      $collection = $this->entity_wrapper_collection->getCollection();
      $smartling_wrapper = reset($collection);

      drupal_set_message(t('The @entity_type "@title" has been sent to Smartling for translation to "@langs".', array(
        '@entity_type' => $entity_type,
        '@title' => $smartling_wrapper->getTitle(),
        '@langs' => implode(', ', $languages),
      )));
    }
  }

}

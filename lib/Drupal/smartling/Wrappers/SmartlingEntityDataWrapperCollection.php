<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Wrappers;
use Drupal\smartling\SmartlingExceptions\SmartlingGenericException;
use Drupal\smartling\SmartlingExceptions\WrongSiteSettingsException;

/**
 * Class SmartlingEntityDataWrapperCollection.
 */
class SmartlingEntityDataWrapperCollection {

  private $collection = array();

  protected $smartling_submission_wrapper;
  protected $log;
  protected $entity_api_wrapper;

  public function __construct (SmartlingEntityDataWrapper $smartling_submission_wrapper, $log, EntityAPIWrapper $entity_api_wrapper) {
    $this->smartling_submission_wrapper = $smartling_submission_wrapper;
    $this->log = $log;
    $this->entity_api_wrapper = $entity_api_wrapper;
  }

  public function getCollection() {
    return $this->collection;
  }

  public function setCollection(array $collection = array()) {
    $this->collection = $collection;
    return $this;
  }

  public function loadByIDs(array $eids) {
    foreach($eids as $eid) {
      $wrapper = clone $this->smartling_submission_wrapper->loadByID($eid);
      $this->add($wrapper);
    }
    return $this;
  }

  public function add($item) {
    $this->collection []= $item;
    return $this;
  }

  public function deleteAll() {
    $this->collection = array();
  }

  public function createForLanguages($entity_type, $entity, $languages) {

    $this->deleteAll();
    $entity = $this->entity_api_wrapper->getOriginalEntity($entity_type, $entity);

    if (empty($entity)) {
      return;
    }

    $wrapper = $this->entity_api_wrapper->entityMetadataWrapper($entity_type, $entity);
    $id      = $wrapper->getIdentifier();
    $bundle  = $wrapper->getBundle();
    $link    = $this->entity_api_wrapper->getLink($entity_type, $entity);

    if (!smartling_translate_fields_configured($bundle, $entity_type) && $entity_type != 'smartling_interface_entity') {
      //$this->log->error("Type '@type' is not supported or it's not configured in Smartling.", array('@type' => $bundle, 'entity_link' => $link), TRUE);
      throw new \Drupal\smartling\SmartlingExceptions\WrongSiteSettingsException(t("Type '@type' is not supported or it's not configured in Smartling.", array('@type' => $bundle)));
    }

    // $d_locale_original = language_default()->language;
    // $d_locale_original = $entity->translations->original;
    $d_locale_original = $this->entity_api_wrapper->entityLanguage($entity_type, $entity);
    foreach ($languages as $target_language) {
      if ($target_language == $d_locale_original) {
        continue;
      }

      $wrapper = clone $this->smartling_submission_wrapper
        ->loadSingleByConditions(array('rid' => $id, 'entity_type' => $entity_type, 'target_language' => $target_language))
        ->orCreateFromDrupalEntity($entity, $entity_type, $d_locale_original, $target_language)
        ->setStatusByEvent(SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE)
        ->setSubmitter()
        ->setSubmissionDate(REQUEST_TIME)
        ->save();

      $this->add($wrapper);
    }

    return $this;
    //return $queued_eids;
  }

  public function getIDs() {
    $eids = array();
    foreach ($this->getCollection() as $wrapper) {
      $eids []= $wrapper->getEID();
    }

    return $eids;
  }

}

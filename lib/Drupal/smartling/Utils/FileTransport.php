<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Utils;

/**
 * Class FileTransport.
 */
class FileTransport {
  protected $smartling_utils;
  protected $drupal_wrapper;
  protected $api_wrapper;

  public function __construct($smartling_utils, $drupal_wrapper, $api_wrapper) {
    $this->smartling_utils = $smartling_utils;
    $this->drupal_wrapper = $drupal_wrapper;
    $this->api_wrapper = $api_wrapper;

  }

  public function upload($xml, $submission, $target_locales) {
    $event   = SMARTLING_STATUS_EVENT_FAILED_UPLOAD;

    $file_name = $submission->getFileName();
    $success = (bool) $this->smartling_utils->saveXml($file_name, $xml, $submission);
    // Init api object.
    if ($success) {
      $file_path = $this->drupal_wrapper->drupalRealpath($this->settings->getDir($file_name), TRUE);
      $event = $this->api_wrapper->uploadFile($file_path, $file_name, 'xml', $target_locales);
    }

    return $event;
  }


  /**
   * Fetch translation status from Smartling server.
   *
   * @return bool
   */
  protected function getProgressStatus($smartling_submission) {
    $file_name = $smartling_submission->getFileName();
    if (!empty($file_name)) {
      $result = $this->smartlingAPI->getStatus($smartling_submission->getEntity());

      if (!empty($result)) {
        return $result['entity_data']->progress;
      }
      else {
        return FALSE;
      }
    }
    else {
      return FALSE;
    }
  }


  public function download($submission) {
    $progress = $this->getProgressStatus($submission);
    if ($progress === FALSE) {
      return;
    }

    $download_result = $this->smartlingAPI->downloadFile($submission->getEntity());

    libxml_use_internal_errors(true);
    if (FALSE === simplexml_load_string($download_result)) {
      return;
    }
    // This is a download result.
    $xml = new \DOMDocument();
    $xml->loadXML($download_result);

    $translated_file_name = $submission->getFileTranslatedName();
//    $file_name = substr($this->entity->file_name, 0, strlen($this->entity->file_name) - 4);
//    $translated_file_name = $file_name . '_' . $this->entity->target_language . '.xml';

    // Save result.
    $isSuccess = $this->smartling_utils->saveXML($translated_file_name, $xml, $submission->getEntity());

    // If result is saved.
    if ($isSuccess) {
      $submission
        ->setStatusByEvent(SMARTLING_STATUS_EVENT_UPDATE_FIELDS)
        ->setProgress($progress)
        ->save();

      $isSuccess = $this->updateDrupalTranslation();
    }

    return $isSuccess;
  }
}

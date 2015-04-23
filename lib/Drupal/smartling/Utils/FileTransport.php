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
  protected $settings;

  public function __construct($settings, $api_wrapper, $drupal_wrapper, $smartling_utils) {
    $this->smartling_utils = $smartling_utils;
    $this->drupal_wrapper = $drupal_wrapper;
    $this->api_wrapper = $api_wrapper;
    $this->settings = $settings;
  }

  public function upload($content, $submission, $target_locales) {
    $event   = SMARTLING_STATUS_EVENT_FAILED_UPLOAD;

    $file_name = $submission->getFileName();
    $success = (bool) $this->smartling_utils->saveFile($file_name, $content, $submission);
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
      $result = $this->api_wrapper->getStatus($smartling_submission->getEntity());

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

    $download_result = $this->api_wrapper->downloadFile($submission->getEntity());

    $translated_file_name = $submission->getFileTranslatedName();

    // Save result.
    $isSuccess = $this->smartling_utils->saveFile($translated_file_name, $download_result, $submission->getEntity());

    // If result is saved.
    if ($isSuccess) {
      $submission
        ->setStatusByEvent(SMARTLING_STATUS_EVENT_UPDATE_FIELDS)
        ->setProgress($progress)
        ->save();

      //$isSuccess = $this->updateDrupalTranslation();
    }

    return $download_result;
  }
}

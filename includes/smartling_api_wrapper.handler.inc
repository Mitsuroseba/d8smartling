<?php

/**
 * @file
 * Smartling settings handler.
 */

/**
 * Class SmartlingApiWrapper.
 */
class SmartlingApiWrapper {

  protected $settingsHandler;
  protected $logger;
  protected $api;


  protected function convert_locale_drupal_to_smartling($locale, $reverse = FALSE) {
    $locales = $this->settingsHandler->getLocalesConvertArray();
    if (!$reverse) {
      if (isset($locales[$locale])) {
        return $locales[$locale];
      }
      else {
        return NULL;
      }
    }
    else {
      foreach ($locales as $key => $loc) {
        if ($locale == $loc) {
          return $key;
        }
      }
    }
  }

  /**
   * Initialize.
   */
  public function __construct(SmartlingSettingsHandler $settings_handler, $logger) {
    $this->settingsHandler = $settings_handler;
    $this->logger = $logger;

    $this->setApi(new SmartlingAPI($settings_handler->getApiUrl(), $settings_handler->getKey(), $settings_handler->getProjectId(), SMARTLING_PRODUCTION_MODE));
  }

  public function setApi(SmartlingAPI $api) {
    $this->api = $api;
  }

  public function downloadFile($entity_data, $link_to_entity) {
    $entity_type = $entity_data->entity_type;
    $d_locale = $entity_data->target_language;
    $file_name_unic = $entity_data->file_name;
    $file_path = $this->settingsHandler->get_dir($entity_data->file_name);

    $retrieval_type = $this->settingsHandler->variableGet('smartling_retrieval_type', 'published');
    $download_param = array(
      'retrievalType' => $retrieval_type,
    );

    $this->logger->setMessage('Smartling queue start download xml file and update fields for @entity_type id - @rid, locale - @locale.')
      ->setVariables(array(
        '@entity_type' => $entity_type,
        '@rid' => $entity_data->rid,
        '@locale' => $entity_data->target_language,
      ))
      ->setLink(l(t('View file'), $file_path))
      ->execute();

    $s_locale = $this->convert_locale_drupal_to_smartling($d_locale);
    // Try to download file.
    $download_result = $this->api->downloadFile($file_name_unic, $s_locale, $download_param);

    if (isset($download_result->response->code)) {
      $download_result = json_decode($download_result);

      $this->logger->setMessage('smartling_queue_download_update_translated_item_process try to download file:<br/>
      Project Id: @project_id <br/>
      Action: download <br/>
      URI: @file_uri <br/>
      Locale: @s_locale <br/>
      Error: response code -> @code and message -> @message')
        ->setVariables(array(
          '@project_id' => $this->settingsHandler->getProjectId(),
          '@file_uri' => $file_name_unic,
          '@s_locale' => $s_locale,
          '@code' => $download_result->response->code,
          '@message' => $download_result->response->messages[0],
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      return FALSE;
    }

    return $download_result;
  }









  //getStatus
  public function getStatus($args, $entity_data, $link_to_entity) {
    $error_result = null;

    if ($entity_data === FALSE) {
      $this->logger->setMessage('Smartling checks status for id - @rid is FAIL! Smartling entity not exist.')
        ->setVariables(array('@rid' => $args->rid))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      return $error_result;
    }

    if ($entity_data->progress == 100) {
      return $error_result;
    }

    $file_name = $entity_data->file_name;
    $file_name_unic = $entity_data->file_name;
    $file_uri = smartling_clean_filename($this->settingsHandler->get_dir() . '/' . $file_name, TRUE);

    $s_locale = $this->convert_locale_drupal_to_smartling($args->d_locale);
    // Try to retrieve file status.
    $status_result = $this->api->getStatus($file_name_unic, $s_locale);
    $status_result = json_decode($status_result);

    // This is a get status.
    if ($this->api->getCodeStatus() != 'SUCCESS') {
      $this->logger->setMessage('Smartling checks status for @entity_type id - @rid: <br/>
      Project Id: @project_id <br/>
      Action: status <br/>
      URI: @file_uri <br/>
      Locale: @d_locale <br/>
      Error: response code -> @code and message -> @message')
        ->setVariables(array(
          '@entity_type' => $args->entity_type,
          '@rid' => $args->rid,
          '@project_id' => $this->settingsHandler->getProjectId(),
          '@file_uri' => $file_name_unic,
          '@d_locale' => $args->d_locale,
          '@code' => $status_result->response->code,
          '@message' => $status_result->response->messages[0],
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      return $error_result;
    }


    $this->logger->setMessage('Smartling checks status for @entity_type id - @rid (@d_locale). approvedString = @as, completedString = @cs')
      ->setVariables(array(
        '@entity_type' => $args->entity_type,
        '@rid' => $args->rid,
        '@d_locale' => $args->d_locale,
        '@as' => $status_result->response->data->approvedStringCount,
        '@cs' => $status_result->response->data->completedStringCount,
      ))
      ->setLink(l(t('View file'), $file_uri))
      ->execute();

    // If true, file translated.
    $response_data = $status_result->response->data;
    $progress = ($response_data->approvedStringCount == $response_data->completedStringCount|| $response_data->approvedStringCount == 0) ?
                100 : (int) (($response_data->completedStringCount / $response_data->approvedStringCount) * 100);
    $entity_data->download = 0;
    $entity_data->progress = $progress;
    $entity_data->status = SMARTLING_STATUS_IN_TRANSLATE;

    return array('entity_data' => $entity_data, 'response_data' => $status_result->response->data);
  }

  public function testConnection() {

  }
}

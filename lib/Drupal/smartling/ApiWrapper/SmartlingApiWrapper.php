<?php

/**
 * @file
 * Smartling api wrapper.
 */

namespace Drupal\smartling\ApiWrapper;

/**
 * Class SmartlingApiWrapper.
 */
class SmartlingApiWrapper {

  protected $settingsHandler;
  protected $logger;
  protected $api;

  /**
   * This functions convert locale format. Example: 'en' => 'en-US'.
   *
   * @param string $locale
   *   Locale string in some format: 'en' or 'en-US'.
   * @param bool $reverse
   *   If TRUE, convert format: 'en-US' => 'en'. FALSE by default.
   *
   * @return string|null
   *   Return locale or NULL.
   */
  protected function convertLocaleDrupalToSmartling($locale, $reverse = FALSE) {
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
  public function __construct($settings_handler, $logger) {
    $this->settingsHandler = $settings_handler;
    $this->logger = $logger;

    $this->setApi(new \SmartlingAPI($settings_handler->getApiUrl(), $settings_handler->getKey(), $settings_handler->getProjectId(), SMARTLING_PRODUCTION_MODE));
  }

  /**
   * Set Smartling API.
   *
   * @param \SmartlingAPI $api
   *   Smartling API.
   */
  public function setApi(\SmartlingAPI $api) {
    $this->api = $api;
  }

  /**
   * Download file from service.
   *
   * @param object $smartling_entity
   *   Smartling transaction entity.
   *
   * @return \DOMDocument
   *   Return xml dom from downloaded file.
   */
  public function downloadFile($smartling_entity) {
    $smartling_entity_type = $smartling_entity->entity_type;
    $d_locale = $smartling_entity->target_language;
    $file_name_unic = $smartling_entity->file_name;
    $file_path = $this->settingsHandler->getDir($smartling_entity->file_name);

    $retrieval_type = $this->settingsHandler->variableGet('smartling_retrieval_type', 'published');
    $download_param = array(
      'retrievalType' => $retrieval_type,
    );

    $this->logger->setMessage('Smartling queue start download xml file and update fields for @entity_type id - @rid, locale - @locale.')
      ->setVariables(array(
        '@entity_type' => $smartling_entity_type,
        '@rid' => $smartling_entity->rid,
        '@locale' => $smartling_entity->target_language,
      ))
      ->setLink(l(t('View file'), $file_path))
      ->execute();

    $s_locale = $this->convertLocaleDrupalToSmartling($d_locale);
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
        ->execute();

      return FALSE;
    }

    return $download_result;
  }


  /**
   * Get status.
   *
   * @param object $smartling_entity
   *   Smartling transaction entity.
   *
   * @return array|null
   *   Return status.
   */
  public function getStatus($smartling_entity) {
    $error_result = NULL;

    if ($smartling_entity === FALSE) {
      $this->logger->setMessage('Smartling checks status for id - @rid is FAIL! Smartling entity not exist.')
        ->setVariables(array('@rid' => $smartling_entity->rid))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->execute();

      return $error_result;
    }

    if ($smartling_entity->progress == 100) {
      return $error_result;
    }

    $file_name = $smartling_entity->file_name;
    $file_name_unic = $smartling_entity->file_name;
    $file_uri = smartling_clean_filename($this->settingsHandler->getDir() . '/' . $file_name, TRUE);

    $s_locale = $this->convertLocaleDrupalToSmartling($smartling_entity->target_language);
    // Try to retrieve file status.
    $status_result = $this->api->getStatus($file_name_unic, $s_locale);
    $status_result = json_decode($status_result);

    // This is a get status.
    if (($this->api->getCodeStatus() != 'SUCCESS') || !isset($status_result->response->data)) {
      $this->logger->setMessage('Smartling checks status for @entity_type id - @rid: <br/>
      Project Id: @project_id <br/>
      Action: status <br/>
      URI: @file_uri <br/>
      Locale: @d_locale <br/>
      Error: response code -> @code and message -> @message')
        ->setVariables(array(
          '@entity_type' => $smartling_entity->entity_type,
          '@rid' => $smartling_entity->rid,
          '@project_id' => $this->settingsHandler->getProjectId(),
          '@file_uri' => $file_name_unic,
          '@d_locale' => $smartling_entity->target_language,
          '@code' => $status_result->response->code,
          '@message' => $status_result->response->messages[0],
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->execute();

      return $error_result;
    }

    $this->logger->setMessage('Smartling checks status for @entity_type id - @rid (@d_locale). approvedString = @as, completedString = @cs')
      ->setVariables(array(
        '@entity_type' => $smartling_entity->entity_type,
        '@rid' => $smartling_entity->rid,
        '@d_locale' => $smartling_entity->target_language,
        '@as' => $status_result->response->data->approvedStringCount,
        '@cs' => $status_result->response->data->completedStringCount,
      ))
      ->setLink(l(t('View file'), $file_uri))
      ->execute();

    // If true, file translated.
    $response_data = $status_result->response->data;
    $approved = $response_data->approvedStringCount;
    $completed = $response_data->completedStringCount;
    $progress = ($approved == $completed || $approved == 0) ? 100 : (int) (($completed / $approved) * 100);
    $smartling_entity->download = 0;
    $smartling_entity->progress = $progress;
    $smartling_entity->status = SMARTLING_STATUS_IN_TRANSLATE;

    return array(
      'entity_data' => $smartling_entity,
      'response_data' => $status_result->response->data,
    );
  }

  /**
   * Test connect.
   */
  public function testConnection($locales) {
    $result = array();

    foreach ($locales as $key => $locale) {
      if ($locale !== 0 && $locale == $key) {
        $s_locale = $this->convertLocaleDrupalToSmartling($locale);
        // Init api object.
        $server_response = $this->api->getList($s_locale, array('limit' => 1));

        if ($this->api->getCodeStatus() == 'SUCCESS') {
          $result[$s_locale] = TRUE;
        }
        else {
          $this->logger->setMessage('Connection test for project: @project_id and locale: @locale FAILED and returned the following result: @server_response.')
            ->setVariables(array(
              '@project_id' => $this->settingsHandler->getProjectId(),
              '@locale' => $key,
              '@server_response' => $server_response,
            ))
            ->execute();
        }
      }
    }

    return $result;
  }

  /**
   * Upload file to service.
   *
   * @param string $file_path
   *   File path.
   * @param string $file_name_unic
   *   File name.
   * @param array $locales
   *   Locales.
   *
   * @return string
   *   Return status string.
   */
  public function uploadFile($file_path, $file_name_unic, array $locales) {
    $locales_to_approve = array();
    foreach ($locales as $locale) {
      $locales_to_approve[] = $this->convertLocaleDrupalToSmartling($locale);
    }

    $upload_params = new \FileUploadParameterBuilder();
    $upload_params->setFileUri($file_name_unic)
      ->setFileType('xml')
      ->setApproved(0);

    if ($this->settingsHandler->getAutoAuthorizeContent()) {
      $upload_params->setLocalesToApprove($locales_to_approve)
        ->setOverwriteApprovedLocales(0);
    }
    if ($this->settingsHandler->getCallbackUrlUse()) {
      $upload_params->setCallbackUrl($this->settingsHandler->getCallbackUrl());
    }
    $upload_params = $upload_params->buildParameters();

    $upload_result = $this->api->uploadFile($file_path, $upload_params);
    $upload_result = json_decode($upload_result);

    if ($this->api->getCodeStatus() == 'SUCCESS') {

      $this->logger->setMessage('Smartling uploaded @file_name for locales: @locales')
        ->setVariables(array(
          '@file_name' => $file_name_unic,
          '@locales' => implode('; ', $locales_to_approve),
        ))
        ->setLink(l(t('View file'), $file_path))
        ->execute();

      return SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE;
    }
    elseif (is_object($upload_result)) {
      foreach ($upload_params as $param_name => $value) {
        $upload_params[$param_name] = $param_name . ' => ' . $value;
      }
      $this->logger->setMessage('Smartling failed to upload xml file: <br/>
          Project Id: @project_id <br/>
          Action: upload <br/>
          URI: @file_uri <br/>
          Error: response code -> @code and message -> @message
          Upload params: @upload_params')
        ->setVariables(array(
          '@project_id' => $this->settingsHandler->getProjectId(),
          '@file_uri' => $file_path,
          '@code' => $upload_result->response->code,
          '@message' => $upload_result->response->messages[0],
          '@upload_params' => implode(' | ', $upload_params),
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->execute();
    }

    return SMARTLING_STATUS_EVENT_FAILED_UPLOAD;
  }
}

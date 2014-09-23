<?php

/**
 * @file
 * Smartling settings handler.
 */

namespace Drupal\smartling\ApiWrapper;

use Drupal\smartling\ApiWrapperInterface;
use Drupal\smartling\Log\SmartlingLog;
use Drupal\smartling\Settings\SmartlingSettingsHandler;
use SmartlingAPI;

/**
 * Class SmartlingApiWrapper.
 */
class SmartlingApiWrapper implements ApiWrapperInterface {

  /**
   * @var SmartlingSettingsHandler
   */
  protected $settingsHandler;

  /**
   * @var SmartlingLog
   */
  protected $logger;

  /**
   * @var SmartlingAPI
   */
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
   *
   * @param SmartlingSettingsHandler $settings_handler
   * @param SmartlingLog $logger
   */
  public function __construct(SmartlingSettingsHandler $settings_handler, SmartlingLog $logger) {
    $this->settingsHandler = $settings_handler;
    $this->logger = $logger;

    $this->setApi(new SmartlingAPI($settings_handler->getApiUrl(), $settings_handler->getKey(), $settings_handler->getProjectId(), SMARTLING_PRODUCTION_MODE));
  }

  /**
   * {@inheritdoc}
   */
  public function setApi(\SmartlingAPI $api) {
    $this->api = $api;
  }

  /**
   * {@inheritdoc}
   */
  public function downloadFile($entity, $link_to_entity) {
    $entity_type = $entity->entity_type;
    $d_locale = $entity->target_language;
    $file_name_unic = $entity->file_name;
    $file_path = $this->settingsHandler->getDir($entity->file_name);

    $retrieval_type = $this->settingsHandler->variableGet('smartling_retrieval_type', 'published');
    $download_param = array(
      'retrievalType' => $retrieval_type,
    );

    $this->logger->setMessage('Smartling queue start download xml file and update fields for @entity_type id - @rid, locale - @locale.')
      ->setVariables(array(
        '@entity_type' => $entity_type,
        '@rid' => $entity->rid,
        '@locale' => $entity->target_language,
      ))
      ->setLink(l(t('View file'), $file_path))
      ->execute();

    $s_locale = $this->convertLocaleDrupalToSmartling($d_locale);
    // Try to download file.
    $download_result = $this->api->downloadFile($file_name_unic, $s_locale, $download_param);

    if (isset($download_result->response->code)) {
      $download_result = json_decode($download_result);

      $code = '';
      $messages = array();
      if (isset($download_result->response)) {
        $code =  isset($download_result->response->code) ? $download_result->response->code : array();
        $messages = isset($download_result->response->messages) ? $download_result->response->messages : array();
      }


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
          '@code' => $code,
          '@message' => implode(' || ', $messages),
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      return FALSE;
    }

    return $download_result;
  }

  /**
   * {@inheritdoc}
   */
  public function getStatus($entity, $link_to_entity) {
    $error_result = NULL;

    if ($entity === FALSE) {
      $this->logger->setMessage('Smartling checks status for id - @rid is FAIL! Smartling entity not exist.')
        ->setVariables(array('@rid' => $entity->rid))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      return $error_result;
    }

    if ($entity->progress == 100) {
      return $error_result;
    }

    $file_name = $entity->file_name;
    $file_name_unic = $entity->file_name;
    $file_uri = smartling_clean_filename($this->settingsHandler->getDir() . '/' . $file_name, TRUE);

    $s_locale = $this->convertLocaleDrupalToSmartling($entity->target_language);
    // Try to retrieve file status.
    $json = $this->api->getStatus($file_name_unic, $s_locale);
    $status_result = json_decode($json);

    if ($status_result === NULL) {
      $this->logger->setMessage('File status commend: downloaded json is broken. JSON: @json')
        ->setVariables(array(
          '@json' => $json,
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->execute();
      return $error_result;
    }

    // This is a get status.
    if (($this->api->getCodeStatus() != 'SUCCESS') || !isset($status_result->response->data)) {
      $code = '';
      $messages = array();
      if (isset($status_result->response)) {
        $code =  isset($status_result->response->code) ? $status_result->response->code : array();
        $messages = isset($status_result->response->messages) ? $status_result->response->messages : array();
      }

      $this->logger->setMessage('Smartling checks status for @entity_type id - @rid: <br/>
      Project Id: @project_id <br/>
      Action: status <br/>
      URI: @file_uri <br/>
      Locale: @d_locale <br/>
      Error: response code -> @code and message -> @message')
        ->setVariables(array(
          '@entity_type' => $entity->entity_type,
          '@rid' => $entity->rid,
          '@project_id' => $this->settingsHandler->getProjectId(),
          '@file_uri' => $file_name_unic,
          '@d_locale' => $entity->target_language,
          '@code' => $code,
          '@message' => implode(' || ', $messages),
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->setLink($link_to_entity)
        ->execute();

      return $error_result;
    }

    $this->logger->setMessage('Smartling checks status for @entity_type id - @rid (@d_locale). approvedString = @as, completedString = @cs')
      ->setVariables(array(
        '@entity_type' => $entity->entity_type,
        '@rid' => $entity->rid,
        '@d_locale' => $entity->target_language,
        '@as' => $status_result->response->data->approvedStringCount,
        '@cs' => $status_result->response->data->completedStringCount,
      ))
      ->setLink(l(t('View file'), $file_uri))
      ->execute();

    // If true, file translated.
    $response_data = $status_result->response->data;
    $progress = ($response_data->approvedStringCount == $response_data->completedStringCount || $response_data->approvedStringCount == 0) ?
      100 : (int) (($response_data->completedStringCount / $response_data->approvedStringCount) * 100);
    $entity->download = 0;
    $entity->progress = $progress;
    $entity->status = SMARTLING_STATUS_IN_TRANSLATE;

    return array(
      'entity_data' => $entity,
      'response_data' => $status_result->response->data,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function testConnection($locales) {
    $result = array();

    foreach ($locales as $key => $locale) {
      if ($locale !== 0 && $locale == $key) {
        $s_locale = $this->convertLocaleDrupalToSmartling($locale);
        // Init api object.
        $this->api->getList($s_locale, array('limit' => 1));

        $result[$s_locale] = $this->api->getCodeStatus() == 'SUCCESS';
      }
    }

    return $result;
  }

  /**
   * {@inheritdoc}
   */
  public function uploadFile($file_path, $file_name_unic, $locales) {
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

      $code = '';
      $messages = array();
      if (isset($upload_result->response)) {
        $code =  isset($upload_result->response->code) ? $upload_result->response->code : array();
        $messages = isset($upload_result->response->messages) ? $upload_result->response->messages : array();
      }

      $this->logger->setMessage('Smartling failed to upload xml file: <br/>
          Project Id: @project_id <br/>
          Action: upload <br/>
          URI: @file_uri <br/>
          Error: response code -> @code and message -> @message
          Upload aparms: @upload_params')
        ->setVariables(array(
          '@project_id' => $this->settingsHandler->getProjectId(),
          '@file_uri' => $file_path,
          '@code' => $code,
          '@message' => implode(' || ', $messages),
          '@upload_params' => implode(' | ', $upload_params),
        ))
        ->setConsiderLog(FALSE)
        ->setSeverity(WATCHDOG_ERROR)
        ->execute();
    }

    return SMARTLING_STATUS_EVENT_FAILED_UPLOAD;
  }

}

<?php

/**
 * @file
 * Contains Drupal\smartling\ApiWrapperInterface.
 */

namespace Drupal\smartling;

interface ApiWrapperInterface {

  /**
   * Set Smartling API instance.
   *
   * @param \SmartlingAPI $api
   *   Smartling API object from Smartling PHP SDK.
   */
  public function setApi(\SmartlingAPI $api);

  /**
   * Download file from service.
   *
   * @param object $entity
   *   Smartling transaction entity.
   * @param string $link_to_entity
   *   Link to entity.
   *
   * @return \DOMDocument|boolean
   *   Return xml dom from downloaded file.
   */
  public function downloadFile($entity, $link_to_entity);

  /**
   * Get status of given entity's translation progress.
   *
   * @param object $entity
   *   Smartling transaction entity.
   * @param string $link_to_entity
   *   Link to entity.
   *
   * @return array|null
   *   Return status.
   */
  public function getStatus($entity, $link_to_entity);

  /**
   * Test Smartling API instance init and connection to Smartling server.
   *
   * @param array $locales
   *   List of locales in Drupal format.
   *
   * @return array
   *   If connections were successful for each locale.
   */
  public function testConnection($locales);

  /**
   * Upload local file to Smartling for translation.
   *
   * @param string $file_path
   *   Real path to file.
   * @param string $file_name_unic
   *   Unified file name.
   * @param array $locales
   *   List of locales in Drupal format.
   *
   * @return string
   *   SMARTLING_STATUS_EVENT_UPLOAD_TO_SERVICE | SMARTLING_STATUS_EVENT_FAILED_UPLOAD
   */
  public function uploadFile($file_path, $file_name_unic, $locales);

}

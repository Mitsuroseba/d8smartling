<?php

/**
 * @file
 * Smartling context base.
 */

include_once 'token_storage.inc';

/**
 * Class ContextBase.
 */
class ContextBase {

  protected $storage;

  protected static $authError = array(
    "response" => array(
      "code" => "AUTHENTICATION_ERROR",
      "data" => array("baseUrl" => NULL, "body" => NULL, "headers" => NULL),
      "messages" => array("Authentication token is empty or invalid."),
    ),
  );
  protected static $validationError = array(
    "response" => array(
      "code" => "VALIDATION_ERROR",
      "data" => array("baseUrl" => NULL, "body" => NULL, "headers" => NULL),
      "messages" => array("node ID is invalid."),
    ),
  );

  protected static $uriMissingError = array(
    "response" => array(
      "code" => "VALIDATION_ERROR",
      "data" => array("baseUrl" => NULL, "body" => NULL, "headers" => NULL),
      "messages" => array("fileUri parameter is missing."),
    ),
  );

  /**
   * Construct.
   *
   * @param TokenStorage $storage
   *   TokenStorage object.
   * @param string $url
   *   Url.
   */
  public function __construct(TokenStorage $storage, $url) {
    $this->storage = $storage;

    if ($url[strlen($url) - 1] != '/') {
      $url .= '/';
    }
    $this->site_url = $url;
  }

  /**
   * Get site url.
   *
   * @return string
   *   Return site url.
   */
  public function getSiteUrl() {
    return $this->site_url;
  }
}

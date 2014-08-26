<?php

/**
 * @file
 * Smartling context html.
 */

include_once 'context_base.inc';

/**
 * Smartling context html.
 *
 * @return string
 *   Return html.
 */
function smartling_translation_context_html() {
  global $base_url;

  $params = drupal_get_query_parameters();
  $html_getter = new SmartlingContextHTML(new SmartlingTokenStorage(), $base_url);

  return $html_getter->getHtml($params['fileUrl'], $params['token']);
}

/**
 * Class SmartlingContextHTML.
 */
class SmartlingContextHTML extends SmartlingContextBase {

  /**
   * Construct.
   *
   * @param SmartlingTokenStorage $storage
   *   SmartlingTokenStorage object.
   * @param string $site_url
   *   Site url.
   */
  public function __construct(SmartlingTokenStorage $storage, $site_url) {
    parent::__construct($storage, $site_url);
  }

  /**
   * Get html.
   *
   * @param string $url
   *   Url.
   * @param mixed $token
   *   Token.
   *
   * @return array
   *   Return response array.
   */
  public function getHtml($url, $token) {
    if (empty($url)) {
      return self::$uriMissingError;
    }

    $cookies = $this->storage->getToken($token);
    if (!$cookies) {
      return self::$authError;
    }

    $crl = curl_init();
    curl_setopt($crl, CURLOPT_URL, $this->getSiteUrl() . $url);
    curl_setopt($crl, CURLOPT_HEADER, 1);
    curl_setopt($crl, CURLOPT_FOLLOWLOCATION, 1);
    curl_setopt($crl, CURLOPT_COOKIE, $cookies);
    curl_setopt($crl, CURLOPT_RETURNTRANSFER, TRUE);
    $response = curl_exec($crl);
    // $headers = curl_getinfo($crl);
    curl_close($crl);
    $response = explode("\r\n\r\n", $response);

    $res = array(
      "response" => array(
        "code" => "SUCCESS",
        "data" => array(
          "body" => $response[1],
          "headers" => $response[0],
          "baseUrl" => $url,
        ),
        "messages" => array(),
      ),
    );

    return $res;
  }
}

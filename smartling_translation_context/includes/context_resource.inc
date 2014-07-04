<?php

/**
 * @file
 * Smartling context resource.
 */

include_once 'context_base.inc';

/**
 * Smartling context resource.
 *
 * @return string
 *   Return html.
 */
function smartling_translation_context_resource() {
  global $base_url;

  $params = drupal_get_query_parameters();
  $html_getter = new ContextHTML(new TokenStorage(), $base_url);

  return $html_getter->getHtml($params['fileUrl'], $params['token']);
}

/**
 * Class ContextResource.
 */
class ContextResource extends ContextBase {

  /**
   * Construct.
   *
   * @param TokenStorage $storage
   *   TokenStorage object.
   * @param string $site_url
   *   Site url.
   */
  public function __construct(TokenStorage $storage, $site_url) {
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
   * @return array|mixed
   *   Return html.
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
    curl_setopt($crl, CURLOPT_FOLLOWLOCATION, 1);
    curl_setopt($crl, CURLOPT_COOKIE, $cookies);
    curl_setopt($crl, CURLOPT_RETURNTRANSFER, TRUE);
    $html = curl_exec($crl);
    // $headers = curl_getinfo($crl);
    curl_close($crl);

    return $html;
  }

}

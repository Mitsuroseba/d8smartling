<?php

/**
 * @file
 * Smartling context token storage.
 */

/**
 * Class SmartlingTokenStorage.
 */
class SmartlingTokenStorage {

  /**
   * Get token.
   *
   * @param mixed $id
   *   The cache ID of the data to retrieve.
   *
   * @return string
   *   Return token.
   */
  public static function getToken($id) {
    $data = cache_get($id);
    return ($data->expire > time()) ? $data->data : '';
  }

  /**
   * Set token.
   *
   * @param mixed $id
   *   Id.
   * @param mixed $value
   *   Token value.
   * @param string $date
   *   Date.
   *
   * @return mixed
   *   Return token result.
   */
  public static function setToken($id, $value, $date) {
    return cache_set($id, $value, 'cache', $date);
  }
}

<?php

/**
 * @file
 * Smartling context authorize.
 */

include_once 'token_storage.inc';

/**
 * Smartling context authorize.
 *
 * @return array
 *   Return response array.
 */
function smartling_translation_context_authorize() {
  global $user;

  $project_id = variable_get('smartling_project_id', '');
  $api_key = variable_get('smartling_key', '');
  $auth = new ContextAuthorize(new TokenStorage(), $user, $project_id, $api_key);

  $params = drupal_get_query_parameters();
  return $auth->authorize($params['projectId'], $params['apiKey'], 'admin');
}

/**
 * Smartling context token expire.
 *
 * @return array
 *   Return response array.
 */
function smartling_translation_context_token_expire() {
  global $user;

  $project_id = variable_get('smartling_project_id', '');
  $api_key = variable_get('smartling_key', '');
  $auth = new ContextAuthorize(new TokenStorage(), $user, $project_id, $api_key);

  $params = drupal_get_query_parameters();
  return $auth->expireToken($params['token']);
}

class ContextAuthorize {
  protected $tokenStorage;
  protected $user;
  protected $apiKey;
  protected $projectId;

  protected static $tokenTtl = 3600;
  protected static $authError = array(
    'response' => array(
      'data' => NULL,
      'code' => "AUTHENTICATION_ERROR",
      "messages" => array("Bad credentials"),
    ),
  );
  protected static $validationError = array(
    'response' => array(
      'data' => NULL,
      'code' => "AUTHENTICATION_ERROR",
      "messages" => array("Bad credentials"),
    ),
  );

  /**
   * Construct.
   *
   * @param TokenStorage $storage
   *   Storage object.
   * @param object $user
   *   User object.
   * @param string $project_id
   *   Project id.
   * @param string $api_key
   *   API key.
   */
  public function __construct(TokenStorage $storage, $user, $project_id, $api_key) {
    $this->tokenStorage = $storage;
    $this->user = $user;

    $this->projectId = $project_id;
    $this->apiKey = $api_key;
  }

  /**
   * Generate token.
   *
   * @return int
   *   Return random int.
   */
  protected function tokenGenerate() {
    return rand();
  }

  /**
   * Authorize.
   *
   * @param string $project_id
   *   Project id.
   * @param string $api_key
   *   API key.
   * @param string $name
   *   Name.
   *
   * @return array
   *   Return response array.
   */
  public function authorize($project_id, $api_key, $name) {
    if ($project_id != $this->projectId || $api_key != $this->apiKey) {
      return self::$validationError;
    }

    $res = $this->authByName($name);
    if (!$res) {
      return self::$authError;
    }

    $token = $this->tokenGenerate();

    $this->tokenStorage->setToken($token, $res, time() + self::$tokenTtl);

    return array(
      'response' => array(
        'data' => array('token' => $token),
        'code' => "SUCCESS",
        "messages" => array(),
      ),
    );
  }

  /**
   * Expire Token.
   *
   * @param mixed $token
   *   Token.
   *
   * @return array
   *   Return response array.
   */
  public function expireToken($token) {
    if ($data = $this->tokenStorage->getToken($token)) {
      $this->tokenStorage->setToken($token, NULL, time() - self::$tokenTtl);
      return array(
        'response' => array(
          'data' => NULL,
          'code' => "SUCCESS",
          "messages" => array(),
        ),
      );
    }

    return array(
      'response' => array(
        'data' => NULL,
        'code' => "VALIDATION_ERROR",
        "messages" => array('Invalid parameters'),
      ),
    );
  }

  /**
   * Auth by name.
   *
   * @param string $name
   *   Name.
   *
   * @return int|string
   *   Return result.
   */
  protected function authByName($name) {
    $user = $this->user;

    if ($user->uid) {
      module_invoke_all('user_logout', $user);
    }
    if (isset($name) && $account = user_load_by_name($name)) {
      $old_uid = $user->uid;
      $user = $account;
      $user->timestamp = time() - 9999;
      if (!$old_uid) {
        // Switch from anonymous to authorized.
        drupal_session_regenerate();
      }
      $edit = array();
      user_module_invoke('login', $edit, $user);
      return session_name() . "=" . session_id();
    }
    return 0;
  }
}

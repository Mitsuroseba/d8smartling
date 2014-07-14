<?php

/**
 * @file
 * Smartling log.
 */

namespace Drupal\smartling\Log;

/**
 * Class SmartlingLog.
 */
class SmartlingLog {

  public $message;
  public $considerLog;
  public $variables;
  public $severity;
  public $link;

  /**
   * Initialize.
   */
  public function __construct() {
    $this->message = '';
    $this->considerLog = TRUE;
    $this->variables = array();
    $this->severity = WATCHDOG_INFO;
    $this->link = NULL;
  }

  /**
   * Set message for watchdog.
   *
   * @param string $message
   *   Message for watchdog.
   *
   * @return SmartlingLog
   *   Return object SmartlingLog.
   */
  public function setMessage($message) {
    $this->message = (string) $message;
    return $this;
  }

  /**
   * Set ConsiderLog for smartling watchdog.
   *
   * @param bool $consider_log
   *   If TRUE need look in smartling log mode settings.
   *
   * @return SmartlingLog
   *   Return object SmartlingLog.
   */
  public function setConsiderLog($consider_log = TRUE) {
    $this->considerLog = (bool) $consider_log;
    return $this;
  }

  /**
   * Set variables for watchdog.
   *
   * @param array $variables
   *   Variables for watchdog.
   *
   * @return SmartlingLog
   *   Return object SmartlingLog.
   */
  public function setVariables(array $variables = array()) {
    $this->variables = (array) $variables;
    return $this;
  }

  /**
   * Set severity for watchdog.
   *
   * @param int $severity
   *   Severity for watchdog level info.
   *
   * @return SmartlingLog
   *   Return object SmartlingLog.
   */
  public function setSeverity($severity = WATCHDOG_INFO) {
    $this->severity = $severity;
    return $this;
  }

  /**
   * Set link for watchdog.
   *
   * @param string $link
   *   Link for watchdog.
   *
   * @return SmartlingLog
   *   Return object SmartlingLog.
   */
  public function setLink($link = NULL) {
    $this->link = $link;
    return $this;
  }

  /**
   * Execute.
   *
   * @return bool
   *   Return TRUE if executed.
   */
  public function execute() {
    $result = FALSE;

    if ($this->considerLog) {
      if (smartling_settings_get_handler()->getLogMode()) {
        watchdog('smartling', $this->message, $this->variables, $this->severity, $this->link);
        self::__construct();
        $result = TRUE;
      }
    }
    else {
      watchdog('smartling', $this->message, $this->variables, $this->severity, $this->link);
      self::__construct();
      $result = TRUE;
    }
    return $result;
  }

}

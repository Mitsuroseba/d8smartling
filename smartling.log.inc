<?php

/**
 * @file
 * Smartling log.
 */

/**
 * Class SmartlingLog.
 */
class SmartlingLog {

  public $message;
  public $consider_log;
  public $variables;
  public $severity;
  public $link;

  /**
   * Initialize.
   */
  public function __construct() {
    $this->message = '';
    $this->consider_log = TRUE;
    $this->variables = array();
    $this->severity = WATCHDOG_INFO;
    $this->link = NULL;
  }

  public function setMessage($message) {
    $this->message = (string) $message;
    return $this;
  }

  public function setConsiderLog($consider_log = TRUE) {
    $this->consider_log = (bool) $consider_log;
    return $this;
  }

  public function setVariables($variables = array()) {
    $this->variables = (array) $variables;
    return $this;
  }

  public function setSeverity($severity = WATCHDOG_INFO) {
    $this->severity = $severity;
    return $this;
  }

  public function setLink($link = NULL) {
    $this->link = $link;
    return $this;
  }

  public function execute() {
    $result = FALSE;

    if ($this->consider_log) {
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

<?php

namespace Drupal\smartling\Log;

class DevNullLogger implements LoggerInterface {

  public function emergency($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('emergency', $message, $context);
  }

  public function alert($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('alert', $message, $context);
  }

  public function critical($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('critical', $message, $context);
  }

  public function error($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('error', $message, $context);
  }

  public function warning($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('warning', $message, $context);
  }

  public function notice($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('notice', $message, $context);
  }

  public function info($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('info', $message, $context);
  }

  public function debug($message, array $context = array(), $ignore_settings = FALSE) {
    return $this->log('debug', $message, $context);
  }

  public function log($level, $message, array $context = array(), $ignore_settings = FALSE) {
    return TRUE;
  }
}
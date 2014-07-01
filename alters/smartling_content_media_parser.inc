<?php

/**
 * @file
 * Class SmartlingMediaParser.
 */

require_once dirname(__FILE__) . '/smartling_content_base_parser.inc';

/**
 * A parser for the Drupal media module and its jsons inside text fields.
 *
 * For example: <p>[[{"fid":"41","view_mode":"default",
 * "fields":{"format":"default"},"type":"media",
 * "attributes":{"class":"media-element file-default"}}]]</p>.
 */
class SmartlingContentMediaParser extends SmartlingContentBaseParser {
  protected $regexp = '~(\[\[\{.*"fid":.+?\}\]\])~i';

  /**
   * Adds some context to the string that is being processed.
   *
   * @param array $matches
   *   Array matches.
   *
   * @return array
   *   Context array.
   */
  protected function getContext($matches) {
    foreach ($matches as $k => $v) {
      $matches[$k] = json_decode($v);
    }

    return $matches;
  }

}
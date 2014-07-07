<?php

/**
 * @file
 * Smartling content base parser.
 */

namespace Smartling\Alters;

use Smartling\Alters\ISmartlingContentParser;

/**
 * Abstract class SmartlingContentBaseParser.
 */
abstract class SmartlingContentBaseParser implements ISmartlingContentParser {
  protected $regexp = '';
  protected $processors;
  protected $field_name;
  protected $entity;

  public function __construct(array $processors) {
    $this->processors = $processors;
  }


  /*
   * Determines additional context based on the content
   *
   * @param array $matches Array of strings that matched the regexp.
   *
   * @return array
   */
  protected abstract function getContext($matches);


  /*
   * Processes each item that was found by a regexp in a parse method.
   *
   * @return string
   */
  protected function processorExecutor($match) {
    $context = $this->getContext($match);

    foreach($this->processors as $processor) {
      $processor->process($match, $context, $this->lang, $this->field_name, $this->entity);
    }

    return $match[1];
  }

  /*
   * Parses the translated content and passes items
   * that were found to a processor.
   *
   * @param string $content
   * @param string $lang
   * @paramm string $field_name
   * @param object $entity
   *
   * @return string
   */
  public function parse($content, $lang, $field_name, $entity) {
    $this->field_name = $field_name;
    $this->entity = $entity;
    $this->lang = $lang;

    $content = preg_replace_callback($this->regexp, array($this, 'processorExecutor'), $content);

    return $content;
  }
}
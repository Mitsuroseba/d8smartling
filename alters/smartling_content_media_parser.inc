<?php
require_once(dirname(__FILE__) . '/smartling_content_parser_interface.inc');

/*
 * A parser for the Drupal media module and its jsons inside text fields.
 * For example: <p>[[{"fid":"41","view_mode":"default","fields":{"format":"default"},"type":"media","attributes":{"class":"media-element file-default"}}]]</p>
 */
class SmartlingMediaParser implements ISmartlingContentParser {
  protected $regexp = array('~(\[\[\{"fid":.+\}\]\]~i', '~(\[\[\{&quot;fid&quot;:.+\}\]\]~i');
  protected $processors;
  protected $field_name;
  protected $entity;
  protected $lang;

  public function __construct(array $processors) {
    $this->processors = $processors;
  }

  /*
   * Adds some context to the string that is being processed
   */
  protected function getContext($matches) {
    foreach($matches as $k => $v) {
      $matches[$k] = json_decode(htmlspecialchars_decode($v));
    }

    return $matches;
  }

  /*
   * Processes each item that was found by a regexp in a parse method.
   *
   * @param array $match Array of string found by regexp.
   *        for example: array([0] => 'href="/admin"', [1] => 'href', [2] => '/admin')
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

  public function parse($content, $lang, $field_name, $entity) {
    $this->field_name = $field_name;
    $this->entity = $entity;
    $this->lang = $lang;

    $content = preg_replace_callback($this->regexp, array($this, 'processorExecutor'), $content);

    return $content;
  }
}
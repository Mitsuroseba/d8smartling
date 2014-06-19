<?php

/**
 * @file
 * Class SmartlingMediaParser.
 */

require_once dirname(__FILE__) . '/smartling_content_parser_interface.inc';

/**
 * A parser for the Drupal media module and its jsons inside text fields.
 *
 * For example: <p>[[{"fid":"41","view_mode":"default",
 * "fields":{"format":"default"},"type":"media",
 * "attributes":{"class":"media-element file-default"}}]]</p>.
 */
class SmartlingMediaParser implements SmartlingIContentParserInterface {

  protected $regexp = array('~(\[\[\{"fid":.+\}\]\]~i', '~(\[\[\{&quot;fid&quot;:.+\}\]\]~i');
  protected $processors;
  protected $fieldName;
  protected $entity;
  protected $lang;

  /**
   * Class construct.
   *
   * @param array $processors
   *   Proessors.
   */
  public function __construct(array $processors) {
    $this->processors = $processors;
  }

  /**
   * Adds some context to the string that is being processed.
   *
   * @param array $matches
   *   Array matches.
   *
   * @return array
   *   Context array.
   */
  protected function getContext(array $matches) {
    foreach ($matches as $k => $v) {
      $matches[$k] = json_decode(htmlspecialchars_decode($v));
    }

    return $matches;
  }

  /**
   * Processes each item that was found by a regexp in a parse method.
   *
   * @param array $match
   *   Array of string found by regexp. For example:
   *   array([0] => 'href="/admin"', [1] => 'href', [2] => '/admin').
   *
   * @return string
   *   Return match.
   */
  protected function processorExecutor(array $match) {
    $context = $this->getContext($match);

    foreach ($this->processors as $processor) {
      $processor->process($match, $context, $this->lang, $this->fieldName, $this->entity);
    }

    return $match[1];
  }

  /**
   * Parse.
   *
   * @param array $content
   *   Context array.
   * @param string $lang
   *   Locale in drupal format (ru, en).
   * @param string $field_name
   *   Field name.
   * @param object $entity
   *   Entity object.
   *
   * @return array
   *   Return content.
   */
  public function parse(array $content, $lang, $field_name, $entity) {
    $this->fieldName = $field_name;
    $this->entity = $entity;
    $this->lang = $lang;

    $content = preg_replace_callback($this->regexp, array($this, 'processorExecutor'), $content);

    return $content;
  }

}

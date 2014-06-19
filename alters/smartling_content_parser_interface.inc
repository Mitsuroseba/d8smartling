<?php

/**
 * @file
 * Interface SmartlingIContentParserInterface.
 */

/**
 * Parses the translated content for the needed parts (like link addresses etc).
 *
 * And passes it to the relevant processors.
 */
interface SmartlingIContentParserInterface {

  /**
   * Array of objects that implement SmartlingIContentProcessorInterface.
   *
   * @param array $processors
   *   Processors.
   */
  public function __construct(array $processors);

  /**
   * Parses string content after the translation is made.
   *
   * @param array $content
   *   Context array.
   * @param string $lang
   *   Locale in drupal format (ru, en).
   * @param string $field_name
   *   Field name.
   * @param object $entity
   *   Entity object.
   */
  public function parse(array $content, $lang, $field_name, $entity);
}

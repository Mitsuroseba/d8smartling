<?php

/**
 * @file
 * Interface IContentParserSmartlingInterface.
 */

/**
 * Parses the translated content for the needed parts (like link addresses etc).
 *
 * And passes it to the relevant processors.
 */
interface IContentParserSmartlingInterface {

  /**
   * Array of objects that implement IContentProcessorSmartlingInterface.
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
  public function parse($content, $lang, $field_name, $entity);
}

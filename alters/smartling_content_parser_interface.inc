<?php

/*
 * Parses the translated content for the needed parts (like link addresses etc)
 * And passes it to the relevant processors.
 */
interface ISmartlingContentParser {
  /*
 * @param array $processors Array of objects that implement ISmartlingContentProcessor
 */
  function __construct(array $processors);

  /*
   * Parses string content after the translation is made.
   *
   * @param string $content
   * @param string $lang
   * @param string $field_name
   * @param object $entity
   *
   * @return string
   */
  function parse($content, $lang, $field_name, $entity);
}

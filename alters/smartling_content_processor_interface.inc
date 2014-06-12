<?php
/*
 * Content Processor interface.
 *
 * It's implementations allow to change translated content. For example, to rewrite
 * paths and links to respect language translated nodes. Or to substitute files from
 * media module to translated ones.
 */
interface ISmartlingContentProcessor {
  /*
   * Changes string content according to the other params.
   *
   * @param string $item
   * @param mixed $context
   * @param string $lang
   * @param string $field_name
   * @param object $entity
   *
   * @return void
   */
  public function process(&$item, $context, $lang, $field_name, $entity);
}
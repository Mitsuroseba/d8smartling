<?php

/**
 * @file
 * Handler code.
 */

/**
 * Handler for field title.
 */
class SmartlingHandlerFieldSmartlingTitle extends views_handler_field {

  /**
   * Render.
   *
   * @param object $values
   *   Smartling entity.
   *
   * @return string
   *   Return title.
   */
  public function render($values) {
    $entity_type = $values->smartling_entity_data_entity_type;
    $rid = $values->smartling_entity_data_rid;

    $function_uri = $entity_type . '_uri';
    $function_load = $entity_type . '_load';
    if (function_exists($function_load)) {
      $entity = $function_load($rid);
    }
    if (function_exists($function_uri)) {
      $uri = $function_uri($entity);
      $path = $uri['path'];
    }

    switch ($entity_type) {
      case 'node':
        $link = l(t('!title', array('!title' => $entity->title)), $path);
        break;

      case 'taxonomy_term':
      case 'user':
        $link = l(t('!title', array('!title' => $entity->name)), $path);
        break;

      case 'comment':
        $link = l(t('!title', array('!title' => $entity->subject)), $path);
        break;

      default:
        $link = '-';
        break;
    }
    return $link;
  }

}

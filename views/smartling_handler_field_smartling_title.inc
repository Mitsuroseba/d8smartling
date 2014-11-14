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
    } else {
      $entity = entity_load($entity_type, array($rid));
      $entity = $entity[$rid];
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

      case 'smartling_interface_entity':
        $link = t('!title', array('!title' => $entity->textgroup_name));
        break;

      default:
        $title = $entity_type . ' ' . $rid;
        if (isset($entity->title)) {
          $title = $entity->title;
        }
        elseif ($entity->name) {
          $title = $entity->name;
        }

        $link = t('!title', array('!title' => $title));
        break;
    }
    return $link;
  }

}

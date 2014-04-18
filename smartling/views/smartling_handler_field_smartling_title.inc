<?php

/**
 * @file
 * Handler code.
 */

/**
 * Handler for field title.
 */
class smartling_handler_field_smartling_title extends views_handler_field {

  function render($values) {
    $entity_type = $values->smartling_entity_data_entity_type;
    $rid = $values->smartling_entity_data_rid;

    switch ($entity_type) {
      case 'node':
        $node = node_load($rid);
        $link = l(t('!title', array('!title' => $node->title)), 'node/' . $rid);
        break;

      case 'taxonomy_term':
        $term = taxonomy_term_load($rid);
        $link = l(t('!title', array('!title' => $term->name)), 'taxonomy/term/' . $rid);
        break;

      case 'comment':
        $comment = comment_load($rid);
        $link = l(t('!title', array('!title' => $comment->subject)), 'comment/' . $rid);
        break;

      case 'user':
        $account = user_load($rid);
        $link = l(t('!title', array('!title' => $account->name)), 'user/' . $rid);
        break;

      default:
        $link = '-';
        break;
    }
    return $link;
  }

}

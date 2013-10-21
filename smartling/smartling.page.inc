<?php

/**
 * @file
 * Smartling Tab for Nodes
 */

/**
 * Page callback for the Smartling local task on node detail pages.
 *
 * @return array
 *   A Drupal render array of the page contents.
 */
function smartling_pm($node) {
  $output = array(
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'smartling') . '/js/smartling_node_settings.js' => array(
          'type' => 'file',
        ),
      ),
    ),
  );

  //dsm($node, '$node');

  /*
    // node translation support
    if ($node->tnid != 0 && $node->tnid != $node->nid) {
    $output[] = array(
    '#markup' => 'This is a translated node. Go to the ' . l('source node', 'node/' . $node->tnid . '/lingotek_pm') . ' to manage the translations.'
    );
    return $output;
    }

    // node disabled support
    if ($sync_status == LingotekSync::STATUS_DISABLED) {
    $output[] = array(
    '#markup' => 'Smartling Translation is currently disabled for this node.'
    );
    return $output;
    }
   */
  if (TRUE) {

    if (TRUE) {
      $content_push_form = drupal_get_form('smartling_get_node_settings_form', $node);
      $output['content_push'] = array(
        '#markup' => drupal_render($content_push_form),
      );
    }
  }
  else {
    $output[] = array(
      '#markup' => '<p class="help">' . t('This node is not compatible with Smartling translation.') . '</p>'
    );
  }

  return $output;
}

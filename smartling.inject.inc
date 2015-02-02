<?php
/**
 * @file
 * Configures the container.
 */

use Symfony\Component\DependencyInjection\ContainerBuilder;

/**
 * Implements hook_inject_build().
 */
function smartling_inject_build(ContainerBuilder $container) {
  $val = variable_get('smartling_retrieval_type', '');
  if ('pseudo_local' == $val) {
    //$obj = new stdClass();
    $container->setAlias('smartling.api_wrapper', 'smartling.local_api_wrapper');

//    $obj = $container->get('smartling.local_api_wrapper');
//    $container->set('smartling.api_wrapper', $obj);
  }
}

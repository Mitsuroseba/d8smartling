<?php

/**
 * @file
 * Contains Drupal\smartling\Processors\NodeProcessor.
 */

namespace Drupal\smartling\Processors;

class NodeProcessor extends GenericEntityProcessor {

  /**
   * {inheritdoc}
   *
   * @todo remove procedural code.
   */
  public function prepareDrupalEntity() {
    if (!$this->isOriginalEntityPrepared && smartling_nodes_method($this->entity->bundle)) {
      $this->isOriginalEntityPrepared = TRUE;
      // Translate subnode instead of main one.
      $this->ifFieldMethod = FALSE;
      $tnid = $this->contentEntity->tnid ?: $this->contentEntity->nid;
      $translations = translation_node_get_translations($tnid);
      if (isset($translations[$this->drupalTargetLocale])) {
        $this->entity->rid = $translations[$this->drupalTargetLocale]->nid;
        $this->contentEntity = node_load($this->entity->rid);
        $this->contentEntityWrapper->set($this->contentEntity);
      } else {
        // If node not exist, need clone.
        $node = clone $this->contentEntity;
        unset($node->nid);
        unset($node->vid);
        node_object_prepare($node);
        $node->language = $this->drupalTargetLocale;
        $node->uid = $this->entity->submitter;
        $node->tnid = $this->contentEntity->nid;

        $node_fields = field_info_instances('node', $this->contentEntity->type);
        foreach ($node_fields as $field) {
          $node->{$field['field_name']} = $this->contentEntity->{$field['field_name']};
        }

        $node->translation_source = $this->contentEntity;

        node_object_prepare($node);
        node_save($node);

        // Update reference to drupal content entity.
        $this->contentEntity = $node;
        $this->entity->rid = $node->nid;
      }
    }
  }

}
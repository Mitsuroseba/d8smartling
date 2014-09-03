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
    if (smartling_nodes_method($this->entity->bundle)) {
      // Translate subnode instead of main one.
      $this->ifFieldMethod = FALSE;
      $tnid = $this->contentEntity->tnid ?: $this->contentEntity->nid;
      $translations = translation_node_get_translations($tnid);
      if (isset($translations[$this->drupalTargetLocale])) {
        $this->entity->rid = $translations[$this->drupalTargetLocale]->nid;
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
          $field_info = field_info_field($field['field_name']);
          if (($field_info['type'] == 'taxonomy_term_reference') && ($field_info['translatable'] == '1')) {
            foreach ($this->contentEntity->{$field['field_name']} as $items) {
              foreach ($items as $index => $item) {
                $term = taxonomy_term_load($this->contentEntity->{$field['field_name']}[$this->contentEntity->language][$index]['tid']);
                if ($translation = i18n_taxonomy_term_get_translation($term, $this->drupalTargetLocale)) {
                  $node->{$field['field_name']}[$this->drupalTargetLocale][$index] = array('taxonomy_term' => $translation, 'tid' => $translation->tid,);
                }
                $field['settings']['options_list_callback'] = 'i18n_taxonomy_allowed_values';
              }
            }
          } else {
            $node->{$field['field_name']} = $this->contentEntity->{$field['field_name']};
          }
        }

        node_object_prepare($node);
        node_save($node);

        // Update reference to drupal content entity.
        $this->contentEntity = $node;
        $this->entity->rid = $node->nid;
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function updateDrupalTranslation() {
    if (smartling_nodes_method($this->entity->bundle)) {
      return;
    }

    parent::updateDrupalTranslation();
  }

}
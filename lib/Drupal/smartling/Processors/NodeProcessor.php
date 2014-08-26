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
  public function prepareOriginalEntity() {
    $this->originalEntity = node_load($this->entity->rid);

    if (smartling_nodes_method($this->entity->bundle)) {
      $this->ifFieldMethod = FALSE;
      $translations = translation_node_get_translations($this->originalEntity->tnid);
      if (isset($translations[$this->drupalLocale])) {
        $this->entity->rid = $translations[$this->drupalLocale]->nid;
      } else {
        // If node not exist, need clone.
        $node = clone $this->originalEntity;
        unset($node->nid);
        unset($node->vid);
        node_object_prepare($node);
        $node->language = $this->drupalLocale;
        $node->uid = $this->entity->submitter;
        $node->tnid = $this->originalEntity->nid;

        $node_fields = field_info_instances('node', $this->originalEntity->type);
        foreach ($node_fields as $field) {
          $field_info = field_info_field($field['field_name']);
          if (($field_info['type'] == 'taxonomy_term_reference') && ($field_info['translatable'] == '1')) {
            foreach ($this->originalEntity->{$field['field_name']} as $items) {
              foreach ($items as $index => $item) {
                $term = taxonomy_term_load($this->originalEntity->{$field['field_name']}[$this->originalEntity->language][$index]['tid']);
                if ($translation = i18n_taxonomy_term_get_translation($term, $this->drupalLocale)) {
                  $node->{$field['field_name']}[$this->drupalLocale][$index] = array('taxonomy_term' => $translation, 'tid' => $translation->tid,);
                }
                $field['settings']['options_list_callback'] = 'i18n_taxonomy_allowed_values';
              }
            }
          } else {
            $node->{$field['field_name']} = $this->originalEntity->{$field['field_name']};
          }
        }

        node_object_prepare($node);
        node_save($node);
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function updateTranslation() {
    if (($this->originalEntityType == 'node') && smartling_nodes_method($this->entity->bundle)) {
      return;
    }

    parent::updateTranslation();
  }

}
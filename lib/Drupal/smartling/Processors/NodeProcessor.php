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
  public function addTranslatedFieldsToNode($node){
    $field_values = array();
    foreach ($this->getTranslatableFields() as $field_name) {
      if (!empty($node->{$field_name}[LANGUAGE_NONE])) {
        $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $node, $this->drupalEntityType, $this->smartling_submission);
        $val = $fieldProcessor->cleanBeforeClone($node);
        if (!empty($val)) {
          $field_values[$field_name] = $val;
        }
      }
    }

    $this->entity_api_wrapper->nodeObjectPrepare($node);
    $this->entity_api_wrapper->entitySave('node', $node);

    foreach ($this->getTranslatableFields() as $field_name) {
      if (!empty($field_values[$field_name])) {
        $node->{$field_name} = $field_values[$field_name];
      }
    }

    foreach ($this->getTranslatableFields() as $field_name) {
      // Run all translatable fields through prepareBeforeDownload
      // to make sure that all related logic was triggered.
      if (!empty($this->contentEntity->{$field_name}[LANGUAGE_NONE])) {
        $fieldProcessor = $this->fieldProcessorFactory->getProcessor($field_name, $node, $this->drupalEntityType, $this->smartling_submission);
        // @TODO get rid of hardcoded language.
        $fieldProcessor->prepareBeforeDownload($this->contentEntity->{$field_name}[LANGUAGE_NONE]);
      }
    }

    return $node;
  }

  public function prepareDrupalEntity() {
    if (!$this->isOriginalEntityPrepared && $this->smartling_utils->isNodesMethod($this->smartling_submission->bundle)) {
      $this->isOriginalEntityPrepared = TRUE;
      // Translate subnode instead of main one.
      $this->ifFieldMethod = FALSE;
      $tnid = $this->contentEntity->tnid ?: $this->contentEntity->nid;
      $translations = translation_node_get_translations($tnid);
      if (isset($translations[$this->drupalTargetLocale])) {
        $this->smartling_submission->rid = $translations[$this->drupalTargetLocale]->nid;

        $node = $this->entity_api_wrapper->entityLoadSingle('node', $this->smartling_submission->rid); //node_load($this->smartling_submission->rid);
        $node->translation_source = $this->contentEntity;

        //$node = node_load($node->nid);
        $node = $this->entity_api_wrapper->entityLoadSingle('node', $node->nid);
        $node = $this->addTranslatedFieldsToNode($node);

        $this->contentEntity = $node;
        $this->contentEntityWrapper->set($this->contentEntity);
      } else {
        // If node not exist, need clone.
        $node = clone $this->contentEntity;
        unset($node->nid);
        unset($node->vid);
        $this->entity_api_wrapper->nodeObjectPrepare($node);
        $node->language = $this->drupalTargetLocale;
        $node->uid = $this->smartling_submission->submitter;
        $node->tnid = $this->contentEntity->nid;

        // @todo Do we need this? clone should do all the stuff.
        $node_fields = field_info_instances('node', $this->contentEntity->type);
        foreach ($node_fields as $field) {
          $node->{$field['field_name']} = $this->contentEntity->{$field['field_name']};
        }

        $node->translation_source = $this->contentEntity;

        $node = $this->addTranslatedFieldsToNode($node);
        $node = $this->entity_api_wrapper->entityLoadSingle('node', $node->nid);
        // Second saving is done for Field Collection field support
        // that need host entity id.
        //node_save($node);

        // Update reference to drupal content entity.
        $this->contentEntity = $node;
        $this->smartling_submission->rid = $node->nid;
      }
    }
  }

  public static function supportedType($bundle) {
    $transl_method = variable_get('language_content_type_' . $bundle, NULL);
    return in_array($transl_method, array(SMARTLING_NODES_METHOD_KEY, SMARTLING_FIELDS_METHOD_KEY));
  }
}
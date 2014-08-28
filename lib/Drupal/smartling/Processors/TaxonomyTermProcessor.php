<?php

/**
 * @file
 * Contains Drupal\smartling\Processors\NodeProcessor.
 */

namespace Drupal\smartling\Processors;

class TaxonomyTermProcessor extends GenericEntityProcessor {

  /**
   * {inheritdoc}
   *
   * @todo remove procedural code.
   */
  public function prepareOriginalEntity() {
    $this->contentEntity = taxonomy_term_load($this->entity->rid);
    $original_entity = $this->contentEntity;
    $term = i18n_taxonomy_term_get_translation($this->contentEntity, $this->drupalLocale);
    if (!is_null($term) && ($term->language != $this->contentEntity->language)) {
      $this->entity->rid = $term->tid;
    }
    else {
      // If term not exist, need create new term.
      $vocabulary = taxonomy_vocabulary_machine_name_load($this->entity->bundle);

      // Add language field or not depending on taxonomy mode.
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
      switch ($vocabulary_mode) {
        case I18N_MODE_TRANSLATE:
          $this->ifFieldsMethod = FALSE;
          // If the term to be added will be a translation of a source term,
          // set the default value of the option list
          // to the target language and
          // create a form element for storing
          // the translation set of the source term.
          $source_term = taxonomy_term_load($this->entity->rid);
          $term = clone $source_term;
          unset($term->tid);

          $target_language = i18n_language_object($this->drupalLocale);
          // Set context language to target language.
          i18n_language_context($target_language);

          $term->language = $target_language->language;

          // Add the translation set to the form so we know the new term
          // needs to be added to that set.
          if (!empty($source_term->i18n_tsid)) {
            $translation_set = i18n_taxonomy_translation_set_load($source_term->i18n_tsid);
          }
          else {
            // No translation set yet, build a new one with the source term.
            $translation_set = i18n_translation_set_create('taxonomy_term', $vocabulary->machine_name)
              ->add_item($source_term);
            taxonomy_term_save($source_term);
          }
          $term->i18n_tsid = $translation_set->tsid;
          break;

        case I18N_MODE_LOCALIZE:
          break;

        case I18N_MODE_LANGUAGE:
        case I18N_MODE_NONE:
          $this->log->setMessage('Translatable @entity_type with id - @rid FAIL. Vocabulary mode - @vocabulary_mode')
            ->setVariables(array(
              '@entity_type' => $this->originalEntityType,
              '@rid' => $this->entity->rid,
              '@vocabulary_mode' => $vocabulary_mode,
            ))
            ->execute();
          break;

        default:
          $this->log->setMessage('Translatable @entity_type with id - @rid FAIL')
            ->setVariables(array(
              '@entity_type' => $this->originalEntityType,
              '@rid' => $this->entity->rid,
            ))
            ->execute();
          break;
      }

      taxonomy_term_save($term);
      $this->entity->rid = $term->tid;
    }
  }

  public function updateTranslation() {
    // Do nothings.
  }
}
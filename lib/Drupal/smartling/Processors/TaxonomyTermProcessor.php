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
  public function prepareDrupalEntity() {
    // @todo move entity load logic to one place.
    $this->contentEntity = taxonomy_term_load($this->smartling_submission->rid);
    /* @var $source_drupal_entity \stdClass */
    $source_drupal_entity = $this->contentEntity;
    $term = i18n_taxonomy_term_get_translation($this->contentEntity, $this->drupalTargetLocale);
    if (!is_null($term) && ($term->language != $this->contentEntity->language)) {
      $this->smartling_submission->rid = $term->tid;
      $this->contentEntity = $term;
    }
    else {
      // If term not exist, need create new term.
      $vocabulary = taxonomy_vocabulary_machine_name_load($this->smartling_submission->bundle);

      // Add language field or not depending on taxonomy mode.
      $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
      switch ($vocabulary_mode) {
        case I18N_MODE_TRANSLATE:
          $this->ifFieldMethod = FALSE;
          // If the term to be added will be a translation of a source term,
          // set the default value of the option list
          // to the target language and
          // create a form element for storing
          // the translation set of the source term.
          $this->contentEntity = clone $source_drupal_entity;
          unset($this->contentEntity->tid);

          $target_language = i18n_language_object($this->drupalTargetLocale);
          // Set context language to target language.
          i18n_language_context($target_language);

          $this->contentEntity->language = $target_language->language;

          // Add the translation set to the form so we know the new term
          // needs to be added to that set.
          if (!empty($source_drupal_entity->i18n_tsid)) {
            $translation_set = i18n_taxonomy_translation_set_load($source_drupal_entity->i18n_tsid);
          }
          else {
            // No translation set yet, build a new one with the source term.
            $translation_set = i18n_translation_set_create('taxonomy_term', $vocabulary->machine_name)
              ->add_item($source_drupal_entity);
            taxonomy_term_save($source_drupal_entity);
          }
          $this->contentEntity->i18n_tsid = $translation_set->tsid;

          break;

        case I18N_MODE_LOCALIZE:
          break;

        case I18N_MODE_LANGUAGE:
        case I18N_MODE_NONE:
          $this->log->error('Translatable @entity_type with id - @rid FAIL. Vocabulary mode - @vocabulary_mode',
            array('@entity_type' => $this->drupalEntityType, '@rid' => $this->smartling_submission->rid, '@vocabulary_mode' => $vocabulary_mode));
          break;

        default:
          $this->log->error('Translatable @entity_type with id - @rid FAIL', array('@entity_type' => $this->drupalEntityType, '@rid' => $this->smartling_submission->rid));
          break;
      }

      taxonomy_term_save($this->contentEntity);
      $this->smartling_submission->rid = $this->contentEntity->tid;
    }
  }

  public static function supportedType($bundle) {
    $vocabulary = taxonomy_vocabulary_machine_name_load($bundle);
    $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
    return in_array($vocabulary_mode, array(I18N_MODE_TRANSLATE, I18N_MODE_LOCALIZE));
  }
}
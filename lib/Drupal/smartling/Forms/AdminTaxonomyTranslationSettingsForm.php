<?php

namespace Drupal\smartling\Forms;

class AdminTaxonomyTranslationSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_admin_taxonomy_translation_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $raw_types = taxonomy_get_vocabularies();
    $term_translate_fields = smartling_settings_get_handler()->taxonomyTermGetFieldsSettings();

    // What types of fields DO we translate?
    $translatable_field_types = smartling_get_translatable_field_types();

    $form['taxonomy_translation'] = array(
      'actions' => array(
        '#type' => 'actions',
      ),
    );

    $form['taxonomy_translation']['title'] = array(
      '#type' => 'item',
      '#title' => t('Which vocabularies do you want to translate?'),
    );

    $rows = array();
    global $_fix_field_settings;

    foreach ($raw_types as $vocabulary) {
      if (smartling_supported_type('taxonomy_term', $vocabulary->machine_name)) {
        $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
        switch ($vocabulary_mode) {
          case I18N_MODE_TRANSLATE:
            $voc_title = t('Translate method');
            break;

          case I18N_MODE_LOCALIZE:
            $voc_title = t('Localize method');
            break;

          case I18N_MODE_LANGUAGE:
          case I18N_MODE_NONE:
          default:
            $voc_title = '-';
            break;
        }

        $fr_fields = array();
        $fr_tt['from'] = array(
          '#type' => 'item',
          '#title' => t('@type', array('@type' => $voc_title)),
        );

        foreach (field_info_instances('taxonomy_term', $vocabulary->machine_name) as $field) {
          $field_label = $field['label'];
          $field_machine_name = $field['field_name'];
          $field_type = $field['widget']['type'];
          if (array_search($field_type, $translatable_field_types)) {
            $fr_fields[$field_machine_name] = array(
              '#type' => 'checkbox',
              '#title' => check_plain($field_label),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name),
                'name' => $vocabulary->machine_name . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name,
            );

            $is_in_conf = (!empty($term_translate_fields) && isset($term_translate_fields[$vocabulary->machine_name][$field_machine_name])) ? TRUE : FALSE;

            if ($is_in_conf) {
              $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
              // Error in field settings.
              if (($vocabulary_mode == I18N_MODE_TRANSLATE) && smartling_field_is_translatable_by_field_name($field_machine_name, 'taxonomy_term') && isset($term_translate_fields[$vocabulary->machine_name][$field['field_name']]) && !in_array($field_machine_name, $_fix_field_settings) && !smartling_is_title_module_field($field_machine_name)) {
                $_fix_field_settings[$field_machine_name] = '<b>' . $field_machine_name . '</b>';
              }
            }
          }
        }
        // Fix double field after change translate method.
        $term_field_names = array(
          'description' => 'description',
          'name' => 'name',
        );
        foreach ($term_field_names as $property_name) {
          if (isset($fr_fields[$property_name . '_field'])) {
            unset($term_field_names[$property_name]);
          }
        }

        if ($vocabulary_mode == I18N_MODE_LOCALIZE) {
          $term_field_names = array('description', 'name');
          foreach ($term_field_names as $term_field_name) {
            if (!isset($fr_fields[$term_field_name . '_field'])) {
              $fr_fields[$term_field_name . '_field'] = array(
                '#type' => 'checkbox',
                '#title' => t('@name (Note: field will be created.)', array('@name' => ucfirst($term_field_name))),
                '#attributes' => array(
                  'id' => array('edit-form-item-' . $vocabulary->machine_name . '-separator-' . $term_field_name),
                  'name' => $term_field_name . '_swap_' . $vocabulary->machine_name,
                  'class' => array('field'),
                ),
              );

              $is_in_conf = (!empty($term_translate_fields) && isset($term_translate_fields[$vocabulary->machine_name][$term_field_name . '_field'])) ? TRUE : FALSE;
              if ($is_in_conf) {
                $fr_fields[$term_field_name . '_field']['#attributes']['checked'] = 'checked';
              }
            }
          }
        }
        // Fake field for taxonomy name and description ($term->name).
        if ($vocabulary_mode == I18N_MODE_TRANSLATE) {
          foreach ($term_field_names as $term_field) {
            $field_machine_name = $term_field . '_property_field';
            $fr_fields[$field_machine_name] = array(
              '#type' => 'checkbox',
              '#title' => t('@name', array('@name' => ucfirst($term_field))),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name),
                'name' => $vocabulary->machine_name . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $vocabulary->machine_name . '-separator-' . $field_machine_name,
            );

            $is_in_conf = (!empty($term_translate_fields) && isset($term_translate_fields[$vocabulary->machine_name][$field_machine_name])) ? TRUE : FALSE;

            if ($is_in_conf) {
              $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
            }
          }
        }
      }
      else {
        $options = array(
          0 => t('- Select Method -'),
          2 => t('Translate method'),
          1 => t('Localize method'),
        );

        $fr_tt['method'][$vocabulary->machine_name] = array(
          '#type' => 'select',
          '#title' => t('Translation Type'),
          '#title_display' => 'invisible',
          '#options' => $options,
          '#required' => FALSE,
          '#default_value' => 0,
          '#attributes' => array(
            'id' => array('edit-form-item-' . $vocabulary->machine_name . '-TT-' . $vocabulary->machine_name),
            'name' => $vocabulary->machine_name . '_TT_' . $vocabulary->machine_name,
            'class' => array('content-type'),
          ),
        );

        $fr_fields = array();
      }
      $rows[$vocabulary->machine_name] = array(
        array(
          'data' => $vocabulary->name,
          'width' => '20%',
        ),
        array(
          'data' => drupal_render($fr_tt),
          'width' => '20%',
        ),
        array(
          'data' => drupal_render($fr_fields),
          'width' => '60%',
        ),
      );
      unset($fr_tt);
      unset($fr_fields);
    }

    $header = array(t('Vocabularies'), t('Translation Type'), t('Fields'));

    $variables = array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('class' => array('smartling-content-settings-table')),
    );

    $form['taxonomy_translation']['types'] = array(
      '#type' => 'markup',
      '#markup' => theme('table', $variables),
    );

    foreach (array_keys($term_translate_fields) as $content_type) {
      $form['taxonomy_translation']['types']['#default_value'][$content_type] = 1;
    }

    $form['taxonomy_translation']['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );

    $form['#submit'][] = 'smartling_admin_taxonomy_translation_settings_form_submit';

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, array &$form_state) {

  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
    $log = smartling_log_get_handler();
    // This is needed for the setup because of the field/node method selector.
    system_settings_form_submit($form, $form_state);

    $translate = array();
    $operations = array();

    foreach ($form_state['input'] as $key => $value) {
      // Look for Selected Content Types and Fields.
      if (FALSE !== strpos($key, '_SEPARATOR_')) {
        // And only if set to translate.
        if ($value != 0) {
          $parts = explode('_SEPARATOR_', $key);
          $machine_name = $parts[0];
          $content_field = $parts[1];

          $translate[$machine_name][$content_field] = $content_field;

          // Set this field to 'translatable'.
          // Update the field via the Field API (Instead of the direct db_update).
          $vocabulary = taxonomy_vocabulary_machine_name_load($machine_name);
          $vocabulary_mode = i18n_taxonomy_vocabulary_mode($vocabulary);
          if ($vocabulary_mode == I18N_MODE_LOCALIZE) {
            $field = field_info_field($content_field);
            $field['translatable'] = 1;
            field_update_field($field);
          }
        }
      }
      // END:  Selected Content Types and Fields.
      // Look for Selected Translation Type.
      if (FALSE !== strpos($key, '_TT_')) {
        // And only if set to translate.
        if ($value != 0) {
          $parts = explode('_TT_', $key);
          $machine_name = $parts[0];
          $vocabulary = taxonomy_vocabulary_machine_name_load($machine_name);
          if ($value == 2) {
            $vocabulary->i18n_mode = I18N_MODE_TRANSLATE;
          }
          elseif ($value == 1) {
            $vocabulary->i18n_mode = I18N_MODE_LOCALIZE;
          }
          taxonomy_vocabulary_save($vocabulary);
        }
      }

      // Look for any terms we need to do the swap for.
      $term_field_names = array('description', 'name');
      foreach ($term_field_names as $term_field_name) {
        if (FALSE !== strpos($key, $term_field_name . '_swap_')) {
          // And only if set to swap.
          if ($value != 0) {
            $machine_name = substr($key, strlen($term_field_name . '_swap_'));

            // Do the actual replacement.
            $entity_type = 'taxonomy_term';
            $legacy_field = $term_field_name;

            // Use the Title module to migrate the content.
            if (title_field_replacement_toggle($entity_type, $machine_name, $legacy_field)) {
              $operations[] = array(
                'title_field_replacement_batch',
                array(
                  $entity_type,
                  $machine_name,
                  $legacy_field,
                ),
              );
              // Add in config.
              $translate[$machine_name][$term_field_name . '_field'] = $term_field_name . '_field';
              $field = field_info_field($term_field_name . '_field');
              $field['translatable'] = 1;
              $operations[] = array('field_update_field', array($field));
            }
          }
        }
      }
    }

    smartling_settings_get_handler()->taxonomyTermSetFieldsSettings($translate);
    drupal_set_message(t('Your taxonomy vocabularies have been updated.'));

    $log->setMessage('Smartling taxonomy vocabularies and fields have been updated.')
      ->setConsiderLog(FALSE)
      ->execute();

    $redirect = url('admin/config/regional/smartling', array(
      'absolute' => TRUE,
      'fragment' => 'smartling-taxonomy-vocabularies-and-fields',
    ));

    if (count($operations) >= 1) {
      $batch = array(
        'title' => t('Preparing content'),
        'operations' => $operations,
      );

      batch_set($batch);
      batch_process($redirect);
    }
    else {
      $form_state['redirect'] = $redirect;
    }
  }
}
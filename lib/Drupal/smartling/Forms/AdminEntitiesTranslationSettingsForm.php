<?php

namespace Drupal\smartling\Forms;

class AdminEntitiesTranslationSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_admin_entities_translation_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $entity_rows = $this->entities_translation_crawler();
    //$user_ = drupal_get_form('smartling_admin_user_translation_settings_form');

    $header = array(t('Entity Type'), t('Fields'));
    //$rows = array(array(t('User'), drupal_render($user_)));

    $variables = array(
      'header' => $header,
      'rows' => $entity_rows,
      'attributes' => array('class' => array('smartling-content-settings-table')),
    );

    $form['entity_translation']['types'] = array(
      '#type' => 'markup',
      '#markup' => theme('table', $variables),
    );

    $form['entity_translation']['actions'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );

    $form['#submit'][] = 'smartling_admin_entities_translation_settings_form_submit';


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
    $input = $form_state['input'];
    $entities = entity_get_info();
    $translate = array();

    // Obtain what fields instances are marked for translation.
    foreach ($input as $key => $value) {
      // Look for Selected Content Types and Fields.
      if (FALSE !== strpos($key, '_SEPARATOR_')) {
        // And only if set to translate.
        if ($value != 0) {
          $parts = explode('_SEPARATOR_', $key);
          $content_type = $parts[0];
          $content_field = $parts[1];

          $translate[$content_type][$content_field] = $content_field;

          $field = field_info_field($content_field);
          $field['translatable'] = 1;
          field_update_field($field);
        }
      }

      if (FALSE !== strpos($key, '_swap_')) {
        // And only if set to swap.
        if ($value != 0) {
          // @todo
          // This is too ambiguous and hard-coded.
          $swap_chunks = explode('_swap_', $key);
          $legacy_field = $swap_chunks[0];
          $bundle = $swap_chunks[1];

          switch ($legacy_field) {
            case 'title':
              $entity_type = 'fieldable_panels_pane';
              $resulting_field = 'title_field';
              break;
            case 'subject':
              $entity_type = 'comment';
              $resulting_field = 'subject_field';
              break;
          }

          // Use the Title module to migrate the content.
          if (title_field_replacement_toggle($entity_type, $bundle, $legacy_field)) {
            $operations[] = array(
              'title_field_replacement_batch',
              array(
                $entity_type,
                $bundle,
                $legacy_field,
              ),
            );
            // Add in config.
            $translate[$bundle][$resulting_field] = $resulting_field;

            $field = field_info_field($resulting_field);
            $field['translatable'] = 1;
            $operations[] = array('field_update_field', array($field));
          }
        }
      }
    }

    // Define parent bundle for those fields.
    $_translate = array();
    $machine_names = array_keys($translate);
    foreach ($entities as $entity => $definition) {
      $bundles = array_keys($definition['bundles']);

      foreach ($machine_names as $name) {
        if (in_array($name, $bundles)) {
          $_translate[$entity][$name] = $translate[$name];
        }
      }
    }

    // Reset default state, since all disabled checkboxes will be ignored
    // and therefore, not updated.
    //smartling_settings_get_handler()->resetAllFieldsSettings();

    // Save the settings, considering entity bundle, to know which
    // update method to be called.
    foreach ($_translate as $k => $v) {
      if (in_array($k, array('user', 'comment', 'field_collection_item', 'fieldable_panels_pane'))) {
        smartling_settings_get_handler()->setFieldsSettings($k, $v);
      }
    }

    drupal_set_message(t('Entities settings updated.'));

    $log = smartling_log_get_handler();
    $log->setMessage('Smartling entities and fields have been updated.')
      ->setConsiderLog(FALSE)
      ->execute();

    $redirect = url('admin/config/regional/smartling', array(
      'absolute' => TRUE,
      'fragment' => 'smartling-entities-settings',
    ));

    if (!empty($operations)) {
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


  /**
   * Aggregate various types of entities and their fields.
   *
   * @return array
   *   Set of rows ready to be placed into table form element.
   *   First element is the entity name, second - checkboxes for each
   *   translatable fields for this entity.
   */
  protected function entities_translation_crawler() {
    // Entities we do not want to parse.
    $exclude = array(
      'node',
      'taxonomy_term',
      'taxonomy_vocabulary',
      'i18n_translation',
      'smartling_interface_entity',
      'file',
      'smartling_entity_data',
    );

    $translatable_field_types = smartling_get_translatable_field_types();
    $entities = entity_get_info();
    $rows = array();

    // Loop through each entity type.
    foreach ($entities as $name => $definition) {
      if (in_array($name, $exclude)) {
        continue;
      }

      $bundles = array_keys($definition['bundles']);
      // Loop through each entity bundle.
      foreach ($bundles as $bundle) {
        $field_instances = field_info_instances($name, $bundle);

        // Loop through each field instance of the bundle.
        foreach ($field_instances as $field) {
          $field_label = $field['label'];
          $field_machine_name = $field['field_name'];
          $field_type = $field['widget']['type'];

          if (in_array($field_type, $translatable_field_types)) {
            $key = $bundle . '_SEPARATOR_' . $field_machine_name;
            $fr_fields[$key] = array(
              '#type' => 'checkbox',
              '#title' => check_plain($field_label),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $bundle . '-separator-' . $field_machine_name),
                'name' => $bundle . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $bundle . '-separator-' . $field_machine_name,
            );

            // Comments subject field need special treatment. i.e. some fields
            // replaced instead of standard ones.
            $comment_key = $bundle . '_SEPARATOR_' . 'subject_field';
            $fpp_key = $bundle . '_SEPARATOR_' . 'title_field';
            if ($name == 'comment' && !isset($fr_fields[$comment_key])) {
              $fr_fields[$comment_key] = array(
                '#type' => 'checkbox',
                '#title' => t('Subject (Note: field will be created.)'),
                '#attributes' => array(
                  'id' => array('edit-form-item-' . $bundle . '-separator-subject'),
                  'name' => 'subject_swap_' . $bundle,
                  'class' => array('field'),
                ),
              );
            }
            else if ($name == 'fieldable_panels_pane' && !isset($fr_fields[$fpp_key])) {
              $fr_fields[$fpp_key] = array(
                '#type' => 'checkbox',
                '#title' => t('Title (Note: field will be created.)'),
                '#attributes' => array(
                  'id' => array('edit-form-item-' . $bundle . '-separator-title'),
                  'name' => 'title_swap_' . $bundle,
                  'class' => array('field'),
                ),
              );
            }

            if (in_array($name, array('user', 'comment', 'field_collection_item', 'fieldable_panels_pane'))) {
              $translate_fields = smartling_settings_get_handler()->getFieldsSettings($name);

              $is_in_conf = !empty($translate_fields) && isset($translate_fields[$bundle][$field_machine_name]);

              if ($is_in_conf) {
                $fr_fields[$key]['#attributes']['checked'] = 'checked';
              }
            }
          }
        }

        $rows[] = array($bundle, drupal_render($fr_fields));
        $fr_fields = NULL;
      }
    }

    return $rows;
  }
}
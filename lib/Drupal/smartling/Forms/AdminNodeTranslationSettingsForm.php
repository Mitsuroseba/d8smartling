<?php

namespace Drupal\smartling\Forms;

class AdminNodeTranslationSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_admin_node_translation_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $raw_types = node_type_get_types();
    $node_translate_fields = smartling_settings_get_handler()->nodeGetFieldsSettings();

    // What types of fields DO we translate?
    $translatable_field_types = smartling_get_translatable_field_types();

    $form['node_translation'] = array(
      'actions' => array(
        '#type' => 'actions',
      ),
    );

    $form['node_translation']['title'] = array(
      '#type' => 'item',
      '#title' => t('Which content types do you want to translate?'),
    );

    $rows = array();
    global $_fix_field_settings;

    foreach ($raw_types as $value) {
      if (smartling_supported_type('node', $value->type)) {
        $fr_tt['from'] = array(
          '#type' => 'item',
          '#title' => t('Fields method'),
        );
        if (smartling_nodes_method($value->type)) {
          $fr_tt['from']['#title'] = t('Nodes method');
        }

        $fr_fields = array();

        foreach (field_info_instances('node', $value->type) as $field) {
          $field_label = $field['label'];
          $field_machine_name = $field['field_name'];
          $field_type = $field['widget']['type'];
          if (array_search($field_type, $translatable_field_types)) {
            $fr_fields[$field_machine_name] = array(
              '#type' => 'checkbox',
              '#title' => check_plain($field_label),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $value->type . '-separator-' . $field_machine_name),
                'name' => $value->type . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $value->type . '-separator-' . $field_machine_name,
            );

            $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;

            if ($is_in_conf) {
              $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
              // Error in field settings.
              if (smartling_nodes_method($value->type) && smartling_field_is_translatable_by_field_name($field_machine_name, 'node') && !in_array($field_machine_name, $_fix_field_settings) && !smartling_is_title_module_field($field_machine_name)) {
                $_fix_field_settings[$field_machine_name] = '<b>' . $field_machine_name . '</b>';
              }
            }
          }
        }

        if (!isset($fr_fields['title_field']) && smartling_fields_method($value->type)) {
          $fr_fields['title_field'] = array(
            '#type' => 'checkbox',
            '#title' => t('Title (Note: field will be created.)'),
            '#attributes' => array(
              'id' => array('edit-form-item-' . $value->type . '-separator-title_field'),
              'name' => 'title_swap_' . $value->type,
              'class' => array('field'),
            ),
          );

          $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type]['title_field'])) ? TRUE : FALSE;
          if ($is_in_conf) {
            $fr_fields['title_field']['#attributes']['checked'] = 'checked';
          }
        }
        // Fix double title after change translate method.
        if (!isset($fr_fields['title_field'])) {
          // Fake field title ($node->title) for nodes method.
          if (smartling_nodes_method($value->type)) {
            $field_machine_name = 'title_property_field';
            $fr_fields[$field_machine_name] = array(
              '#type' => 'checkbox',
              '#title' => t('Title'),
              '#attributes' => array(
                'id' => array('edit-form-item-' . $value->type . '-separator-title_property_field' . $field_machine_name),
                'name' => $value->type . '_SEPARATOR_' . $field_machine_name,
                'class' => array('field'),
              ),
              '#id' => 'edit-form-item-' . $value->type . '-separator-' . $field_machine_name,
            );

            $is_in_conf = (!empty($node_translate_fields) && isset($node_translate_fields[$value->type][$field_machine_name])) ? TRUE : FALSE;
            if ($is_in_conf) {
              $fr_fields[$field_machine_name]['#attributes']['checked'] = 'checked';
            }
          }
        }
      }
      else {
        $options = array(
          0 => t('- Select Method -'),
          2 => t('Nodes method'),
          1 => t('Fields method'),
        );

        $fr_tt['method'][$value->type] = array(
          '#type' => 'select',
          '#title' => t('Translation Type'),
          '#title_display' => 'invisible',
          '#options' => $options,
          '#required' => FALSE,
          '#default_value' => 0,
          '#attributes' => array(
            'id' => array('edit-form-item-' . $value->type . '-TT-' . $value->type),
            'name' => $value->type . '_TT_' . $value->type,
            'class' => array('content-type'),
          ),
        );

        $fr_fields = array();
      }
      $rows[$value->type] = array(
        array(
          'data' => check_plain($value->name),
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

    $header = array(t('Entity Type'), t('Translation Type'), t('Fields'));

    $variables = array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('class' => array('smartling-content-settings-table')),
    );

    $form['node_translation']['types'] = array(
      '#type' => 'markup',
      '#markup' => theme('table', $variables),
    );

    foreach (array_keys($node_translate_fields) as $content_type) {
      $form['node_translation']['types']['#default_value'][$content_type] = 1;
    }

    $form['node_translation']['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );

    $form['#submit'][] = 'smartling_admin_node_translation_settings_form_submit';

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
    $smartling_settings = smartling_settings_get_handler();
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
          $content_type = $parts[0];
          $content_field = $parts[1];

          $translate[$content_type][$content_field] = $content_field;

          // Set this field to 'translatable'.
          // Update the field via the Field API (Instead of the direct db_update).
          if (smartling_fields_method($content_type)) {
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
          $content_type = $parts[0];
          if ($value == 2) {
            variable_set('language_content_type_' . $content_type, SMARTLING_NODES_METHOD_KEY);
          }
          elseif ($value == 1) {
            variable_set('language_content_type_' . $content_type, SMARTLING_FIELDS_METHOD_KEY);
          }
        }
      }

      // Look for any nodes we need to do the Title swap for.
      if (FALSE !== strpos($key, 'title_swap_')) {
        // And only if set to swap.
        if ($value != 0) {
          $content_type = substr($key, strlen('title_swap_'));

          // Do the actual title replacement.
          $entity_type = 'node';
          $bundle = $content_type;
          $legacy_field = 'title';

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
            $translate[$content_type]['title_field'] = 'title_field';
            $field = field_info_field('title_field');
            $field['translatable'] = 1;
            $operations[] = array('field_update_field', array($field));
          }
        }
      }
    }

    $smartling_settings->nodeSetFieldsSettings($translate);
    drupal_set_message(t('Your content types have been updated.'));
    $log->setMessage('Smartling content types and fields have been updated.')
      ->setConsiderLog(FALSE)
      ->execute();

    $redirect = url('admin/config/regional/smartling', array(
      'absolute' => TRUE,
      'fragment' => 'smartling-nodes-settings',
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
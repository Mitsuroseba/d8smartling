<?php

namespace Drupal\smartling\Forms;

class NodeSettingsForm implements FormInterface {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_get_node_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    if (smartling_translate_fields_is_set($form['#node']->type, 'node')) {
      // Vertical Tab.
      $form['smartling'] = array(
        '#title' => t('Smartling management'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#group' => 'additional_settings',
        '#attributes' => array('id' => array('smartling_fieldset')),
        '#attached' => array(
          'css' => array(
            drupal_get_path('module', 'smartling') . '/css/smartling_entity_settings.css' => array(
              'type' => 'file',
            ),
          ),
        ),
        '#modal' => TRUE,
      );

      if (language_default('language') != field_valid_language(NULL, FALSE)) {
        //Otherwise if "title" module is enabled - it will spoil the title of the original node.
        $form['smartling']['error_default_language'] = array(
          '#type' => 'markup',
          '#markup' => '<p>Please switch to site\'s default language in order to execute the operations. The default language is: ' . language_default('language') . '</p>',
        );
        return $form;
      }

      $form['smartling']['content'] = array(
        '#type' => 'container',
      );

      $languages = array();
      // This is node for fields method translate or original for nodes method.
      if (($form['#node']->tnid == '0') || ($form['#node']->tnid == $form['#node']->nid)) {
        $languages = smartling_language_list();
      }
      elseif ($form['#node']->tnid != $form['#node']->nid) {
        // This is node for nodes method translate | not original.
        $languages = smartling_language_list();
        $node_original = node_load($form['#node']->tnid);
        unset($languages[$node_original->language]);
      }

      $options = array();

      if (!is_null($form['nid']['#value'])) {
        $check = array();

        if (($form['#node']->tnid != '0') && ($form['#node']->tnid != $form['#node']->nid)) {
          // For not original node in nodes translate method.
          $translations = translation_node_get_translations($form['#node']->tnid);
          $original_nid = FALSE;
          // Get original.
          foreach ($translations as $langcode => $value) {
            if ($translations[$langcode]->nid == $form['#node']->tnid) {
              $original_nid = $translations[$langcode]->nid;
              break;
            }
          }

          foreach ($languages as $d_locale => $language) {
            if ($language->enabled != '0') {

              $entity_data = smartling_entity_load_by_conditions(array(
                'rid' => $original_nid,
                'entity_type' => $form['#entity_type'],
                'target_language' => $d_locale,
              ));
              $language_name = check_plain($language->name);

              if ($entity_data !== FALSE) {
                $options[$d_locale] = smartling_entity_status_message(t('Node'), $entity_data->status, $language_name, $entity_data->progress);
              }
              else {
                $options[$d_locale] = $language_name;
              }

              $check[] = ($entity_data) ? $d_locale : FALSE;
            }
          }
        }
        elseif (($form['#node']->tnid != '0') && ($form['#node']->tnid == $form['#node']->nid)) {
          // For original node in nodes translate method.
          $translations = translation_node_get_translations($form['#node']->tnid);
          $original_nid = FALSE;
          // Get original.
          foreach ($translations as $langcode => $value) {
            if ($translations[$langcode]->nid == $form['#node']->tnid) {
              $original_nid = $translations[$langcode]->nid;
              break;
            }
          }

          foreach ($languages as $d_locale => $language) {

            if ($form['language']['#default_value'] != $d_locale && $language->enabled != '0') {

              $entity_data = smartling_entity_load_by_conditions(array(
                'rid' => $original_nid,
                'entity_type' => $form['#entity_type'],
                'target_language' => $d_locale,
              ));
              $language_name = check_plain($language->name);

              if ($entity_data !== FALSE) {
                $options[$d_locale] = smartling_entity_status_message(t('Node'), $entity_data->status, $language_name, $entity_data->progress);
              }
              else {
                $options[$d_locale] = $language_name;
              }

              $check[] = ($entity_data) ? $d_locale : FALSE;
            }
          }
        }
        else {
          // For fieds method.
          foreach ($languages as $d_locale => $language) {
            if ($form['language']['#default_value'] != $d_locale && $language->enabled != '0') {

              $entity_data = smartling_entity_load_by_conditions(array(
                'rid' => $form['nid']['#value'],
                'entity_type' => $form['#entity_type'],
                'target_language' => $d_locale,
              ));
              $language_name = check_plain($language->name);

              if ($entity_data !== FALSE) {
                $options[$d_locale] = smartling_entity_status_message(t('Node'), $entity_data->status, $language_name, $entity_data->progress);
              }
              else {
                $options[$d_locale] = $language_name;
              }
              $check[] = ($entity_data) ? $d_locale : FALSE;
            }
          }
        }

        $form['smartling']['content']['target'] = array(
          '#type' => 'checkboxes',
          '#title' => 'Target Locales',
          '#options' => $options,
          '#default_value' => $check,
        );
      }
      else {
        foreach ($languages as $langcode => $language) {
          $options[$langcode] = check_plain($language->name);
        }

        $form['smartling']['content']['target'] = array(
          '#type' => 'checkboxes',
          '#title' => 'Target Locales',
          '#options' => $options,
        );
      }

      $form['smartling']['submit_to_translate'] = array(
        '#type' => 'submit',
        '#value' => 'Send to Smartling',
        '#submit' => array('smartling_get_node_settings_form_submit'),
        '#states' => array(
          'invisible' => array(
            // Hide the button if term is language neutral.
            'select[name=language]' => array('value' => LANGUAGE_NONE),
          ),
        ),
      );

      $form['smartling']['submit_to_download'] = array(
        '#type' => 'submit',
        '#value' => 'Download Translation',
        '#submit' => array('smartling_download_translate_form_submit'),
        '#states' => array(
          'invisible' => array(
            // Hide the button if term is language neutral.
            'select[name=language]' => array('value' => LANGUAGE_NONE),
          ),
        ),
      );

      return $form;
    }
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
    if (!smartling_is_configured()) {
      return;
    }

    $log = smartling_log_get_handler();
    $entity = $this->getOriginalEntity($form['#entity']);
    $entity_type = $form['#entity_type'];

    if (empty($entity)) {
      return;
    }

    $wrapper = entity_metadata_wrapper($entity_type, $entity);
    $id = $wrapper->getIdentifier();
    $title = $wrapper->label();
    //$comment = comment_form_submit_build_comment($form, $form_state);
    $d_locale_original = $entity->translations->original;
    $langs = array();
    $link = smartling_get_link_to_entity($entity_type, $entity);

//    $log = smartling_log_get_handler();
//    $node = node_form_submit_build_node($form, $form_state);

    $smartling_queue = \DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();

    /* @var $entity_wrapper \EntityDrupalWrapper */
//    $entity_type = $form['#entity_type'];
//    $entity_wrapper = entity_metadata_wrapper($entity_type, $node);

    foreach ($form_state['values']['target'] as $target_language) {
      if ($target_language) {
        $conditions = array(
          'rid' => $id,
          'entity_type' => $entity_type, //$entity_wrapper->getBundle(),
          'target_language' => $target_language,
        );
        $smartling_entity = smartling_entity_load_by_conditions($conditions);

        if (!$smartling_entity) {
          $smartling_entity = smartling_create_from_entity($entity, $entity_type, $entity_wrapper->language->value(), $target_language);
        }

        $processor = smartling_get_entity_processor($smartling_entity);
        $processor->sendToUploadQueue();

        $smartling_queue->createItem($smartling_entity->eid);

        $log->setMessage('Add smartling queue task for node id - @nid, locale - @locale')
          ->setVariables(array(
            '@nid' => $id,
            '@locale' => $target_language,
          ))
          ->setLink($link)
          ->execute();

        drupal_set_message(t('The node "@title" has been scheduled to be sent to Smartling for translation to "@lang".', array(
          '@title' => $title,
          '@lang' => $target_language,
        )));
      }
    }

    unset($_GET['destination']);
    // For not change node status to red when send node and change content.
    $entity->send_to_smartling = TRUE;
    node_save($entity);
  }
}
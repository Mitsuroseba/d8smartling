<?php

/**
 * @file
 * Functions for taxonomy_term entity.
 */

/**
 * Implements hook_form_alter().
 */
function smartling_form_alter(&$form, &$form_state, $form_id) {
  $active_node_type = node_type_get_types();
  foreach ($active_node_type as $node_type => $value) {

    if (smartling_fields_method($node_type) && ($form_id == 'comment_node_' . $node_type . '_form')) {
      $comment = $form['#entity'];

      if (smartling_supported_type($form['#entity_type'], $form['#bundle'])) {

        if (!empty($comment->cid)) {
          if (($comment->language != LANGUAGE_NONE)) {
            $form = array_merge($form, smartling_get_comment_settings_form($form, $form_state));
            break;
          }
          else {
            $form['smartling'] = array(
              '#title' => t('Smartling management'),
              '#type' => 'fieldset',
              '#collapsible' => TRUE,
              '#collapsed' => TRUE,
              '#weight' => 100,
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

            $form['smartling']['content'] = array(
              '#type' => 'container',
            );

            $form['smartling']['content']['lang'] = array(
              '#type' => 'item',
              '#title' => '',
              '#markup' => t('This entity is as to be Language Neutral.'),
            );
          }
        }
      }
      else {
        drupal_set_message(t('Add translatable field to comment or <a href="/admin/config/regional/entity_translation/translatable/comment_body?destination=@current_path">Enable translation</a> for comment_body field to use Smartling translate.', array('@current_path' => current_path())), 'info');
      }
    }
  }
}

function smartling_get_comment_settings_form($form, &$form_state) {
  if (!isset($form_state['confirm_delete']) || $form_state['confirm_delete'] !== TRUE) {
    $form['smartling'] = array(
      '#title' => t('Smartling management'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 100,
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

    $form['smartling']['content'] = array(
      '#type' => 'container',
    );

    $languages = smartling_language_list();
    $options = array();
    $check = array();
    $comment = $form['#entity'];

    if (!is_null($comment->cid)) {
      foreach ($languages as $d_locale => $language) {
        if ($language->enabled != '0') {

          $entity_data = smartling_entity_load_by_conditions(array('rid' => $comment->cid, 'entity_type' => $form['#entity_type'], 'target_language' => $d_locale));

          if ($entity_data !== FALSE) {
            switch ($entity_data->status) {
              case SMARTLING_STATUS_IN_QUEUE:
                $options[$d_locale] = '<span title="' . t('Term is waiting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-queue">' . $entity_data->progress . '%</span></span>';
                break;
              case SMARTLING_STATUS_IN_TRANSLATE:
                $options[$d_locale] = '<span title="' . t('Translation is in progress') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status in-translate">' . $entity_data->progress . '%</span></span>';
                break;
              case SMARTLING_STATUS_TRANSLATED:
                $options[$d_locale] = '<span title="' . t('Term is translated') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status translated">' . $entity_data->progress . '%</span></span>';
                break;
              case SMARTLING_STATUS_CHANGE:
                $options[$d_locale] = '<span title="' . t('Term was changed after submitting for translation') . '"><span class="lang-name">' . $language->name . '</span><span class="lang-status status-changed">' . $entity_data->progress . '%</span></span>';
                break;
            }
          }
          else {
            $options[$d_locale] = $language->name;
          }

          $check[] = ($entity_data) ? $d_locale : FALSE;
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
      foreach ($languages as $d_locale => $language) {
        $options[$d_locale] = $language->name;
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
      '#submit' => array('smartling_get_comment_settings_form_submit'),
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
  }
  else {
    $form = array();
  }

  return $form;
}

/**
 * smartling_get_comment_settings_form_submit
 */
function smartling_get_comment_settings_form_submit($form, &$form_state) {
  $comment = comment_form_submit_build_comment($form, $form_state);
  $d_locale_original = $comment->translations->original;
  $langs = array();
  $link = smartling_get_link_to_entity($comment->cid, $form['#entity_type']);

  if (count(array_filter($form_state['values']['target'])) !== 0) {
    global $user;

    $smartling_queue = DrupalQueue::get('smartling_upload');
    $smartling_queue->createQueue();

    foreach ($form_state['values']['target'] as $d_locale) {
      if ($d_locale !== 0 && ($d_locale_original !== $d_locale)) {
        $entity_data = smartling_entity_load_by_conditions(array('rid' => $comment->cid, 'entity_type' => $form['#entity_type'], 'target_language' => $d_locale));

        if ($entity_data == FALSE) {
          $entity_data = smartling_entity_data_create();
          $entity_data->rid = $comment->cid;
          $entity_data->entity_type = $form['#entity_type'];
          $entity_data->bundle = $form['#bundle'];
          $entity_data->original_language = $d_locale_original;
          $entity_data->target_language = $d_locale;
          $entity_data->progress = 0;
        }

        $entity_data->translated_file_name = FALSE;
        $entity_data->submitter = $user->uid;
        $entity_data->submission_date = time();
        smartling_entity_data_save($entity_data);

        smartling_set_translation_status($entity_data, SMARTLING_STATUS_EVENT_SEND_TO_UPLOAD_QUEUE);
        $langs[] = $d_locale;
      }
    }
    $obj = new stdClass;
    $obj->rid = $comment->cid;
    $obj->entity_type = $form['#entity_type'];
    $obj->bundle = $form['#bundle'];
    $obj->title = $comment->subject;
    $obj->original_language = $d_locale_original;
    $obj->d_locales = $langs;
    $obj->submitter = $user->uid;
    // Create queue item
    $smartling_queue->createItem($obj);
    unset($obj);
    smartling_watchdog('Add smartling queue task for comment cid - @cid, locale - @locale', TRUE, array('@cid' => $comment->cid, '@locale' => implode('; ', $langs)), WATCHDOG_INFO, $link);

    $langs = implode(', ', $langs);
    drupal_set_message(t('The comment "@title" has been sent to Smartling for translation to "@langs".', array('@title' => $comment->subject, '@langs' => $langs)));
  }

  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }

  comment_save($comment);
  smartling_watchdog('Updated comment %comment.', TRUE, array('%comment' => $comment->subject), WATCHDOG_INFO, $link);
}

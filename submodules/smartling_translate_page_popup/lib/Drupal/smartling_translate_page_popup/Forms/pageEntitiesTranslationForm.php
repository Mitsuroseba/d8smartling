<?php

namespace Drupal\smartling_translate_page_popup\Forms;

class pageEntitiesTranslationForm implements \Drupal\smartling\Forms\FormInterface {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'smartling_page_entities_translation_form';
  }

  public function buildForm(array $form, array &$form_state) {
    $options = array();

    $path = drupal_get_path('module', 'smartling_translate_page_popup');
    $start =<<<EOF
    <div id="smartling_translate_popup" class="smartling_state_compressed">
      <div id="smartling_popup_header">
        <div>
          <a id="smartling_popup_closebutton" class="helper_button">x</a>
          <a id="smartling_popup_sizebutton" class="helper_button">_</a>
          <a href="http://www.smartling.com/" target="blank"><img src="/$path/static/logo.png" height="30px"></a>
        </div>
      </div>
	  <div id="smartling_popup_content" class="not_visible">
	    <span id="smartling_stats"><a>Stats</a></span>
	    <div id="translation_result"></div>
EOF;
    $end =<<<EOF
      </div>
    </div>
EOF;


    if (empty($_SESSION['smartling']['page_entities'] )) {
      $form['smartling']['start'] = array(
        '#prefix' => $start,
        '#markup' => t('It seems that there are no items on the page that Smartling can translate. This can happen if <a href="@link">Smartling module is not configured</a> properly or there are no content items in the <b>site\'s default</b> or <b>neutral</b> language.', array('@link' => url('admin/config/regional/smartling'))),
        '#suffix' => $end,
        '#attached' => array('css' => array(drupal_get_path('module', 'smartling_translate_page_popup') . '/static/smartling_translate_page_popup.css'),
          'js'  => array(drupal_get_path('module', 'smartling_translate_page_popup') . '/static/smartling_translate_page_popup.js',))
      );
      return $form;
    }

    foreach ($_SESSION['smartling']['page_entities'] as $id => $title) {
      list($entity_id, $entity_type) = explode('_||_', $id);
      $res = smartling_entity_load_all_by_conditions(array(
        'rid' => $entity_id,
        'entity_type' => $entity_type,
      ));

      $data = '';
      if (!empty($res)) {
        foreach($res as $dt) {
          $data .= $dt->target_language . ' - ' . $dt->progress . '%; ';
        }
      }

      $options [$id] = $title . " <span class='entity_type'>($entity_type)</span> <span class='entity_progress'>($data)</span>";
    }
    unset($_SESSION['smartling']['page_entities']);

    $form['smartling']['start'] = array(
      '#markup' =>$start
    );

    $form['smartling']['items'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,//$_SESSION['smartling']['page_entities'], //drupal_map_assoc(array(t('SAT'), t('ACT'))),
      '#title' => t('Content items'),
      //'#description' => t('What items would you like to translate?'),
      '#required' => TRUE,
    );

    $form['smartling']['languages'] = array(
      '#type' => 'checkboxes',
      '#options' => smartling_language_options_list(),//$_SESSION['smartling']['page_entities'], //drupal_map_assoc(array(t('SAT'), t('ACT'))),
      '#title' => t('Languages'),
      '#required' => TRUE,
    );

    $form['smartling']['submit'] = array(
      '#type' => 'submit',
      '#ajax' => array(
        'callback' => 'smartling_translate_page_popup_form_submit',
      ),
      '#value' => t('Translate'),
    );

    $form['smartling']['end'] = array(
      '#markup' =>$end,
      '#attached' => array('css' => array(drupal_get_path('module', 'smartling_translate_page_popup') . '/static/smartling_translate_page_popup.css'),
                           'js'  => array(drupal_get_path('module', 'smartling_translate_page_popup') . '/static/smartling_translate_page_popup.js',))

    );

    return $form;
  }

  public function validateForm(array &$form, array &$form_state) {
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
    $errors = form_get_errors();
    if ($errors) {
      $commands[] = ajax_command_replace('#translation_result', '<div id="translation_result" class="failed">' . implode(" ", $errors) . '</div>');
      return array('#type' => 'ajax', '#commands' => $commands);
    }
    //print_r($form_state['input']);
    //die('hi');
    //$elem
    //print_r($form_state['input'], TRUE)
    //drupal_set_message('hi', 'error');
    //rgb(194, 82, 20)
    $commands[] = (mt_rand(0,1)==0)?ajax_command_replace('#translation_result', '<div id="translation_result" class="success">Everything is ok!</div>'):
      ajax_command_replace('#translation_result', '<div id="translation_result" class="failed">Something is wrong!</div>');
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
  }
}
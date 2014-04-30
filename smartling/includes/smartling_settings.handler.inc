<?php

/**
 * @file
 * Smartling settings handler.
 */

/**
 * Class SmartlingSettingsHandler.
 */
class SmartlingSettingsHandler {

  public $apiUrl;
  public $projectId;
  public $key;
  public $retrievalType;
  public $targetLocales;
  public $localesConvertArray;
  public $callbackUrl;
  public $logMode;
  public $nodeFieldsSettings;
  public $commentFieldsSettings;
  public $taxonomy_termFieldsSettings;
  public $userFieldsSettings;

  /**
   * Initialize.
   */
  public function __construct() {
    $this->apiUrl = variable_get('smartling_api_url', 'https://capi.smartling.com/v1');
    $this->callbackUrl = variable_get('smartling_callback_url', 'smartling-callback');
    $this->logMode = variable_get('smartling_log', 1);
    $this->projectId = variable_get('smartling_project_id', '');
    $this->key = variable_get('smartling_key', '');
    $this->retrievalType = variable_get('smartling_retrieval_type', 'published');

    $this->targetLocales = variable_get('smartling_target_locales', array());
    $this->localesConvertArray = variable_get('smartling_locales_convert_array', array());

    $this->nodeFieldsSettings = variable_get('smartling_node_fields_settings', array());
    $this->commentFieldsSettings = variable_get('smartling_comment_fields_settings', array());
    $this->taxonomy_termFieldsSettings = variable_get('smartling_taxonomy_term_fields_settings', array());
    $this->userFieldsSettings = variable_get('smartling_user_fields_settings', array());
  }

  public function getPropertyName($entity_type) {
    return $entity_type . 'FieldsSettings';
  }

  public function getMethodName($entity_type) {
    return $entity_type . 'GetFieldsSettings';
  }

  public function nodeSetFieldsSettings($nodeFieldsSettings) {
    if (!empty($nodeFieldsSettings)) {
      $this->nodeFieldsSettings = $nodeFieldsSettings;
      variable_set('smartling_node_fields_settings', $nodeFieldsSettings);
    }
    else {
      $this->nodeFieldsSettings = array();
      variable_delete('smartling_node_fields_settings');
    }
  }

  public function nodeGetFieldsSettings() {
    return $this->nodeFieldsSettings;
  }

  public function nodeGetFieldsSettingsByBundle($bundle) {
    return (isset($this->nodeFieldsSettings[$bundle])) ? $this->nodeFieldsSettings[$bundle] : array();
  }

  public function commentSetFieldsSettings($commentFieldsSettings) {
    if (!empty($commentFieldsSettings)) {
      $this->commentFieldsSettings = $commentFieldsSettings;
      variable_set('smartling_comment_fields_settings', $commentFieldsSettings);
    }
    else {
      $this->commentFieldsSettings = array();
      variable_delete('smartling_comment_fields_settings');
    }
  }

  public function commentGetFieldsSettings() {
    return $this->commentFieldsSettings;
  }

  public function commentGetFieldsSettingsByBundle($bundle) {
    return (isset($this->commentFieldsSettings[$bundle])) ? $this->commentFieldsSettings[$bundle] : array();
  }

  public function taxonomy_termSetFieldsSettings($taxonomy_termFieldsSettings) {
    if (!empty($taxonomy_termFieldsSettings)) {
      $this->taxonomy_termFieldsSettings = $taxonomy_termFieldsSettings;
      variable_set('smartling_taxonomy_term_fields_settings', $taxonomy_termFieldsSettings);
    }
    else {
      $this->taxonomy_termFieldsSettings = array();
      variable_delete('smartling_taxonomy_term_fields_settings');
    }
  }

  public function taxonomy_termGetFieldsSettings() {
    return $this->taxonomy_termFieldsSettings;
  }

  public function taxonomy_termGetFieldsSettingsByBundle($bundle) {
    return (isset($this->taxonomy_termFieldsSettings[$bundle])) ? $this->taxonomy_termFieldsSettings[$bundle] : array();
  }

  public function userSetFieldsSettings($userFieldsSettings) {
    if (!empty($userFieldsSettings)) {
      $this->userFieldsSettings = $userFieldsSettings;
      variable_set('smartling_user_fields_settings', $userFieldsSettings);
    }
    else {
      $this->userFieldsSettings = array();
      variable_delete('smartling_user_fields_settings');
    }
  }

  public function userGetFieldsSettings() {
    return $this->userFieldsSettings;
  }

  public function userGetFieldsSettingsByBundle($bundle) {
    return (isset($this->userFieldsSettings[$bundle])) ? $this->userFieldsSettings[$bundle] : array();
  }

  public function addMultipleFieldsToSettings($entity_type, $bundle, $field_names = array()) {
    $nameSettings = $this->getPropertyName($entity_type);
    foreach ($field_names as $field_name) {
      $this->{$nameSettings}[$bundle][$field_name] = $field_name;
    }
    variable_set('smartling_' . $entity_type . '_translate_fields', $this->{$nameSettings});
  }

  public function addSingleFieldToSettings($entity_type, $bundle, $field_name) {
    $this->addMultipleFieldsToSettings($entity_type, $bundle, array($field_name));
  }

  public function deleteMultipleFieldsFromSettings($entity_type, $bundle, $field_names = array()) {
    $nameSettings = $this->getPropertyName($entity_type);
    foreach ($field_names as $field_name) {
      if (isset($this->{$nameSettings}[$bundle][$field_name])) {
        unset($this->{$nameSettings}[$bundle][$field_name]);

        if (count($this->{$nameSettings}[$bundle]) == 0) {
          unset($this->{$nameSettings}[$bundle]);
        }
      }
    }
    if ($this->{$nameSettings} == array()) {
      variable_delete('smartling_' . $entity_type . '_fields_settings');
    }
    else {
      variable_set('smartling_' . $entity_type . '_fields_settings', $this->{$nameSettings});
    }
  }

  public function deleteSingleFieldFromSettings($entity_type, $bundle, $field_name) {
    $this->deleteMultipleFieldsFromSettings($entity_type, $bundle, array($field_name));
  }

  public function deleteMultipleBundleFromSettings($entity_type, $bundles = array()) {
    $nameSettings = $this->getPropertyName($entity_type);
    foreach ($bundles as $bundle) {
      if (isset($this->{$nameSettings}[$bundle])) {
        unset($this->{$nameSettings}[$bundle]);
      }
    }
    if ($this->{$nameSettings} == array()) {
      variable_delete('smartling_' . $entity_type . '_fields_settings');
    }
    else {
      variable_set('smartling_' . $entity_type . '_fields_settings', $this->{$nameSettings});
    }
  }

  public function deleteSingleBundleFromSettings($entity_type, $bundle) {
    $this->deleteMultipleBundleFromSettings($entity_type, array($bundle));
  }

  public function getApiUrl() {
    return $this->apiUrl;
  }

  public function setApiUrl($apiUrl) {
    if (empty($apiUrl)) {
      $apiUrl = 'https://capi.smartling.com/v1';
    }
    $this->apiUrl = (string) $apiUrl;
    variable_set('smartling_api_url', $this->apiUrl);
  }

  public function getCallbackUrl() {
    return $this->callbackUrl;
  }

  public function setCallbackUrl($callbackUrl) {
    if (empty($callbackUrl)) {
      $callbackUrl = 'smartling-callback';
    }
    $this->callbackUrl = (string) $callbackUrl;
    variable_set('smartling_callback_url', $this->callbackUrl);
  }

  public function getLogMode() {
    return $this->logMode;
  }

  public function setLogMode($logMode = TRUE) {
    $this->logMode = (bool) $logMode;
    variable_set('smartling_log', $this->logMode);
  }

  public function getLogModeOptions() {
    return array(FALSE => 'OFF', TRUE => 'ON');
  }

  public function getProjectId() {
    return $this->projectId;
  }

  public function setProjectId($projectId) {
    if (empty($projectId)) {
      $this->projectId = NULL;
      variable_del('smartling_project_id');
    }
    else {
      $this->projectId = (string) $projectId;
      variable_set('smartling_project_id', $this->projectId);
    }
  }

  public function getKey() {
    return $this->key;
  }

  public function setKey($key) {
    if (empty($key)) {
      $this->key = NULL;
      variable_del('smartling_key');
    }
    else {
      $this->key = (string) $key;
      variable_set('smartling_key', $this->key);
    }
  }

  public function getRetrievalType() {
    return $this->retrievalType;
  }

  public function setRetrievalType($retrievalType) {
    $this->retrievalType = $retrievalType;
    variable_set('smartling_retrieval_type', $retrievalType);
  }

  public function getRetrievalTypeOptions() {
    return array('pseudo' => 'pseudo', 'published' => 'published', 'pending' => 'pending');
  }

  public function getTargetLanguageOptionsList() {
    $target_language_options_list = array();
    $languages = language_list();
    $default_language = language_default();
    unset($languages[$default_language->language]);

    foreach ($languages as $langcode => $language) {
      if ($language->enabled != '0') {
        $target_language_options_list[$langcode] = $language->name;
      }
    }
    return $target_language_options_list;
  }

  public function getTargetLocales() {
    return $this->targetLocales;
  }

  public function setTargetLocales($targetLocales) {
    $this->targetLocales = $targetLocales;
    variable_set('smartling_target_locales', $targetLocales);
  }

  public function makeTargetLocales($targetLocales) {
    foreach ($targetLocales as $key => $lang) {
      if ($lang === 0) { // Must be ===
        unset($targetLocales[$key]);
      }
    }
    if (!empty($targetLocales)) {
      $this->targetLocales = $targetLocales;
      variable_set('smartling_target_locales', $targetLocales);
    }
  }

  public function getlocalesConvertArray() {
    return $this->localesConvertArray;
  }

  public function setlocalesConvertArray($localesConvertArray) {
    $this->localesConvertArray = $localesConvertArray;
    variable_set('smartling_locales_convert_array', $localesConvertArray);
  }

  public function makeLocalesConvertArray($values) {
    $localesConvertArray = $values['target_locales'];
    foreach ($values['target_locales'] as $key => $lang) {
      if ($lang === 0) { // Must be ===
        unset($localesConvertArray[$key]);
      }
      else {
        if (!empty($values['target_locales_text_key_' . $key])) {
          $localesConvertArray[$key] = $values['target_locales_text_key_' . $key];
        }
      }
    }
    $this->localesConvertArray = $localesConvertArray;
    variable_set('smartling_locales_convert_array', $localesConvertArray);
  }

}

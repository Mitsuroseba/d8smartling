<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TitlePropertyFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

class FieldCollectionFieldProcessor extends BaseFieldProcessor {


  /**
   * Wrapper for Smartling settings storage.
   *
   * @todo avoid procedural code and inject storage to keep DI pattern.
   *
   * @return array()
   */
  protected function getTransletableFields() {
    //return smartling_settings_get_handler()->getFieldsSettings($this->entity->entity_type, $this->entity->bundle);
    return array('field_text1', 'field_some2');
  }

  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();

    //return $entity_current_translatable_content;
    if (!empty($this->entity->{$this->fieldName}[$this->sourceLanguage])) {
      foreach ($this->entity->{$this->fieldName}[$this->sourceLanguage] as $delta => $value) {
        $fid = (int)$value['value'];
        $entity = field_collection_item_load($fid);

        foreach ($this->getTransletableFields() as $field_name) {
          /* @var $fieldProcessor \Drupal\smartling\FieldProcessors\BaseFieldProcessor */
          $fieldProcessor = drupal_container()->get('smartling.field_processor_factory')->getProcessor($field_name, $entity, 'field_collection_item', $this->smartling_entity, $this->targetLanguage);

          if ($fieldProcessor) {
            $data[$fid][$field_name] = $fieldProcessor->getSmartlingContent();
          }
        }
      }
    }

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function getDrupalContent() {
    $data = $this->entity->{$this->fieldName};

    foreach ($this->smartling_entity[$this->fieldName][$this->sourceLanguage] as $delta => $value) {
      $data = $value;
    }

    return $data;
  }

  public function fetchDataFromXML(\DomXpath $xpath) {
    //@todo fetch format from xml as well.
    $result = array();
    $data = $xpath->query('//field_collection[@id="' . $this->fieldName . '"]')
      ->item(0);
    //echo $xpath->document;

    if (!$data) {
      return NULL;
    }

    $item = $data->firstChild;
    //$this->fetchDataFromXML($item);
    do {
      if ($item->tagName == 'string') {
        $eid = $item->attributes->getNamedItem('eid');
        $field = $item->attributes->getNamedItem('id');
        $delta = $item->attributes->getNamedItem('delta');

        $result[$eid->value][$field->value][$delta->value] = $item->nodeValue;
      }
    } while ($item = $item->nextSibling);

//    $quantity = $quantity_value->getAttribute('quantity');

//    for ($i = 0; $i < $quantity; $i++) {
//      $field = $xpath->query('//string[@id="' . $this->fieldName . '-' . $i . '"][1]')
//        ->item(0);
//      $data[$i]['value'] = $this->processXMLContent((string) $field->nodeValue);
//    }

    return $result;
  }


  protected function clone_fc_items($entity_type, &$entity, $fc_field, $language = LANGUAGE_NONE){
    $entity_wrapper = entity_metadata_wrapper($entity_type, $entity);
    $old_fc_items = $entity_wrapper->{$fc_field}->value();
    if (!is_array($old_fc_items)) {
      $old_fc_items = array($old_fc_items);
    }
    $field_info_instances = field_info_instances();
    $field_names = element_children($field_info_instances['field_collection_item'][$fc_field]);
    unset($entity->{$fc_field}[$language]);
    $result = array();
    foreach ($old_fc_items as $old_fc_item) {
      $old_fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $old_fc_item);
      $new_fc_item = entity_create('field_collection_item', array('field_name' => $fc_field));
      $new_fc_item->setHostEntity($entity_type, $entity);
      $new_fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $new_fc_item);
      foreach ($field_names as $field_name) {
        //if (is_array($old_fc_item->{$field_name})){
        if (!empty($old_fc_item->{$field_name})){
          $new_fc_item->{$field_name} = $old_fc_item->{$field_name};
        }
        //}
      }
      $new_fc_item_wrapper->save();
      $result[] = array('value' => $new_fc_item_wrapper->getIdentifier(), 'revision_id' => $new_fc_item_wrapper->getIdentifier());
      //Now check if any of the fields in the newly cloned fc item is a field collection and recursively call this function to properly clone it.
      foreach ($field_names as $field_name) {
        if (!empty($new_fc_item->{$field_name})){
          $field_info = field_info_field($field_name);
          if ($field_info['type'] == 'field_collection'){
            clone_fc_items('field_collection_item',$new_fc_item, $field_name,$language);
          }
        }
      }
    }
    return $result;
  }

  public function prepareBeforeDownload(array $fieldData) {
    return $this->clone_fc_items($this->entityType, $this->entity, $this->fieldName);
  }

  public function setDrupalContentFromXML($xpath) {

    $content = $this->fetchDataFromXML($xpath);

    foreach($content as $id => $val) {
      $this->saveContentToEntity($id, $val);
    }
    //$this->entity->{$this->fieldName}[$this->targetLanguage] = $content;
  }

  protected function saveContentToEntity($id, $value) {
    $entity = field_collection_item_load($id);
    $wrapper = entity_metadata_wrapper('field_collection_item', $entity);

    foreach($value as $field_name => $val) {
 //     $val[0] .= 'hi ';
        $wrapper->{$field_name}->set($val);
    }
    $wrapper->save();
    //field_collection_item_save($entity);
    //return $id;
  }

}



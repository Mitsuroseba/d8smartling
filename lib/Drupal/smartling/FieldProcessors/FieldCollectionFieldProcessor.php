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
    return smartling_settings_get_handler()->fieldCollectionGetFieldsSettingsByBundle($this->fieldName);
  }

  protected function fieldCollectionItemLoad($id) {
    return field_collection_item_load($id);
  }

  protected function getProcessor($field_name, $entity, $smartling_entity, $targetLanguage) {
    return drupal_container()->get('smartling.field_processor_factory')->getProcessor($field_name, $entity, 'field_collection_item', $smartling_entity, $targetLanguage);
  }
  /**
   * {@inheritdoc}
   */
  public function getSmartlingContent() {
    $data = array();

    if (!empty($this->entity->{$this->fieldName}[$this->sourceLanguage])) {
      foreach ($this->entity->{$this->fieldName}[$this->sourceLanguage] as $delta => $value) {
        $fid = (int) $value['value'];
        $entity = $this->fieldCollectionItemLoad($fid);

        foreach ($this->getTransletableFields() as $field_name) {
          /* @var $fieldProcessor \Drupal\smartling\FieldProcessors\BaseFieldProcessor */
          $fieldProcessor = $this->getProcessor($field_name, $entity, $this->smartling_entity, $this->targetLanguage);

          if ($fieldProcessor) {
            $data[$fid][$field_name] = $fieldProcessor->getSmartlingContent();
          }
        }
      }
    }

    return $data;
  }

  public function fetchDataFromXML(\DomXpath $xpath) {
    //@todo fetch format from xml as well.
    $result = array();
    $data = $xpath->query('//field_collection[@id="' . $this->fieldName . '"]')
      ->item(0);

    if (!$data) {
      return NULL;
    }

    $item = $data->firstChild;
    do {
      if ($item->tagName == 'string') {
        $eid = $item->attributes->getNamedItem('eid');
        $field = $item->attributes->getNamedItem('id');
        $delta = $item->attributes->getNamedItem('delta');
        //$quantity = $item->attributes->getNamedItem('quantity');

        $result[$eid->value][$field->value][$delta->value] = $item->nodeValue;
      }
    } while ($item = $item->nextSibling);

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
      //$old_fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $old_fc_item);
      $new_fc_item = entity_create('field_collection_item', array('field_name' => $fc_field));
      $new_fc_item->setHostEntity($entity_type, $entity);
      $new_fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $new_fc_item);
      foreach ($field_names as $field_name) {
        if (!empty($old_fc_item->{$field_name})){
          $new_fc_item->{$field_name} = $old_fc_item->{$field_name};
        }
      }
      $new_fc_item_wrapper->save();
     // field_attach_update($entity_type, $entity);
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

  public function setDrupalContentFromXML($fieldValue) {

    $content = $fieldValue;

    //$values = $this->entity->{$this->fieldName}[LANGUAGE_NONE];
    if (empty($values)) {
      return;
    }

    //$id = current($values);
    foreach($content as $id => $val) {
      $this->saveContentToEntity($id, $val);
      //$id = next($values);
    }
  }

  protected function saveContentToEntity($id, $value) {
    $entity = $this->fieldCollectionItemLoad($id);

    $fieldProcessorFactory = drupal_container()->get('smartling.field_processor_factory');
    foreach ($value as $field_name => $fieldValue) {
      $smartling_entity = clone $this->entity;
      // @TODO test if format could be set automatically.
      $fieldProcessor = $fieldProcessorFactory->getProcessor($field_name, $entity, 'field_collection_item', $smartling_entity, LANGUAGE_NONE);
      $fieldProcessor->setDrupalContentFromXML($fieldValue);
    }

    entity_save('field_collection_item', $entity);
  }

  public function cleanBeforeClone($entity) {
    $val = '';
    $field_name = $this->fieldName;
    if (isset($entity->{$field_name})) {
      $val = $entity->{$field_name};
      unset($entity->{$field_name});
    }
    return $val;
  }

  public function putDataToXML($xml, $localize, $data, $fieldName = NULL) {
    $collection = $xml->createElement('field_collection');
    $attr = $xml->createAttribute('id');
    $attr->value = !empty($fieldName) ? $fieldName : $this->fieldName;
    $collection->appendChild($attr);

    foreach($data as $eid => $field_collection) {
      foreach ($field_collection as $key => $value) {
        $quantity = count($value);
        foreach ($value as $item_key => $item) {
          if (is_array($item)) {
            $this->putDataToXML($xml, $collection, array($item_key => $item), $key);
          }
          else {
            $string = $xml->createElement('string');

            $string_attr = $xml->createAttribute('eid');
            $string_attr->value = $eid;
            $string->appendChild($string_attr);

            $string_attr = $xml->createAttribute('id');
            $string_attr->value = $key;
            $string->appendChild($string_attr);

            $string_attr = $xml->createAttribute('delta');
            $string_attr->value = $item_key;
            $string->appendChild($string_attr);

            $string_val = $xml->createTextNode($item);
            $string->appendChild($string_val);

            $string_attr = $xml->createAttribute('quantity');
            $string_attr->value = $quantity;
            $string->appendChild($string_attr);

            $collection->appendChild($string);
            $localize->appendChild($collection);
          }
        }
      }
    }
  }
}




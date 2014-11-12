<?php

/**
 * @file
 * Contains Drupal\smartling\FieldProcessors\TitlePropertyFieldProcessor.
 */

namespace Drupal\smartling\FieldProcessors;

use Drupal\smartling\FieldProcessorFactory;

class FieldCollectionFieldProcessor extends BaseFieldProcessor {

  protected $fieldFactory;

  public function __construct($entity, $entity_type, $field_name, $smartling_data, $source_language, $target_language) {
    parent::__construct($entity, $entity_type, $field_name, $smartling_data, $source_language, $target_language);

    $this->fieldFactory = drupal_container()->get('smartling.field_processor_factory');

    return $this;
  }

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

  protected function getProcessor($field_name, $entity, $smartling_entity, $targetLanguage, $sourceLanguage) {
    return drupal_container()->get('smartling.field_processor_factory')->getProcessor($field_name, $entity, 'field_collection_item', $smartling_entity, $targetLanguage, $sourceLanguage);
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
          $fieldProcessor = $this->getProcessor($field_name, $entity, $this->smartling_entity, $this->targetLanguage, $this->sourceLanguage);

          if ($fieldProcessor) {
            $data[$fid][$field_name] = $fieldProcessor->getSmartlingContent();
          }
        }
      }
    }

    return $data;
  }

  protected function getTranslatableFields() {
    // @todo Inject via DIC.
    return smartling_settings_get_handler()->getFieldsSettings('field_collection', $this->entity->field_name);
  }

  protected function importSmartlingXMLToFieldCollectionEntity(\DomXpath $xpath) {
    $point = null;
    foreach ($this->getTranslatableFields() as $field_name) {
      // @TODO test if format could be set automatically.
      $fieldProcessor = $this->fieldFactory->getProcessor($field_name, $this->entity, 'field_collection_item', $this->smartling_entity, $this->targetLanguage);
      $fieldValue = $fieldProcessor->fetchDataFromXML($xpath);
      $fieldProcessor->setDrupalContentFromXML($fieldValue);
    }

    unset($fieldProcessor);
    entity_save('field_collection_item', $this->entity);
  }

  public function fetchDataFromXML(\DomXpath $xpath) {
    //@todo fetch format from xml as well.
    $result = array();
    $data = $xpath->query('/data/localize/field_collection[@id="' . $this->fieldName . '"]');

    if (!$data->length) {
      $data = $xpath->query('//field_collection[@id="' . $this->fieldName . '"]');

      if (!$data->length) {
        return FALSE;
      }
    }

    $delta = 0;
    foreach ($data as $field_collection_tag) {
      $eid = $this->entity->{$this->fieldName}[$this->targetLanguage][$delta]['value'];
      $parentEntity = clone $this->entity;
      $this->entity = field_collection_item_load($eid);
      $this->entity = entity_load_single('field_collection_item', $eid);
      $host_entity = $this->entity->hostEntity();
      $doc = new \DOMDocument();
      $nested_item = $field_collection_tag->cloneNode(TRUE);
      $doc->appendChild($doc->importNode($nested_item, TRUE));
      $nested_xpath = new \DomXpath($doc);
      $this->importSmartlingXMLToFieldCollectionEntity($nested_xpath);
      $delta++;
      $this->entity = $parentEntity;
    }

//    $item = $data->firstChild;
//    $delta = 0;
//    $eid = $data->attributes->getNamedItem('eid')->value;
//    $eid = $this->entity->{$this->fieldName}[$this->sourceLanguage][$delta]['value'];
//    do {
//      $this->entity = field_collection_item_load($eid);
//      $doc = new \DOMDocument();
//      $nested_item = $item->cloneNode(TRUE);
//      $doc->appendChild($doc->importNode($nested_item, TRUE));
//      $nested_xpath = new \DomXpath($doc);
//      $this->importSmartlingXMLToFieldCollectionEntity($nested_xpath);
////      if ($item->tagName == 'string') {
////        $field = $item->attributes->getNamedItem('id')->value;
////        $string_delta = $item->attributes->getNamedItem('delta')->value;
////        $result[$eid][$field][$string_delta] = $item->nodeValue;
////      }
////      elseif ($item->tagName == 'field_collection') {
//////        $eid = $item->attributes->getNamedItem('eid')->value;
////        $field = $item->attributes->getNamedItem('id')->value;
////        // Ugly DOM* PHP API requires DOMDocument here.
////        $doc = new \DOMDocument();
////        $nested_item = $item->cloneNode(TRUE);
////        $doc->appendChild($doc->importNode($nested_item, TRUE));
////        $nested_xpath = new \DomXpath($doc);
////        $entity = $this->fieldCollectionItemLoad($eid);
////        $smartling_entity = clone $this->smartling_entity;
////        $fieldProcessor = $this->fieldFactory->getProcessor($field, $entity, 'field_collection', $smartling_entity, $this->targetLanguage);
////        $result[$eid][$field][$delta] = $fieldProcessor->fetchDataFromXML($nested_xpath);
////        unset($fieldProcessor);
////        $delta++;
////      }
//      $delta++;
//    } while ($item = $item->nextSibling);

    return array(array('value' => $eid));
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
            $this->clone_fc_items('field_collection_item', $new_fc_item, $field_name, $language);
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
    return;
    $content = $fieldValue;

//    //$values = $this->entity->{$this->fieldName}[LANGUAGE_NONE];
//    if (empty($values)) {
//      return;
//    }

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
      $smartling_entity = clone $this->smartling_entity;
      $fieldProcessor = $fieldProcessorFactory->getProcessor($field_name, $entity, 'field_collection_item', $smartling_entity, LANGUAGE_NONE);
      $fieldProcessor->setDrupalContentFromXML($fieldValue);
      unset($fieldProcessor);
    }

    unset($fieldProcessorFactory);
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
    foreach ($data as $entity_id => $field_collection) {
      $collection = $xml->createElement('field_collection');
      $attr = $xml->createAttribute('id');
      $attr->value = !empty($fieldName) ? $fieldName : $this->fieldName;
      $collection->appendChild($attr);
      $attr = $xml->createAttribute('eid');
      $attr->value = $entity_id;
      $collection->appendChild($attr);

      foreach ($field_collection as $field_name => $value) {
        foreach ($value as $delta => $item) {
          // If field value is an array and value key is valid field name
          // then process it as nested field collection.
          if (is_array($item) && static::isFieldOfType($field_name, 'field_collection')) {
            $this->putDataToXML($xml, $collection, array($delta => $item), $field_name);
          }
          else {
            $fieldProcessor = $this->fieldFactory->getProcessor($field_name, $this->entity, $this->entityType, $this->smartling_entity, $this->targetLanguage);
            $fieldProcessor->putDataToXml($xml, $collection, array($delta => $item));
          }

          $localize->appendChild($collection);
        }
      }
    }
  }

  protected function buildSingleStringTag($xml, $entity_id, $field_name, $delta, $quantity, $value) {
    $string = $xml->createElement('string');

    $string_attr = $xml->createAttribute('eid');
    $string_attr->value = $entity_id;
    $string->appendChild($string_attr);

    $string_attr = $xml->createAttribute('id');
    $string_attr->value = $field_name;
    $string->appendChild($string_attr);

    $string_attr = $xml->createAttribute('delta');
    $string_attr->value = $delta;
    $string->appendChild($string_attr);

    $string_val = $xml->createTextNode($value);
    $string->appendChild($string_val);

    $string_attr = $xml->createAttribute('quantity');
    $string_attr->value = $quantity;
    $string->appendChild($string_attr);

    return $string;
  }
}




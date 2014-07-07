<?php

/**
 * @file
 * Class SmartlingContentMediaProcessor.
 */

namespace Smartling\Alters;

use Smartling\Alters\ISmartlingContentProcessor;

/*
 * Demo url processor. No real value here for now.
 */
class SmartlingContentMediaProcessor implements ISmartlingContentProcessor {

  protected function getFileById($fid) {
    return file_load($fid);//entity_load('file', FALSE, array('fid' => $fid));
  }

  protected function getFileByName($fname) {
    return entity_load('file', FALSE, array('filename' => $fname));
  }


  public function process(&$item, $context, $lang, $field_name, $entity) {
    $file = $this->getFileById($context[0][0][0]->fid);
    if (empty($file)) {
      return;
    }
    $new_file = $this->getFileByName($lang . '_' . $file->filename);


    if ($new_file) {
      $media_content = json_decode($item[1]);
      $media_content[0][0]->fid = $new_file[0]->fid;
      $item[1] = json_encode($media_content);
    }
  }
}
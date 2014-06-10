<?php

interface ISmartlingContentParser {
  function __construct(array $processors);
  function parse($content, $field_name, $entity);
}

<?php

/**
 * @file
 * A feeds mapping implementation for our particular Multifield for authors.
 *
 * Adapted from a patch on comment 21 of this issue:
 * https://www.drupal.org/project/multifield/issues/2414049
 *
 * Set the xpath so it returns all the names with their full XML.
 */

/**
 * Implements hook_feeds_processor_targets_alter().
 */
function biblio_feeds_processor_targets_alter(&$targets, $entity_type, $bundle_name) {

  foreach (field_info_instances($entity_type, $bundle_name) as $name => $instance) {
    $info = field_info_field($name);
    if ($instance['field_name'] == 'field_biblio_authors') {
      $targets['field_biblio_authors'] = array(
        'name'        => check_plain($instance['label'] . t(' (Multifield: ') . $instance['field_name'] . ')'),
        'callback'    => 'biblio_feeds_set_target',
        'description' => t('The field instance @label.', array(
          '@label' => $instance['label'],
          '@id'    => $name,
        )),
      );
    }
  }
}

/**
 * Entity reference callback for mapping.
 *
 * When the callback is invoked, $target contains the name of the field the
 * user has decided to map to and $values contains the value of the feed item
 * element the user has picked as a source.
 *
 * @param $source
 *   A FeedsSource object.
 * @param $entity
 *   The entity to map to.
 * @param $target
 *   The target key on $entity to map to.
 * @param $values
 *   The value to map. MUST be an array.
 */
function biblio_feeds_set_target($source, $entity, $target, $values, $mapping) {

  // Don't do anything if we weren't given any data.
  if (empty($values)) {
    return;
  }
  $info = field_info_field($target);

  // Set the language of the field depending on the mapping.
  $language = isset($mapping['language']) ? $mapping['language'] : LANGUAGE_NONE;
  // Hack! Set the feed to autocreate terms if they do not exist.
  $mapping['autocreate'] = TRUE;

  // Iterate over all values.
  $iterator = 0;
  $field = isset($entity->$target) ? $entity->$target : array();

  $subfields = field_info_instances('multifield', $target);
  foreach ($values as $value) {
    // Only process if this value was set for this instance.
    if ($value) {
      // We should have the full XML for this name, which would look like:
      // <name type="personal"><namePart type="family">Protzen</namePart><namePart type="given">Jean-Pierre</namePart><role><roleTerm type="code" authority="marcrelator">edt</roleTerm></role></name>
      // Corporate names may nto have family/given.
      $xml = new SimpleXMLElement('<?xml version="1.0" encoding="UTF-8"?>' . "\n" . $value);
      $family = _biblio_get_xml_value($xml, 'namePart[@type="family"]');
      $given = _biblio_get_xml_value($xml, 'namePart[@type="given"]');
      $other = _biblio_get_xml_value($xml, 'namePart[not(@type="given") and not(@type="family")]');
      if ($family && $given) {
        $family .= ", ";
      }
      $role = _biblio_get_xml_value($xml, 'role/roleTerm');
      $subvalues = array(
        $family . $given . $other,
        'primary',
        $role,
      );

      $delta = 0;
      foreach($subfields as $subfield => $subinstance) {
        $subinfo = field_info_field($subfield);
        $dummyObj = new stdclass();
        switch ($subinfo['type']) {
          case 'date':
          case 'datestamp':
          case 'datetime':
            $function = 'text_feeds_set_target';
            break;
          case 'file':
          case 'image':
            $function = 'file_feeds_set_target';
            break;
          case 'list_text':
          case 'text':
          case 'text_long':
          case 'text_with_summary':
            $function = 'text_feeds_set_target';
            break;
          case 'list_integer':
          case 'list_float':
          case 'list_boolean':
          case 'number_integer':
          case 'number_decimal':
          case 'number_float':
            $function = 'number_feeds_set_target';
            break;
          case 'entityreference':
            $function = 'entityreference_feeds_set_target';
            break;
          case 'taxonomy_term_reference':
            $function = 'taxonomy_feeds_set_target';
            break;
        }

        if(function_exists($function)) {
          $function($source, $dummyObj, $subfield, array($subvalues[$delta]), $mapping);
        }

        if(!empty($dummyObj->$subfield)) {
          $field[$language][$iterator][$subfield] = $dummyObj->$subfield;
        }
        $delta++;
      }
    }

    // Break out of the loop if this field is single-valued.
    if ($info['cardinality'] == 1) {
      break;
    }
    $iterator++;
  }
  // Add the field to the entity definition.
  $entity->{$target} = $field;
}

/**
 * Extracts the value of the element by xpath.
 */
function _biblio_get_xml_value($xml, $xpath) {
  $nodes = $xml->xpath($xpath);
  $return = '';
  foreach ($nodes as $node) {
    $return .= (string) $node;
  }

  return $return;
}
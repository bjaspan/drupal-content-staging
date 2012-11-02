<?php

/**
 * @file
 * Services client allows you to push different objects from local drupal installation
 * to remote servers via REST api.
 */

/**
 * This hooks allows to alter source object which is going to be mapped
 * to data sent via services. Use this hook to introduce new properties
 * that can be easily mapped to remote objects.
 *
 * @param $object
 *   Object that should be altered.
 * @param $type
 *   String representation of object type
 *   - 'user'
 *   - 'node'
 */
function hook_services_client_data_alter(&$object, $type) {
  // Create timestamp from field_expiration
  $object->expiration_date = strtotime($object->field_expiration[0]['value']);
}

/**
 * Allows to exclude data from being sent
 *
 * @param type $object
 * @param type $type
 */
function hook_services_client_data_exclude($object, $type) {
  
}
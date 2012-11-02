<?php

/**
 * @file
 * Services client drush integration
 */

/**
 * Implementation of hook_drush_commands()
 */
function services_client_drush_command() {
  $items = array();

  $items['services-client-entity-mapping'] = array(
    'description' => "Creates entity mapping",
    'arguments' => array(
      'entity_type' => 'Type of the entity',
      'entity_bundle' => '',
    ),
    'options' => array(
      'mirror' => '',
      'language' => '',
    ),
    'examples' => array(
      'drush sc-em node page' => 'Cretae mapping for node',
    ),
    'aliases' => array('sc-em'),
  );

  $items['services-client-process-queue'] = array(
    'description' => "Process entries from queue",
    'options' => array(
      'time' => '',
    ),
    'examples' => array(
      'drush sc-pq' => 'Process queued synchronizations',
    ),
    'aliases' => array('sc-pq')
  );

  $items['services-client-sync-users'] = array(
    'description' => "Sync user references",
    'options' => array(
      'min' => 'Min uid of user',
      'max' => 'Max uid of user',
      'hook' => 'Hook name',
    ),
    'examples' => array(
      'drush sc-su --hook=docs_user --min=1 --max=5000' => 'Sync all users',
    ),
    'aliases' => array('sc-su'),
  );

  $items['services-client-sync-nodes'] = array(
    'description' => "Sync nodes",
    'options' => array(
      'min' => 'Min nid of node',
      'max' => 'Max nid of node',
      'type' => 'Node type',
      'hook' => 'Hook name',
    ),
    'examples' => array(
      'drush sc-su --hook=insight_node --min=1 --max=5000' => 'Sync all users',
    ),
    'aliases' => array('sc-sn'),
  );


  return $items;
}

/**
 * Create mapping for entity
 *
 * @param $entity_type
 * @param $entity_bundle
 */
function drush_services_client_entity_mapping($entity_type, $entity_bundle) {
  $mirror = drush_get_option('mirror', FALSE);
  $language = drush_get_option('language', 'und');

  $properties = entity_get_property_info($entity_type);
  $bundle_fields = array_keys($properties['bundles'][$entity_bundle]['properties']);
  $all_fields = field_info_fields();

  $fields = array();
  foreach ($bundle_fields as $field_name) {
    $fields[$field_name] = $all_fields[$field_name];
  }

  $properties = array_keys($properties['properties']);

  $output = array();

  foreach ($properties as $property) {
    if ($mirror) {
      $property = "{$property}|{$property}";
    }
    $output[] = $property;
  }

  foreach ($fields as $field_name => $field) {
    foreach ($field['columns'] as $column_name => $info) {
      $line = implode(SERVICES_CLIENT_DELMITER_ARRAY, array($field_name, $language, 0, $column_name));
      if ($mirror) {
        $line = "{$line}|{$line}";
      }
      $output[] = $line;
    }
  }

  drush_print(implode("\n", $output));
}

/**
 * Process queued sync jobs
 */
function drush_services_client_process_queue() {
  // How long can command run
  $time = drush_get_option('time', 55);
  $end = time() + $time;

  // Run only one process at time
  if (lock_acquire('services_client_process_queue', (float) $time)) {
    $queue = DrupalQueue::get('services_client_sync');
    while (time() < $end && ($item = $queue->claimItem())) {
      services_client_queue_sync($item->data);
      $queue->deleteItem($item);
    }
    lock_release('services_client_process_queue');
  }
}

/**
 * Sync all user references to insight
 */
function drush_services_client_sync_users() {
  // Disable failure notifications for drush run
  global $conf;
  $conf['services_client_notify'] = FALSE;

  $min = drush_get_option('min');
  $max = drush_get_option('max');
  $hook = drush_get_option('hook', NULL);

  $query = db_select('users', 'u', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('u', array('uid'));

  if ($min) {
    $query->condition('uid', $min, '>=');
  }

  if ($max) {
    $query->condition('uid', $max, '<');
  }

  $result = $query->execute();

  foreach ($result as $row) {
    $attempts = 0;
    $sucessful = FALSE;

    // Make 3 attempts to sync node
    while (!$sucessful && $attempts < 3) {
      unset($user->_services_client);
      $attempts++;
      $account = user_load($row['uid']);
      $result = services_client_data_process($account, 'user_save', $hook);
      if (!count($result['errors'])) {
        $sucessful = TRUE;
      }
      elseif ($attempts < 3) {
        sleep($attempts);
      }
    }

    if (!$sucessful) {
      drush_print(dt("ERROR: !uid|!name", array(
        '!uid' => $account->uid,
        '!name' => $account->name,
      )));
    }

    // Prevent memory from filling
    drupal_static_reset();
  }
}

/**
 * Sync all nodes to specific connection
 */
function drush_services_client_sync_nodes() {
  // Disable failure notifications for drush run
  global $conf;
  $conf['services_client_notify'] = FALSE;

  $min = drush_get_option('min');
  $max = drush_get_option('max');
  $hook = drush_get_option('hook', NULL);
  $type = drush_get_option('type', NULL);

  $query = db_select('node', 'n', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('n', array('nid'));

  if ($min) {
    $query->condition('nid', $min, '>=');
  }

  if ($max) {
    $query->condition('nid', $max, '<');
  }

  if ($type) {
    $query->condition('type', $type);
  }

  $result = $query->execute();

  foreach ($result as $row) {
    $attempts = 0;
    $sucessful = FALSE;

    $node = node_load($row['nid']);

    // Make 3 attempts to sync node
    while (!$sucessful && $attempts < 3) {
      unset($node->_services_client);
      $attempts++;
      $sc_result = services_client_data_process($node, 'node_save', $hook);
      if (!count($sc_result['errors'])) {
        $sucessful = TRUE;
      }
      elseif ($attempts < 3) {
        sleep($attempts);
      }
    }

    if (!$sucessful) {
      drush_print(dt("ERROR: !nid|!title", array(
        '!nid' => $node->nid,
        '!title' => $node->title,
      )));
    }

    // Reset static caches
    drupal_static_reset();
  }
}

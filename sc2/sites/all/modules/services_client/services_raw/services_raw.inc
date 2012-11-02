<?php

/**
 * @file
 * Custom services definition and implementation of all callbacks.
 */

/**
 * Provides API definition of provided services objects and operations.
 */
function _services_raw_services_resources() {
  $services = array();

  // Node
  $services['node_raw'] = array(
    'create' => array(
      'help' => 'Creates a node using direct node_save call.',
      'file' => array('file' => 'inc', 'module' => 'services_raw'),
      'callback' => '_services_raw_node_create',
      'args' => array(
         array(
          'name' => 'node',
          'type' => 'struct',
          'description' => 'The node object',
          'source' => 'data',
          'optional' => FALSE,
        ),
      ),
      'access callback' => '_services_raw_node_access',
      'access arguments' => array('create'),
      'access arguments append' => TRUE,
    ),
    'update' => array(
      'help' => 'Update a node using direct node_save call.',
      'file' => array('file' => 'inc', 'module' => 'services_raw'),
      'callback' => '_services_raw_node_update',
      'args' => array(
        array(
          'name' => 'nid',
          'optional' => FALSE,
          'source' => array('path' => 0),
          'type' => 'int',
          'description' => 'The nid of the node to get',
        ),
        array(
          'name' => 'node',
          'optional' => FALSE,
          'source' => 'data',
          'description' => 'The node data to update',
          'type' => 'array',
        ),
      ),
      'access callback' => '_services_raw_node_access',
      'access arguments' => array('update'),
      'access arguments append' => TRUE,
    ),
  );

  // User
  $services['user_raw'] = array(
    'create' => array(
      'help' => 'Creates a user using direct user_save',
      'file' => array('file' => 'inc', 'module' => 'services_raw'),
      'callback' => '_services_raw_user_create',
      'access callback' => '_services_raw_user_access',
      'access arguments' => array('create'),
      'access arguments append' => FALSE,
      'args' => array(
        array(
          'name' => 'account',
          'type' => 'array',
          'description' => 'The user object',
          'source' => 'data',
          'optional' => FALSE,
        ),
      ),
    ),

    'update' => array(
      'help' => 'Updates a user using direct user_save',
      'file' => array('file' => 'inc', 'module' => 'services_raw'),
      'callback' => '_services_raw_user_update',
      'access callback' => '_services_raw_user_access',
      'access arguments' => array('update'),
      'access arguments append' => TRUE,
      'args' => array(
        array(
          'name' => 'uid',
          'type' => 'int',
          'description' => 'Unique identifier for this user',
          'source' => array('path' => 0),
          'optional' => FALSE,
        ),
        array(
          'name' => 'data',
          'type' => 'array',
          'description' => 'The user object with updated information',
          'source' => 'data',
          'optional' => FALSE,
        ),
      ),
    ),
  );

  return $services;
}

//
// Node
//

/**
 * Determine whether user has access to node resource and raw node_save.
 *
 * @param $op
 *   Operation
 * @param $args
 *   Additional arguments passed to call
 */
function _services_raw_node_access($op = 'view', $args = array()) {
  // Load original node resource
  module_load_include('inc', 'services', 'resources/node_resource');
  // Check access
  return user_access('access raw node_save') && _node_resource_access($op, $args);
}

/**
 * Update node calling raw node_save.
 *
 * @param $nid
 *   Nid of saved node
 * @param $node
 *   Array of node that needs to be saved
 */
function _services_raw_node_update($nid, $node) {
  // Load original node resource
  module_load_include('inc', 'services', 'resources/node_resource');

  // Adds backwards compatability with regression fixed in #1083242
  $node = _services_arg_value($node, 'node');

  $node['nid'] = $nid;

  $old_node = node_load($nid);
  if (empty($old_node->nid)) {
    return services_error(t('Node @nid not found', array('@nid' => $nid)), 404);
  }

  // If revision is not included load revision of existing node
  if (empty($node['vid']) && $old_node->vid) {
    $node['vid'] = $old_node->vid;
  }

  // Validate the node. If there is validation error Exception will be thrown
  // so code below won't be executed.
  _node_resource_validate_type($node);

  // Check if call isn't trying to change node type
  if ($old_node->type != $node['type']) {
    return services_error(t("You can't change node type via services"), 406);
  }

  try {
    $node = (object) $node;
    node_save($node);

    if (empty($node->nid)) {
      return services_error(t('Error when saving node.'), 406);
    }
  }
  catch (Exception $e) {
    return services_error(t('Error when saving node.'), 406, array('error' => $e->getMessage()));
  }

  $result = array('nid' => $node->nid);
  if (($uri = services_resource_uri(array('node', $node->nid)))) {
    $result['uri'] = $uri;
  }
  return $result;
}

/**
 * Create node calling raw node_save.
 *
 * @param $node
 *   Array of node that needs to be saved
 */
function _services_raw_node_create($node) {
  // Load original node resource
  module_load_include('inc', 'services', 'resources/node_resource');
  
  // Adds backwards compatability with regression fixed in #1083242
  $node = _services_arg_value($node, 'node');

  if (!isset($node['name']) && !isset($node['uid'])) {
    // Assign username to the node from $user created at auth step.
    global $user;
    $node['name'] = $user->name;
  }

  // Validate the node. If there is validation error Exception will be thrown
  // so code below won't be executed.
  _node_resource_validate_type($node);

  // Prepare node object, ensure that we're not going to update some node.
  $node = (object) $node;
  $node->is_new = TRUE;
  unset($node->nid);

  node_submit($node);
  node_save($node);

  if (!$node->nid) {
    return services_error(t('Error when saving node'), 406);
  }

  // Only add the URI for servers that support it.
  $result = array('nid' => $node->nid);
  if (($uri = services_resource_uri(array('node', $node->nid)))) {
    $result['uri'] = $uri;
  }
  return $result;
}

//
// User
//

/**
 * Determine whether user has access to node resource and raw node_save.
 *
 * @param $op
 *   Operation
 * @param $args
 *   Additional arguments passed to call
 */
function _services_raw_user_access($op = 'view', $args = array()) {
  // Load original node resource
  module_load_include('inc', 'services', 'resources/user_resource');
  // Check access
  $result = user_access('access raw user_save') && _user_resource_access($op, $args);
  return $result;
}

/**
 * Create new user account using user_save
 *
 * @param $account
 *   Account that should be created
 */
function _services_raw_user_create($account) {
  // Load original node resource
  module_load_include('inc', 'services', 'resources/user_resource');

  // Adds backwards compatability with regression fixed in #1083242
  $account = _services_arg_value($account, 'account');

  // Prevent re-writing user
  unset($account['uid']);

  // Check if user email already exists
  $exists = db_select('users', 'u')
    ->fields('u', array('mail'))
    ->condition('u.mail', $account['mail'])
    ->countQuery()
    ->execute()
    ->fetchField();
  if ($exists) {
    return services_error(t("Error when saving user account."), 406, array(
      'error' => t('Account with specified email @mail already exists.', array(
        '@mail' => $account['mail']
       )
    )));
  }

  try {
    if (!($account = user_save(NULL, $account))) {
      return services_error(t("Error when saving user account."), 406);
    }
  }
  catch (Exception $e) {
    return services_error(t("Error when saving user account."), 406, array('error' => $e->getMessage()));
  }

  $result = array('uid' => $account->uid);
  if (($uri = services_resource_uri(array('user', $account->uid)))) {
    $result['uri'] = $uri;
  }
  return $result;
}

/**
 * Update user account using user_save
 *
 * @param $uid
 * @param $account
 *   Account that should be created
 */
function _services_raw_user_update($uid, $account) {
  // Adds backwards compatability with regression fixed in #1083242
  $account = _services_arg_value($account, 'data');

  $original_account = user_load($uid);

  $account['uid'] = $uid;

  try {
    if (!($account = user_save($original_account, $account))) {
      return services_error(t("Error when saving user account."), 406);
    }
  }
  catch (Exception $e) {
    return services_error(t("Error when saving user account."), 406, array('error' => $e->getMessage()));
  }

  $result = array('uid' => $account->uid);
  if (($uri = services_resource_uri(array('user', $account->uid)))) {
    $result['uri'] = $uri;
  }
  return $result;
}

//
// Comment
//

//
// Taxonomy
//

//
// File
//

<?php

/**
 * @file
 * Builds placeholder replacement tokens for nodes.
 */

/**
 * Implements hook_token_info().
 */
function uuid_node_token_info() {
  $tokens = array();
  $tokens['uuid'] = array(
    'name' => t('Node UUID'), 
    'description' => t('The universally unique ID of the node.'),
  );

  $tokens['revision-uuid'] = array(
    'name' => t('Node Revision UUID'),
    'description' => t('The universally unique ID of the node revision.'),
  );

  return array(
    'tokens' => array('node' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function uuid_node_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ('node' != $type || empty($data['node'])) {
    return $replacements;
  }

  foreach ($tokens as $name => $original) {
    switch ($name) {
      case 'uuid':
        $replacements[$original] = $data['node']->uuid;
        break;

      case 'revision-uuid':
        $replacements[$original] = $data['node']->vuuid;
        break;
    }
  }
  return $replacements;
}

<?php

/**
 * @file
 * Builds placeholder replacement tokens for users.
 */

/**
 * Implements hook_token_info().
 */
function uuid_user_token_info() {
  $tokens = array();
  $tokens['uuid'] = array(
    'name' => t('User UUID'), 
    'description' => t('The universally unique ID of the user.'),
  );

  return array(
    'tokens' => array('user' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function uuid_user_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ('user' != $type) {
    return $replacements;
  }

  foreach ($tokens as $name => $original) {
    switch ($name) {
      case 'uuid':
        $replacements[$original] = $data['user']->uuid;
        break;
    }
  }
  return $replacements;
}

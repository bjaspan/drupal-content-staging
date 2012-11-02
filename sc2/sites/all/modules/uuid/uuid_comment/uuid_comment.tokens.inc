<?php

/**
 * @file
 * Builds placeholder replacement tokens for comments.
 */

/**
 * Implements hook_token_info().
 */
function uuid_comment_token_info() {
  $tokens = array();
  $tokens['uuid'] = array(
    'name' => t('Comment UUID'), 
    'description' => t('The universally unique ID of the comment.'),
  );

  return array(
    'tokens' => array('comment' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function uuid_comment_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ('comment' != $type || empty($data['comment'])) {
    return $replacements;
  }

  foreach ($tokens as $name => $original) {
    switch ($name) {
      case 'uuid':
        $replacements[$original] = $data['comment']->uuid;
        break;
    }
  }
  return $replacements;
}

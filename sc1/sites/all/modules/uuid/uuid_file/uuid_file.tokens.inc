<?php

/**
 * @file
 * Builds placeholder replacement tokens for files.
 */

/**
 * Implements hook_token_info().
 */
function uuid_file_token_info() {
  $tokens = array();
  $tokens['uuid'] = array(
    'name' => t('File UUID'), 
    'description' => t('The universally unique ID of the file.'),
  );

  return array(
    'tokens' => array('file' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function uuid_file_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ('file' != $type || empty($data['file'])) {
    return $replacements;
  }

  foreach ($tokens as $name => $original) {
    switch ($name) {
      case 'uuid':
        $replacements[$original] = $data['file']->uuid;
        break;
    }
  }
  return $replacements;
}

<?php

/**
 * @file
 * Builds placeholder replacement tokens for taxonomys.
 */

/**
 * Implements hook_token_info().
 */
function uuid_taxonomy_token_info() {
  $tokens = array();
  $tokens['uuid'] = array(
    'name' => t('Taxonomy Term UUID'), 
    'description' => t('The universally unique ID of the taxonomy term.'),
  );

  return array(
    'tokens' => array('term' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function uuid_taxonomy_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ('term' != $type || empty($data['term'])) {
    return $replacements;
  }

  foreach ($tokens as $name => $original) {
    switch ($name) {
      case 'uuid':
        $replacements[$original] = $data['term']->uuid;
        break;
    }
  }
  return $replacements;
}

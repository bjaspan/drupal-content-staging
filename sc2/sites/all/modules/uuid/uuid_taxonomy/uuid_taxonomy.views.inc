<?php

/**
 * @file
 * Views related functions for uuid_taxonomy.
 */

/**
 * Implements hook_views_alter_data().
 */
function uuid_taxonomy_views_data_alter(&$data) {
  $data['taxonomy_term_data']['uuid'] = array(
    'title' => t('UUID'),
    'help' => t('The term UUID.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
}

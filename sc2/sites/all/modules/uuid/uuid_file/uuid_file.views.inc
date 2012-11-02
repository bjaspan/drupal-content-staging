<?php

/**
 * @file
 * Views related functions for uuid_file.
 */

/**
 * Implements hook_views_alter_data().
 */
function uuid_file_views_data_alter(&$data) {
  $data['file_managed']['uuid'] = array(
    'title' => t('UUID'),
    'help' => t('The file UUID.'),
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

<?php

/**
 * @file
 * Views related functions for uuid_node.
 */

/**
 * Implements hook_views_alter_data().
 */
function uuid_node_views_data_alter(&$data) {
  $data['node']['uuid'] = array(
    'title' => t('UUID'),
    'help' => t('The node UUID.'),
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

  $data['node_revision']['uuid'] = array(
    'title' => t('UUID'),
    'help' => t('The node revision UUID.'),
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

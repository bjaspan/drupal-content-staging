<?php

/**
 * @file
 * Plugin definition
 */

/**
 * List of auth plugins provided by module
 */
function _services_client_connection_auth() {
  $path = drupal_get_path('module', 'services_client_connection') . '/plugins';

  $info = array();

  $info['ServicesClientConnectionSessionAuth'] = array(
    'name' => 'Session Authentication',
    'description' => 'Provides session authentication against services module',
    'services_version' => array(1, 3),
    'handler' => array(
      'parent' => 'ServicesClientConnectionAuth',
      'class' => 'ServicesClientConnectionSessionAuth',
      'file' => 'ServicesClientConnectionSessionAuth.inc',
      'path' => $path,
    ),
  );

  return $info;
}

/**
 * List of server plugins provided by module
 */
function _services_client_connection_server() {
  $path = drupal_get_path('module', 'services_client_connection') . '/plugins';

  $info = array();

  $info['ServicesClientConnectionXmlrpcServer'] = array(
    'name' => 'XMLRPC server',
    'description' => 'Provides integration for XMLRPC server',
    'services_version' => 3,
    'handler' => array(
      'parent' => 'ServicesClientConnectionServer',
      'class' => 'ServicesClientConnectionXmlrpcServer',
      'file' => 'ServicesClientConnectionXmlrpcServer.inc',
      'path' => $path,
    ),
  );

  $info['ServicesClientConnectionRestServer'] = array(
    'name' => 'REST server',
    'description' => 'Provides integration for REST server',
    'services_version' => 3,
    'handler' => array(
      'parent' => 'ServicesClientConnectionServer',
      'class' => 'ServicesClientConnectionRestServer',
      'file' => 'ServicesClientConnectionRestServer.inc',
      'path' => $path,
    ),
  );

  return $info;
}

/**
 * List of request plugins provided by module
 */
function _services_client_connection_request() {
  $path = drupal_get_path('module', 'services_client_connection') . '/plugins';

  $info = array();

  $info['ServicesClientConnectionCurlRequest'] = array(
    'name' => 'cURL Request',
    'description' => 'Provides communication via cURL',
    'handler' => array(
      'parent' => 'ServicesClientConnectionRequest',
      'class' => 'ServicesClientConnectionCurlRequest',
      'file' => 'ServicesClientConnectionCurlRequest.inc',
      'path' => $path,
    ),
  );

  return $info;
}

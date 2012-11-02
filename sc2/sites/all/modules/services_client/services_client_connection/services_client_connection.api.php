<?php

/**
 * @file
 * Services Client Connection exposed hooks
 */

/**
 * Allows to alter or react on HTTP call to remote site
 *
 * @param ServicesClientConnectionHttpRequest $request
 *   Processed request
 * @return
 *   Hook shoud modify passed objecgt
 */
function hook_services_client_connection_request_alter(ServicesClientConnectionHttpRequest &$request) {

}

/**
 * Allow to alter response from remote site
 *
 * @param ServicesClientConnectionResponse $response
 *   Response created by request plugin
 * @param ServicesClientConnectionHttpRequest $request
 *   Original request
 */
function hook_services_client_connection_response_alter(ServicesClientConnectionResponse &$response, ServicesClientConnectionHttpRequest $request) {
  
}

/**
 * Provide default connections defined in code
 *
 * @return
 *   Array of connection definitions
 */
function hook_services_client_connection_default_connections() {
  
}

/**
 * Connection is saved, module should save data to own table
 *
 * @param $connection
 *   Connection object
 */
function hook_services_client_connection_save(&$connection) {
  
}

/**
 * Add custom module properties to connection object
 *
 * @param $connection
 *   Connection object loaded form DB
 */
function hook_services_client_connection_load(&$connection) {

}


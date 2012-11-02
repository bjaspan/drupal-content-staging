<?php

/**
 * @file
 * Base plugin definitions. All other plugins should be extended from this set of
 * plugins.
 */

/**
 * Base plugin class
 */
class ServicesClientConnectionPlugin {

  /**
   * Connection definition
   */
  protected $connection;

  /**
   * Plugin specific configuration
   *
   * @var array
   */
  protected $config;

  /**
   * Reference to client
   * 
   * @var ServicesClientConnection
   */
  protected $client;

  public function __construct($connection, $config, $client = NULL) {
    // Store configuration and connection definition
    $this->connection = $connection;
    $this->config     = $config;
    $this->client     = $client;
  }

  /**
   * Configuration form options
   */
  public function configForm(&$form, &$form_state) {
    $form['markup'] = array(
      '#type' => 'item',
      '#markup' => t("This plugin doesn't provide any configuration options."),
    );
  }

  public function configFormValidate(&$form, &$form_state) {}

  public function configFormSubmit(&$form, &$form_state) {}

  /**
   * Allows module to alter data in process of request
   *
   * @param ServicesClientConnectionHttpRequest $request
   */
  public function prepareRequest(ServicesClientConnectionHttpRequest &$request) {}

  public function processResponse(ServicesClientConnectionResponse &$response) {}
}

/**
 * Base authentication class
 */
class ServicesClientConnectionAuth extends ServicesClientConnectionPlugin {

  /**
   * Make initial conneciton to client
   */
  public function connect() {}

  /**
   * Login and hold login state on beginning of session
   */
  public function login() {}

  /**
   * Logout client from remote site
   */
  public function logout() {}
}

class ServicesClientConnectionServer extends ServicesClientConnectionPlugin {
  
}

class ServicesClientConnectionRequest extends ServicesClientConnectionPlugin {

  /**
   * Make a call to remote site and retrieve data
   *
   * @param ServicesClientConnectionHttpRequest $request
   */
  public function call(ServicesClientConnectionHttpRequest &$request) {}
}

<?php

/**
 * @file
 * Adapters to communicate with different versions of remote services modules
 */

/**
 * Interface that defines methods to deal with differences between versions
 * of services module on remote servers.
 */
abstract class IRemoteServicesApi {

  /**
   * Connection object
   *
   * @var stdClass
   */
  protected $connection;

  /**
   * URL of remote gateway
   * 
   * @var string
   */
  protected $gateway;

  /**
   * Create adapter for communication with remote api
   */
  public function __construct($connection) {
    // Kepp connection object
    $this->connection = $connection;
    
    // Construct our gateway URL
    $gateway = rtrim(($connection->secured == 'yes' ? 'https://' : 'http://') . $connection->domain, '/');

    // Instantiate the services object with the endpoint and services module version
    $endpoint = '/' . ltrim($connection->endpoint, '/');

    $this->gateway = $gateway . $endpoint;
  }

  /**
   * Parse repsonse string from remote server
   */
  abstract public function parseResponse(&$response);

  public function prepareRequest(ServicesClientCurlRequest &$request) {
    $request->url = $this->getUrl($request->resource, $request->method, $request->data, $request->id);
    $request->data = $this->getData($request->resource, $request->method, $request->data, $request->id);
  }
  
  /**
   * Get URL that should be used in request
   */
  public function getUrl($resource, $method, $data, $id = NULL) {
    return $this->gateway;
  }

  /**
   * Get $data array that should be used in request
   */
  public function getData($resource, $method, $data, $id = NULL) {
    return $data;
  }

  /**
   * Get connection object
   */
  public function getConnection() {
    return $this->connection;
  }
}

class ServicesClientServices3 extends IRemoteServicesApi {

  /**
   * List of standard services methods
   *
   * @var array
   */
  private $methods = array(
    'index', 'create', 'retrieve', 'udpate', 'delete'
  );

  /**
   * Get full remote URL path
   *
   * @param string $path
   * @return string
   */
  private function url($path) {
    return $this->gateway . '/' . $path;
  }

  /**
   * Parse remote response
   *
   * @param $response
   */
  public function parseResponse(&$response) {
    $response->response = json_decode($response->response);
  }

  /**
   * Create request URL
   */
  public function getUrl($resource, $method, $data, $id = NULL) {
    $parts = array($resource);

    // Normal resource methods don't need special URL
    if (in_array($method, $this->methods)) {
      if ($id) {
        $parts[] = $id;
      }
    }
    // Consider other requests as actions and include them in URL
    else {
      $parts[] = $method;
    }

    $path = implode('/', $parts);
    return $this->url($path);
  }
}

class ServicesClientServices1 extends IRemoteServicesApi {

  /**
   * Parse remote response
   *
   * @param $response
   */
  public function parseResponse(&$response) {
    if (isset($response->response) && trim($response->response) != '') {
      $json_obj = @json_decode(@json_encode(simplexml_load_string($response->response, 'SimpleXMLElement', LIBXML_NOCDATA)));
      if (isset($json_obj->response)) {
        $response->response = $json_obj->response;
      }
      elseif ($json_obj->status == 'error') {
        $response->error = $json_obj->message;
        $response->response = NULL;
      }
    }
  }

  public function prepareRequest(ServicesClientCurlRequest &$request) {
    parent::prepareRequest($request);
    if ($request->auth['sessid']) {
      $request->data['sessid'] = $request->auth['sessid'];
    }
  }

  /**
   * Alter data sent to remote services 1 module
   */
  public function getData($resource, $method, $data, $id = NULL) {
    $data['method'] = $this->getLegacyMethod($resource, $method);

    if ($data['method'] == 'node.load') {
      $data['nid'] = $id;
    }

    return $data;
  }

  /**
   * Retrieve legacy method of services 1.x
   *
   * @param $resource
   * @param $method
   * @return array
   */
  private function getLegacyMethod($resource, $method) {
    $mapping = array(
      'node' => array(
        'retrieve' => 'node.load',
        'update' => 'node.save',
        'create' => 'node.save',
        'delete' => 'node.delete',
      ),
      'user' => array(
        'login' => 'user.login',
        'logout' => 'user.logout',
        'retrieve' => 'user.load',
        'create' => 'user.save',
        'update' => 'user.save',
      ),
      'uuid' => array(
        'retrieve' => 'uuid.load',
        'update' => 'uuid.save',
      ),
      'system' => array(
        'connect' => 'system.connect'
      ),
    );

    if (!isset($mapping[$resource][$method])) {
      $message = t('Missing services 1 mapping for resource: !resource, method: !method', array(
        '!resource' => $resource,
        '!method' => $method,
      ));
      throw new ServicesClientMissingAdapterMappingException($message);
    }

    return $mapping[$resource][$method];
  }
}

<?php

define('SERVICES_CLIENT_STATUS_UNCONNECTED', 0);
define('SERVICES_CLIENT_STATUS_LOGGEDIN', 1);

/**
 * Defines an object for working with a remote API, not using Drupal API
 */
class ServicesClientCurl {

  /**
   * Version of services module on remote side
   * 
   * @var IRemoteServicesApi
   */
  public $servicesAdapter;

  /**
   * Write debug output to watchdog
   *
   * @var bool
   */
  public $debug;

  /**
   * Login status
   *
   * @var const
   */
  public $status;

  /**
   * Session name (obtained at login)
   *
   * @var string
   */
  public $session;

  /**
   * Session id obtained at login
   *
   * @var string
   */
  public $sessid;

  /**
   * Constructor
   *
   * @param IRemoteServicesApi $services_adapter
   *   Services version adapter.
   */
  public function __construct($services_adapter) {
    $this->servicesAdapter = $services_adapter;

    $this->status  = SERVICES_CLIENT_STATUS_UNCONNECTED;
    $this->session = '';
    $this->sessid  = '';
  }

  /**
   * Get session cookie. Should be used in HTTP headers as CURLOP_COOKIE
   *
   * @return
   *   Cookie string in format session=sessid
   */
  private function getCookieHeader() {
    return $this->session . '=' . $this->sessid;
  }

  /**
   * Retrieve default CURL options required for every CURL request
   *
   * @param bool $include_auth
   *   Whether to include session cookie
   */
  private function getDefaultCurlOptions($url, $include_auth = FALSE) {
    // Default options for every CURL request
    $ret = array(
      CURLOPT_URL            => $url,
      CURLOPT_FAILONERROR    => TRUE,
      CURLOPT_RETURNTRANSFER => TRUE,
      CURLOPT_TIMEOUT        => 4,
      CURLOPT_HTTPHEADER     => array('Accept: application/json'),
    );

    if ($include_auth) {
      $ret[CURLOPT_COOKIE]   = $this->getCookieHeader();
    }

    return $ret;
  }

  /**
   * Return the standard set of curl options for a POST
   *
   * @param string $url
   *   Requested URL
   * @param mixed $data
   *   Data that are sent to POST request
   * @param bool $include_auth
   *   Whether to include session cookie
   * @return array
   *   CURL Options
   */
  private function getCurlPostOptions($url, $data, $include_auth = FALSE) {
    $ret = $this->getDefaultCurlOptions($url, $include_auth);

    $ret += array(
      CURLOPT_POST       => TRUE,
      CURLOPT_POSTFIELDS => $data
    );

    return $ret;
  }

  /**
   * Return the standard set of curl options for a GET
   *
   * @param string $url
   *   Requested URL
   * @param mixed $data
   *   Data that are sent to POST request
   * @param bool $include_auth
   *   Whether to include session cookie
   * @return array
   *   CURL Options
   */
  private function getCurlGetOptions($url, $data = NULL, $include_auth = FALSE) {
    if ($data) {
      $url = $url . '?' . $data;
    }

    $ret = $this->getDefaultCurlOptions($url, $include_auth);

    $ret += array(
      CURLOPT_BINARYTRANSFER => 1
    );

    return $ret;
  }
  
  /**
   * Return the standard set of curl options for a PUT
   *
   * @param string $url
   *   Requested URL
   * @param mixed $data
   *   Data that are sent to POST request
   * @param bool $include_auth
   *   Whether to include session cookie
   * @return array
   *   CURL Options
   */
  private function getCurlPutOptions($url, $data, $include_auth = FALSE) {
    $ret = $this->getDefaultCurlOptions($url, $include_auth);

    $ret += array(
      CURLOPT_CUSTOMREQUEST => 'PUT',
      CURLOPT_HTTPHEADER => array(
        'Content-Length: ' . strlen($data),
        'Accept: application/json'
      ),
      CURLOPT_POSTFIELDS => $data,
    );

    return $ret;
  }

  /**
   * Return the standard set of curl options for a DELETE
   *
   * @param string $url
   *   Requested URL
   * @param mixed $data
   *   Data that are sent to POST request
   * @param bool $include_auth
   *   Whether to include session cookie
   * @return array
   *   CURL Options
   */
  private function getCurlDeleteOptions($url, $data = NULL, $include_auth = FALSE) {
    $ret = $this->getDefaultCurlOptions($url, $include_auth);

    $ret += array(
      CURLOPT_CUSTOMREQUEST => 'DELETE',
    );

    return $ret;
  }

  /**
   * Return
   * @param type $url
   * @param type $data
   * @param type $include_auth
   */
  private function getCurlOptions($url, $method, $data = NULL, $include_auth = FALSE) {
    switch ($method) {
      case 'POST':
        return $this->getCurlPostOptions($url, $data, $include_auth);
      case 'GET':
        return $this->getCurlGetOptions($url, $data, $include_auth);
      case 'PUT':
        return $this->getCurlPutOptions($url, $data, $include_auth);
      case 'DELETE': 
        return $this->getCurlDeleteOptions($url, NULL, $include_auth);
      default:
        return NULL;
    }
  }

  /**
   * Return false if we're logged in
   * 
   * @return bool
   */
  private function verifyUnconnected() {
    if ($this->status != SERVICES_CLIENT_STATUS_UNCONNECTED) {
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Return false if we're not logged in
   *
   * @return bool
   */
  private function verifyLoggedIn() {
    if ($this->status != SERVICES_CLIENT_STATUS_LOGGEDIN) {
      return FALSE;
    }
    return TRUE;
  }
  
  /**
   * Perform the common logic for performing an HTTP request with cURL
   *
   * @param ServicesClientCurlRequest $request
   *   Request
   * @return stdClass
   *   An object with 'response', 'error' and 'info' fields.
   */
  private function curlHttpRequest(ServicesClientCurlRequest $request, $include_auth = TRUE) {
    // Allow other modules to alter request
    drupal_alter('services_client_request', $request, $this->connection);

    // Build CURL request
    if (is_array($request->data)) {
      $request->data = http_build_query($request->data, '', '&');
    }

    // Create CURL request
    $request->curlOpts = $this->getCurlOptions($request->url, $request->curlMethod, $request->data, $include_auth);

    // Create curl resource
    $ch = curl_init();

    // Get additional request options
    if (!empty($request->curlOpts)) {
      curl_setopt_array($ch, $request->curlOpts);
    }

    // Create new response object
    $ret = new stdClass;
    $ret->response = curl_exec($ch); // execute and get response
    $ret->error    = curl_error($ch);
    $ret->info     = curl_getinfo($ch);

    $this->debug(t('Request: <pre>!request</pre>, Response: <pre>!response</pre>', array(
      '!request' => print_r($request, TRUE),
      '!response' => print_r($ret, TRUE),
    )));

    // Close cURL connection
    curl_close($ch);

    // In case of successful response parse JSON
    if ($ret->info['http_code'] == 200) {
      $this->servicesAdapter->parseResponse($ret);
    }

    return $ret;
  }

  /**
   * Check if client is logged in. If not method throws exception.
   */
  private function requireLogin() {
    if (!$this->verifyLoggedIn()) {
      throw new ServicesClientException(t('You have to be logged in for this operation.'));
    }
  }

  /**
   * Prepare request specific to version of remote API. This method returns
   * two variables.
   *   $request = $this->prepareRequest('user', 'login', $data);
   *
   * @param $resource
   *   Resource type on remote site
   * @param $method
   *   Method that should be executed
   * @param $data
   *   Optional - request data
   * @return ServicesClientCurlRequest
   *   Request
   */
  private function prepareRequest($resource, $method = NULL, $data = NULL, $id = NULL) {
    
    // Create request object
    $request = new ServicesClientCurlRequest();
    $request->data = $data;
    $request->method = $method;
    $request->resource = $resource;
    $request->id = $id;

    if ($this->sessid) {
      $request->auth = array(
        'sessid' => $this->sessid,
        'session' => $this->session
      );
    }

    // Pass object to adapter to prepare version specific request
    $this->servicesAdapter->prepareRequest($request);
    
    return $request;
  }

  /**
   * Login to remote Drupal site using session authentication.
   *
   * @param string $username
   * @param string $password
   * @return bool
   */
  public function login($username, $password) {
    if (!$this->verifyUnconnected()) {
      throw new ServicesClientException(t('Remote API client is already logged in.'));
    }

    // Build GET data request
    $data = array(
      'username' => $username,
      'password' => $password,
    );
    $request = $this->prepareRequest('user', 'login', $data);
    $request->curlMethod = 'POST';
    
    // Call remote API
    $ret = $this->curlHttpRequest($request, FALSE);

    // Check for sucessful response
    if (empty($ret->error) && $ret->info['http_code'] == 200 && $ret->response->sessid) {
      $this->sessid  = $ret->response->sessid;
      $this->session = $ret->response->session_name;
      $this->status = SERVICES_CLIENT_STATUS_LOGGEDIN;
      return TRUE;
    }

    return NULL;
  }
  
  /**
   * End existing session and clear cookie
   */
  public function logout() {
    // Check if client has been logged in.
    if (!$this->verifyLoggedIn()) {
      throw new ServicesClientException(t('Remote client is already loged out.'));
    }

    $request = $this->prepareRequest('user', 'logout');
    $request->curlMethod = 'POST';

    $ret = $this->curlHttpRequest($request);

    if ($ret->info['http_code'] == 200) {
      $this->status = SERVICES_CLIENT_STATUS_UNCONNECTED;
      $this->sessid  = '';
      $this->session = '';
      return TRUE;
    }

    return NULL;
  }

  /**
   * Perform an 'Index' operation on a resource type.
   * 
   * @param string $resource_type
   *   Type of requested resource, i.e. node, user
   * @return array
   *   An array of resource descriptions, or NULL if an error occurs
   */
  public function index($resource_type) {
    $this->requireLogin();
    $request = $this->prepareRequest($resource_type, 'index');
    $ret = $this->curlHttpRequest($request);
    return $ret->response;
  }
  
  /**
   * Create a new resource of the named type given an array of data.
   *
   * @param string $resource_type
   * @param mixed $data
   * @return type 
   */
  public function create($resource_type, $data) {
    $this->requireLogin();
    $request = $this->prepareRequest($resource_type, 'create', $data);
    $request->curlMethod = 'POST';
    $ret = $this->curlHttpRequest($request);
    return $ret->response;
  }

  /**
   * Perform a 'GET' operation on the named resource type and id.
   * 
   * @param string $resource_type
   * @param int $id
   * @param mixed $data
   * @return
   *   Response from remote API
   */
  public function get($resource_type, $id, $data = array()) {
    $this->requireLogin();
    $request = $this->prepareRequest($resource_type, 'retrieve', $data, $id);
    $ret = $this->curlHttpRequest($request);
    return $ret->response;
  }

  /**
   * Update a resource given the resource type and updating array
   *
   * @param string $resource_type
   * @param int $id
   * @param mixed $data
   * @return
   *   Response from remote API
   */
  public function update($resource_type, $id, $data = array()) {
    $this->requireLogin();
    $request = $this->prepareRequest($resource_type, 'update', $data, $id);
    $request->curlMethod = 'PUT';
    $ret = $this->curlHttpRequest($request);
    return $ret->response;
  }

  /**
   * Delete a resource given the resource type and id
   *
   * @param string $resource_type
   * @param int $id
   * @return
   *   Response from remote API
   */
  public function delete($resource_type, $id) {
    $this->requireLogin();
    $request = $this->prepareRequest($resource_type, 'delete', array(), $id);
    $request->curlMethod = 'DELETE';
    $ret = $this->curlHttpRequest($request);
    return $ret->response;
  }

  /**
   * Write debug data per connection settings
   *
   * @param $message
   * @param $data
   */
  private function debug($message, $data = array()) {
    services_client_debug($this->servicesAdapter->getConnection(), $message, $data);
  }
}

/**
 * Represents CURL Request objects
 */
class ServicesClientCurlRequest {

  /**
   * Resource ID
   *
   * @var mixed
   */
  public $id = NULL;

  /**
   * Resource on remote server
   *
   * @var string
   */
  public $resource = NULL;

  /**
   * Remote URL that will be called
   *
   * @var string
   */
  public $url = NULL;

  /**
   * Data that are going to be sent to client
   *
   * @var array
   */
  public $data = NULL;

  /**
   * Remote method
   *
   * @var string
   */
  public $method = NULL;

  /**
   * CURL request options
   *
   * @var array
   */
  public $curlOpts = array();

  /**
   * CURL method
   *
   * @var string
   */
  public $curlMethod = 'GET';

  /**
   * Session information
   *
   * @var array
   */
  public $auth = array();
}

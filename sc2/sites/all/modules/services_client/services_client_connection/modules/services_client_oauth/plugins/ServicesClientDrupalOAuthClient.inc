<?php

/**
 * @file
 * Custom DrupalOAuthRequest implementation
 */

class ServicesClientDrupalOAuthClient extends DrupalOAuthClient {
  /**
   * Gets a request token from the provider.
   *
   * @param string $endpoint
   *   Optional. The endpoint path for the provider.
   *     - If you provide the full URL (e.g. "http://example.com/oauth/request_token"),
   *       then it will be used.
   *     - If you provide only the path (e.g. "oauth/request_token"), it will
   *       be converted into a full URL by prepending the provider_url.
   *     - If you provide nothing it will default to '/oauth/request_token'.
   * @param array $options
   *   An associative array of additional optional options, with the following keys:
   *     - 'params'
   *       An associative array of parameters that should be included in the
   *       request.
   *     - 'realm'
   *       A string to be used as the http authentication realm in the request.
   *     - 'get' (default FALSE)
   *       Whether to use GET as the HTTP-method instead of POST.
   *     - 'callback'
   *       A full URL of where the user should be sent after the request token
   *       has been authorized.
   *       Only used by versions higher than OAUTH_COMMON_VERSION_1.
   *     - 'force_port'
   *       A port number that is forced when creating normalized URL for
   *       calculating signature.
   * @return DrupalOAuthToken
   *  The returned request token.
   */
  public function getRequestToken($endpoint = NULL, $options = array()) {
    if ($this->requestToken) {
      return clone $this->requestToken;
    }

    $options += array(
        'params' => array(),
        'realm' => NULL,
        'get' => FALSE,
        'callback' => NULL,
        'force_port' => NULL,
      );

    if (empty($endpoint)) {
      if (!empty($this->consumer->configuration['request_endpoint'])) {
        $endpoint = $this->consumer->configuration['request_endpoint'];
      }
      else {
        $endpoint = '/oauth/request_token';
      }
    }
    if ($this->version > OAUTH_COMMON_VERSION_1) {
      $options['params']['oauth_callback'] = $options['callback'] ? $options['callback'] : 'oob';
    }

    $response = $this->get($endpoint, array(
        'params' => $options['params'],
        'realm' => $options['realm'],
        'get' => $options['get'],
        'force_port' => $options['force_port'],
      ));

    $params = array();
    parse_str($response, $params);

    if (empty($params['oauth_token']) || empty($params['oauth_token_secret'])) {
      throw new Exception('No valid request token was returned');
    }

    if ($this->version > OAUTH_COMMON_VERSION_1 && empty($params['oauth_callback_confirmed'])) {
      $this->version = OAUTH_COMMON_VERSION_1;
    }

    $this->requestToken = new DrupalOAuthToken($params['oauth_token'], $params['oauth_token_secret'], $this->consumer, array(
      'type'    => OAUTH_COMMON_TOKEN_TYPE_REQUEST,
      'version' => $this->version,
    ));

    return clone $this->requestToken;
  }

  /**
   * Fetches the access token using the request token.
   *
   * @param string $endpoint
   *   Optional. The endpoint path for the provider.
   *     - If you provide the full URL (e.g. "http://example.com/oauth/access_token"),
   *       then it will be used.
   *     - If you provide only the path (e.g. "oauth/access_token"), it will
   *       be converted into a full URL by prepending the provider_url.
   *     - If you provide nothing it will default to '/oauth/access_token'.
   * @param array $options
   *   An associative array of additional optional options, with the following keys:
   *     - 'params'
   *       An associative array of parameters that should be included in the
   *       request.
   *     - 'realm'
   *       A string to be used as the http authentication realm in the request.
   *     - 'get' (default FALSE)
   *       Whether to use GET as the HTTP-method instead of POST.
   *     - 'verifier'
   *       A string containing a verifier for he user from the provider.
   *       Only used by versions higher than OAUTH_COMMON_VERSION_1.
   *     - 'force_port'
   *       A port number that is forced when creating normalized URL for
   *       calculating signature.
   * @return DrupalOAuthToken
   *  The access token.
   */
  public function getAccessToken($endpoint = NULL, $options = array()) {
    if ($this->accessToken) {
      return clone $this->accessToken;
    }

    $options += array(
        'params' => array(),
        'realm' => NULL,
        'get' => FALSE,
        'verifier' => NULL,
        'force_port' => NULL,
      );

    if (empty($endpoint)) {
      if (!empty($this->consumer->configuration['access_endpoint'])) {
        $endpoint = $this->consumer->configuration['access_endpoint'];
      }
      else {
        $endpoint = '/oauth/access_token';
      }
    }
    if ($this->version > OAUTH_COMMON_VERSION_1 && $options['verifier'] !== NULL) {
      $options['params']['oauth_verifier'] = $options['verifier'];
    }

    $response = $this->get($endpoint, array(
        'token' => TRUE,
        'params' => $options['params'],
        'realm' => $options['realm'],
        'get' => $options['get'],
        'force_port' => $options['force_port'],
      ));

    $params = array();
    parse_str($response, $params);

    if (empty($params['oauth_token']) || empty($params['oauth_token_secret'])) {
      throw new Exception('No valid access token was returned');
    }

    // Check if we've has recieved this token previously and if so use the old one
    //TODO: Is this safe!? What if eg. multiple users are getting the same access token from the provider?
    $this->accessToken = DrupalOAuthToken::loadByKey($params['oauth_token'], $this->consumer);
    //TODO: Can a secret change even though the token doesn't? If so it needs to be changed.
    if (!$this->accessToken) {
      $this->accessToken = new DrupalOAuthToken($params['oauth_token'], $params['oauth_token_secret'], $this->consumer, array(
        'type' => OAUTH_COMMON_TOKEN_TYPE_ACCESS,
      ));
    }

    return clone $this->accessToken;
  }

  /**
   * Make an OAuth request.
   *
   * @param string $path
   *   The path being requested.
   *     - If you provide the full URL (e.g. "http://example.com/oauth/request_token"),
   *       then it will be used.
   *     - If you provide only the path (e.g. "oauth/request_token"), it will
   *       be converted into a full URL by prepending the provider_url.
   * @param array $options
   *   An associative array of additional options, with the following keys:
   *     - 'token' (default FALSE)
   *       Whether a token should be used or not.
   *     - 'params'
   *       An associative array of parameters that should be included in the
   *       request.
   *     - 'realm'
   *       A string to be used as the http authentication realm in the request.
   *     - 'get' (default FALSE)
   *       Whether to use GET as the HTTP-method instead of POST.
   *     - 'force_port'
   *       A port number that is forced when creating normalized URL for
   *       calculating signature.
   * @return string
   *   a string containing the response body.
   */
  protected function get($path, $options = array()) {
    $options += array(
        'token' => FALSE,
        'params' => array(),
        'realm' => NULL,
        'get' => FALSE,
        'force_port' => NULL,
      );

    if (empty($options['realm']) && !empty($this->consumer->configuration['authentication_realm'])) {
      $options['realm'] = $this->consumer->configuration['authentication_realm'];
    }

    $token = $options['token'] ? $this->requestToken : NULL;
    $path = $this->getAbsolutePath($path);

    $req = ServicesClientOAuthRequest::from_consumer_and_token($this->consumer, $token,
      $options['get'] ? 'GET' : 'POST', $path, $options['params']);
    if ($options['force_port']) {
      $req->force_port = $options['force_port'];
    }
    $req->sign_request($this->signatureMethod, $this->consumer, $token);

    $url = $req->get_normalized_http_url(FALSE);
    $params = array();
    foreach ($req->get_parameters() as $param_key => $param_value) {
      if (substr($param_key, 0, 5) != 'oauth') {
        $params[$param_key] = $param_value;
      }
    }
    if (!empty($params)) {
      $url .= '?' . http_build_query($params);
    }

    $headers = array(
      'Accept: application/x-www-form-urlencoded',
      $req->to_header($options['realm']),
    );

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    if (!$options['get']) {
      curl_setopt($ch, CURLOPT_POST, 1);
      curl_setopt($ch, CURLOPT_POSTFIELDS, '');
    }
    $oauth_version = _oauth_common_version();
    curl_setopt($ch, CURLOPT_USERAGENT, 'Drupal/' . VERSION . ' OAuth/' . $oauth_version);
    curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($ch, CURLOPT_HEADER, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);

    $response = curl_exec($ch);
    $error = curl_error($ch);
    curl_close($ch);

    if ($error) {
      throw new Exception($error);
    }

    $result = $this->interpretResponse($response);
    if ($result->responseCode != 200) {
      throw new Exception('Failed to fetch data from url "' . $path . '" (HTTP response code ' . $result->responseCode . ' ' . $result->responseMessage . '): ' . $result->body, $result->responseCode);
    }

    return $result->body;
  }
}
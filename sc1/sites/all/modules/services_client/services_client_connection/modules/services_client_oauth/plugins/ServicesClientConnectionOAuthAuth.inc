<?php

/**
 * OAuth authentication support
 */
class ServicesClientConnectionOAuthAuth extends ServicesClientConnectionAuth {

  /**
   * Last request
   *
   * @var ServicesClientOAuthRequest
   */
  protected $request;

  public function configForm(&$form, &$form_state) {
    if (!empty($this->config)) {
      $form['consumer_key'] = array(
        '#title' => t('Consumer Key'),
        '#type' => 'textfield',
        '#default_value' => $this->config['consumer_key']
      );

      $form['consumer_secret'] = array(
        '#title' => t('Consumer Secret'),
        '#type' => 'textfield',
        '#default_value' => $this->config['consumer_secret']
      );

      $form['access_key'] = array(
        '#title' => t('Access Key'),
        '#type' => 'textfield',
        '#default_value' => $this->config['access_key']
      );

      $form['access_secret'] = array(
        '#title' => t('Access Secret'),
        '#type' => 'textfield',
        '#default_value' => $this->config['access_secret']
      );

      $form['force_port'] = array(
        '#title' => t('Force Port'),
        '#type' => 'textfield',
        '#default_value' => isset($this->config['force_port']) ? $this->config['force_port'] : '',
        '#description' => t('If required force port in URL which is used for calculating signature. I.e. 80 for https connection.')
      );

      $form['add_token'] = array(
        '#type' => 'item',
        '#markup' => t('Re-authorize !connection', array(
          '!connection' => l(t('connection'), 'services_client_oauth/request', array('query' => array(
            'connection_name' => $this->connection->name,
          ))),
        ))
      );
    }
    else {
      $form['add_token'] = array(
        '#type' => 'item',
        '#markup' => t('Authorize new !connection', array(
          '!connection' => l(t('connection'), 'services_client_oauth/request', array('query' => array(
            'connection_name' => $this->connection->name,
          ))),
        ))
      );
    }
  }

  /**
   * Implements configFormSubmit().
   */
  public function configFormSubmit(&$form, &$form_state) {
    parent::configFormSubmit($form, $form_state);

    $form_state['config']['consumer_key'] = $form_state['values']['consumer_key'];
    $form_state['config']['consumer_secret'] = $form_state['values']['consumer_secret'];
    $form_state['config']['access_key'] = $form_state['values']['access_key'];
    $form_state['config']['access_secret'] = $form_state['values']['access_secret'];
    $form_state['config']['force_port'] = $form_state['values']['force_port'];
  }

  /**
   * Implements prepareRequest().
   *
   * @param ServicesClientConnectionHttpRequest $request
   */
  public function prepareRequest(ServicesClientConnectionHttpRequest &$request) {
    parent::prepareRequest($request);

    if ($this->isAuthorized()) {
      // By default authenticate request before its sent to request plugin
      $this->sign($request);
    }
  }

  /**
   * Check if current plugin configuration has been authorized
   */
  protected function isAuthorized() {
    $keys = array('consumer_key', 'consumer_secret', 'access_key', 'access_secret');
    foreach ($keys as $key) {
      if (!isset($this->config[$key]) || empty($this->config[$key])) {
        return FALSE;
      }
    }

    return TRUE;
  }

  /**
   * Autneticate request
   * 
   * @param ServicesClientConnectionHttpRequest $request
   */
  protected function sign(ServicesClientConnectionHttpRequest &$request) {
    // Build params that should be signed
    $params = $this->getRequestParams($request);

    $consumer = new DrupalOAuthConsumer($this->config['consumer_key'], $this->config['consumer_secret']);
    $token = new DrupalOAuthToken($this->config['access_key'], $this->config['access_secret'], $consumer);
    // Create request
    $req = ServicesClientOAuthRequest::from_consumer_and_token($consumer, $token, $request->http_method, $request->url, $params);
    // For special use cases we may need to force port to URL when calculating singature
    if (isset($this->config['force_port']) && !empty($this->config['force_port'])) {
      $req->force_port = $this->config['force_port'];
    }
    $signature_method = DrupalOAuthClient::signatureMethod();

    $req->sign_request($signature_method, $consumer, $token);

    $header = explode(':', $req->to_header());
    $request->http_headers[$header[0]] = trim($header[1]);

    // Store latest OAuthRequest
    $this->request = $req;
  }

  /**
   * Determine which params should be inlcuded in signing
   * 
   * @param ServicesClientConnectionHttpRequest $request
   */
  protected function getRequestParams(ServicesClientConnectionHttpRequest &$request) {
    $params = NULL;

    // If GET request take any $req->data params
    if ($request->http_method == 'GET' && is_array($request->data) && !empty($request->data)) {
      $params = $request->data;
    }

    // Try to guess whether POST request contains data encoded by http_build_query
    // which could be assigned to signature on remote site.
    if (
      $request->http_method == 'POST' &&
      is_array($request->data_raw) &&
      !empty($request->data_raw) &&
      isset($request->http_headers['Content-Type']) &&
      $request->http_headers['Content-Type'] == 'application/x-www-form-urlencoded'
    ) {
      // If post URL is comming with Query
      $data = $request->data;
      if (!empty($request->query)) {
        if ($data) {
          $data .= '&';
        }
        $data .= http_build_query($request->query, NULL, '&');
      }

      $params = OAuthUtil::parse_parameters($data);
      if (count($params) == 1) {
        $params = array_filter($params);
      }
    }

    // Flatten array keys
    if (is_array($params) && !empty($params)) {
      $params = $this->flattenParamKeys($params);
    }

    return $params;
  }

  /**
   * Flatten array keys as they would be sent to in HTTP query.
   *
   * array('body' => array('und' => array(0 => array('value' => 'val'))))
   *
   * will get converted to:
   *
   * array('body[und][0][value]' => 'val');
   *
   *
   * @param array $params
   *   HTTP query params
   * @return type
   *   Array of params with flattened keys structure
   */
  protected function flattenParamKeys($params) {
    $string = http_build_query($params, NULL, '&');
    $rows = explode('&', $string);
    $out = array();
    foreach ($rows as $row) {
      list($key, $val) = explode('=', $row);
      $out[urldecode($key)] = urldecode($val);
    }
    return $out;
  }
}

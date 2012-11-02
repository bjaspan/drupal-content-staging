<?php

/**
 * @file
 * Make requests via cURL
 */

class ServicesClientConnectionCurlRequest extends ServicesClientConnectionRequest {

  /**
   * Configuration form allow to set request timeouts
   */
  public function configForm(&$form, &$form_state) {
    $form['request_timeout'] = array(
      '#type' => 'textfield',
      '#title' => t('Request timeout'),
      '#default_value' => isset($this->config['request_timeout']) ? $this->config['request_timeout'] : 5,
      '#description' => t('Enter request timeout after which it will be terminated.'),
    );

    $form['ssl_verifypeer_skip'] = array(
      '#type' => 'checkbox',
      '#title' => t('Skip SSL Cert verification'),
      '#default_value' => isset($this->config['ssl_verifypeer_skip']) ? $this->config['ssl_verifypeer_skip'] : FALSE,
      '#description' => t("Don't verify remote certificate."),
    );
  }

  /**
   * Save required values
   */
  public function configFormSubmit(&$form, &$form_state) {
    parent::configFormSubmit($form, $form_state);

    $form_state['config']['request_timeout'] = $form_state['values']['request_timeout'];
    $form_state['config']['ssl_verifypeer_skip'] = $form_state['values']['ssl_verifypeer_skip'];
  }

  /**
   * Retrieve default CURL options required for every CURL request
   *
   * @param ServicesClientConnectionHttpRequest $request
   *   Request data that should be processed
   * @return array
   *   cURL opts array
   */
  private function getDefaultCurlOptions(&$request) {
    // Add params to URL
    if (!empty($request->query)) {
      $request->url .= strpos($request->url, '?') === FALSE ? '?' : '&';
      $request->url .= http_build_query($request->query, NULL, '&');
    }

    // Default options for every CURL request
    $ret = array(
      CURLOPT_URL            => $request->url,
      CURLOPT_RETURNTRANSFER => TRUE,
      CURLOPT_TIMEOUT        => isset($this->config['request_timeout']) ? $this->config['request_timeout'] : 5,
      CURLOPT_HTTPHEADER     => array(),
      CURLOPT_HEADER         => TRUE,
      CURLINFO_HEADER_OUT    => TRUE,
    );

    if ($request->data && empty($request->http_headers['Content-Length'])) {
      $ret[CURLOPT_HTTPHEADER][] = 'Content-Length: ' . strlen($request->data);
    }

    foreach ($request->http_headers as $name => $value) {
      if (is_string($name)) {
        $ret[CURLOPT_HTTPHEADER][] = $name . ': ' . $value;
      }
      else {
        $ret[CURLOPT_HTTPHEADER][] = $value;
      }
    }

    // Optionally we can skip SSL verification
    if (isset($this->config['ssl_verifypeer_skip']) && $this->config['ssl_verifypeer_skip']) {
      $ret[CURLOPT_SSL_VERIFYPEER] = FALSE;
    }

    return $ret;
  }

  /**
   * Return the standard set of curl options for a POST
   *
   * @param ServicesClientConnectionHttpRequest $request
   *   Request data that should be processed
   * @return array
   *   cURL opts array
   */
  private function getCurlPostOptions(&$request) {
    $ret = $this->getDefaultCurlOptions($request);

    $ret += array(
      CURLOPT_POST       => TRUE,
      CURLOPT_POSTFIELDS => $request->data
    );

    return $ret;
  }

  /**
   * Return the standard set of curl options for a GET
   *
   * @param ServicesClientConnectionHttpRequest $request
   *   Request data that should be processed
   * @return array
   *   cURL opts array
   */
  private function getCurlGetOptions(&$request) {

    // URL have changed to authenticate URL
    if (is_array($request->data_raw) && !empty($request->data_raw)) {
      if (strpos($request->url, '?') === FALSE) {
        $request->url .= '?' . http_build_query($request->data_raw, NULL, '&');
      }
      else {
        $request->url .= '&' . http_build_query($request->data_raw, NULL, '&');
      }
      $request->data = '';

      // Set corrent request content type
      $request->http_headers['Content-Type'] = 'application/x-www-form-urlencoded';
      // We're passing params in URL, don't indicate any content length of http request
      $request->http_headers['Content-Length'] = 0;
    }

    $ret = $this->getDefaultCurlOptions($request);

    $ret += array(
      CURLOPT_BINARYTRANSFER => 1
    );

    return $ret;
  }

  /**
   * Return the standard set of curl options for a PUT
   *
   * @param ServicesClientConnectionHttpRequest $request
   *   Request data that should be processed
   * @return array
   *   cURL opts array
   */
  private function getCurlPutOptions(&$request) {
    $ret = $this->getDefaultCurlOptions($request);

    $ret += array(
      CURLOPT_CUSTOMREQUEST => 'PUT',
      CURLOPT_POSTFIELDS => $request->data,
    );

    return $ret;
  }

  /**
   * Return the standard set of curl options for a DELETE
   *
   * @param ServicesClientConnectionHttpRequest $request
   *   Request data that should be processed
   * @return array
   *   cURL opts array
   */
  private function getCurlDeleteOptions(&$request) {
    $ret = $this->getDefaultCurlOptions($request);

    $ret += array(
      CURLOPT_CUSTOMREQUEST => 'DELETE',
    );

    return $ret;
  }

  /**
   * Returns cURL opts
   *
   * @param ServicesClientConnectionHttpRequest $request
   *   Request data that should be processed
   * @return array
   *   cURL opts array
   */
  private function getCurlOptions($request) {
    switch ($request->http_method) {
      case 'POST':
        return $this->getCurlPostOptions($request);
      case 'GET':
        return $this->getCurlGetOptions($request);
      case 'PUT':
        return $this->getCurlPutOptions($request);
      case 'DELETE':
        return $this->getCurlDeleteOptions($request);
      default:
        return NULL;
    }
  }

  /**
   * Process request and call remote API
   * 
   * @param ServicesClientConnectionHttpRequest $request 
   */
  public function call(ServicesClientConnectionHttpRequest &$request) {
    parent::call($request);

    $opts = $this->getCurlOptions($request);

    if ($request->cookie) {
      if (isset($opts['CURLOPT_COOKIE'])) {
        $opts[CURLOPT_COOKIE] .= '; ' . implode('; ', $request->cookie);
      }
      else {
        $opts[CURLOPT_COOKIE] = implode('; ', $request->cookie);
      }
    }

    $ch = curl_init();
    if ($opts) {
      curl_setopt_array($ch, $opts);
    }

    $ret = new ServicesClientConnectionResponse();
    $ret->raw_response  = curl_exec($ch); // execute and get response
    if (!empty($ret->raw_response)) {
      $ret->original_response = $ret->raw_response;
      $pos = strrpos($ret->raw_response, "\r\n\r\n");
      if ($pos !== FALSE) {
        $ret->raw_headers = trim(substr($ret->raw_response, 0, $pos));
        $ret->raw_response = trim(substr($ret->raw_response, $pos + 4));
      }
    }
    $ret->error_code    = curl_errno($ch);
    $ret->error_message = curl_error($ch);
    $ret->info          = curl_getinfo($ch);
    $ret->response_code = $ret->info['http_code'];

    // Try to process HTTP headers
    if (isset($ret->raw_headers)) {
      $ret->response_headers = explode("\r\n", $ret->raw_headers);
    }

    // Services are usually returning specific error message after
    // HTTP code in format:
    //   HTTP/1.0 404 Not found: Site not found on master
    // Try to parse string after :
    if (!empty($ret->response_headers) && !empty($ret->response_code)) {
      $http_response = $ret->response_headers[0];
      if (preg_match('~^[^:]+:(.+)$~i', $http_response, $matches)) {
        $ret->services_error = trim($matches[1]);
      }
    }

    curl_close($ch);

    return $ret;
  }
}
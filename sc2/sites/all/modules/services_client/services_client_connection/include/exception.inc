<?php

/**
 * @file
 * Custom exception definitions
 */

class ServicesClientConnectionException extends Exception {

  /**
   * Save exception to log
   *
   * @param $severity
   *   Watchodg severity default set to ERROR
   */
  public function log($severity = WATCHDOG_ERROR) {
    watchodg('scc', check_plain($this->getMessage()), array(), $severity);
  }
}

/**
 * Common parent class for all services client module exceptions
 */
class ServicesClientConnectionResponseException extends Exception {

  /**
   * Whole response that represents failed request
   *
   * @var ServicesClientConnectionResponse
   */
  public $response;

  /**
   * Request that has been made to server
   *
   * @var ServicesClientConnectionHttpRequest
   */
  public $request;

  public function __construct($response, $request, $message = NULL, $code = NULL) {
    // Save request and response source
    $this->response = $response;
    $this->request = $request;
    $message = empty($message) ? $this->response->error_message : $message;
    parent::__construct($message, $code);
  }

  /**
   * Dump request and response to watchdog
   * 
   * @param $severity
   *   Watchodg severity default set to ERROR
   */
  public function log($severity = WATCHDOG_ERROR) {
    watchdog('scc', 'Request: <pre>@request</pre> Error response: <pre>@response</pre>', array(
      '@request' => print_r($this->request, TRUE),
      '@response' => print_r($this->response, TRUE),
    ), $severity);
  }
}

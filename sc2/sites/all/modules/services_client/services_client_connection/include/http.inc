<?php


/**
 * HTTP request definition
 */
class ServicesClientConnectionHttpRequest {

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
   * Optionally action
   *
   * @var string
   */
  public $action = NULL;

  /**
   * Optionally relation type
   *
   * @var string
   */
  public $relation = NULL;

  /**
   * Query data
   *
   * @var array
   */
  public $query = array();

  /**
   * Remote URL that will be called
   *
   * @var string
   */
  public $url = '';

  /**
   * Data that are going to be sent to client
   *
   * @var array
   */
  public $data = array();

  /**
   * Raw PHP data that are beiing sent to server
   * 
   * @var array
   */
  public $data_raw = array();

  /**
   * Define which HTTP method should be used
   *
   * @var string
   */
  public $http_method = 'GET';

  /**
   * Custom HTTP headers
   *
   * @var array
   */
  public $http_headers = array();

  /**
   * Cookies sent to remote server
   *
   * @var array
   */
  public $cookie = array();
}

class ServicesClientConnectionResponse {

  /**
   * Raw response retrieved from client
   */
  public $raw_response;

  /**
   * Response HTTP code
   */
  public $response_code;

  /**
   * Possible error code
   */
  public $error_code;

  /**
   * Error message
   */
  public $error_message;

  /**
   * Additional data from Request class
   */
  public $info;

  /**
   * Parsed data
   */
  public $data;
}

/**
 * Helper HTTP class
 */
class ServicesClientConnectionHttp {

  /**
   * List of HTTP code statuses with texts
   *
   * @var array
   */
  public static $http_codes = array(
    // [Informational 1xx]
    100 => 'Continue',
    101 => 'Switching Protocols',
    // [Successful 2xx]
    200 => 'OK',
    201 => 'Created',
    202 => 'Accepted',
    203 => 'Non-Authoritative Information',
    204 => 'No Content',
    205 => 'Reset Content',
    206 => 'Partial Content',
    // [Redirection 3xx]
    300 => 'Multiple Choices',
    301 => 'Moved Permanently',
    302 => 'Found',
    303 => 'See Other',
    304 => 'Not Modified',
    305 => 'Use Proxy',
    306 => '(Unused)',
    307 => 'Temporary Redirect',
    // [Client Error 4xx]
    400 => 'Bad Request',
    401 => 'Unauthorized',
    402 => 'Payment Required',
    403 => 'Forbidden',
    404 => 'Not Found',
    405 => 'Method Not Allowed',
    406 => 'Not Acceptable',
    407 => 'Proxy Authentication Required',
    408 => 'Request Timeout',
    409 => 'Conflict',
    410 => 'Gone',
    411 => 'Length Required',
    412 => 'Precondition Failed',
    413 => 'Request Entity Too Large',
    414 => 'Request-URI Too Long',
    415 => 'Unsupported Media Type',
    416 => 'Requested Range Not Satisfiable',
    417 => 'Expectation Failed',
    // [Server Error 5xx]
    500 => 'Internal Server Error',
    501 => 'Not Implemented',
    502 => 'Bad Gateway',
    503 => 'Service Unavailable',
    504 => 'Gateway Timeout',
    505 => 'HTTP Version Not Supported'
  );

  /**
   * Check if error code represents error
   *
   * @param bool
   */
  public static function isError($code) {
    return ($code >= 400 && $code < 600);
  }

  public static function getHttpMessage($code) {
    return isset(self::$http_codes[$code]) ? self::$http_codes[$code] : FALSE;
  }
}

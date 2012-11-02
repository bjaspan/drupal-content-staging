<?php

/**
 * @file
 * 
 */

class ServicesClientConnectionRestServer extends ServicesClientConnectionServer {

  /**
   * Response parser
   *
   * @var ServicesClientConnectionRestServerParser
   */
  protected $parser;

  /**
   * Request formatter
   *
   * @var ServicesClientConnectionRestServerFormatter
   */
  protected $formatter;

  /**
   * Specificy REST actions
   */
  protected $rest_actions = array(
    'retrieve' => 'GET',
    'update'   => 'PUT',
    'create'   => 'POST',
    'delete'   => 'DELETE'
  );

  /**
   * Constructor
   */
  public function __construct($connection, $config, $client = NULL) {
    parent::__construct($connection, $config, $client);

    // Init parser
    $parser = isset($this->config['response_parser']) ? $this->config['response_parser'] : NULL;
    $this->parser = new ServicesClientConnectionRestServerParser($parser);

    // Init formatter
    $formatter = isset($this->config['request_formatter']) ? $this->config['request_formatter'] : NULL;
    $this->formatter = new ServicesClientConnectionRestServerFormatter($formatter);
  }

  /**
   * Implements configForm().
   */
  public function configForm(&$form, &$form_state) {
    $form['request_formatter'] = array(
      '#type' => 'select',
      '#title' => t('Request formatter'),
      '#options' => ServicesClientConnectionRestServerFormatter::getFormatters(),
      '#default_value' => isset($this->config['request_formatter']) ? $this->config['request_formatter'] : '',
    );

    $form['response_parser'] = array(
      '#type' => 'select',
      '#title' => t('Response parser'),
      '#options' => ServicesClientConnectionRestServerParser::getParsers(),
      '#default_value' => isset($this->config['response_parser']) ? $this->config['response_parser'] : '',
    );
  }

  /**
   * Implements configFormSubmit().
   */
  public function configFormSubmit(&$form, &$form_state) {
    $form_state['config'] = array(
      'request_formatter' => $form_state['values']['request_formatter'],
      'response_parser' => $form_state['values']['response_parser'],
    );
  }

  /**
   * Implements prepareRequest().
   */
  public function prepareRequest(ServicesClientConnectionHttpRequest &$request) {
    parent::prepareRequest($request);

    // Normalize endpoint URL
    $endpoint = rtrim($this->connection->endpoint, '/');

    // No special changes to URL
    $url_parts = array($endpoint, $request->resource);

    // Add resource ID if present
    if ($request->id) {
      $url_parts[] = $request->id;
    }

    // For special REST actions - CRUD don't add action to URL and
    // add HTTP method.
    if (in_array($request->action, array_keys($this->rest_actions))) {
      $request->http_method = $this->rest_actions[$request->action];
    }
    // For relationships
    elseif (!empty($request->relation)) {
      $url_parts[] = $request->relation;
      $request->http_method = 'GET';
    }
    // For other actions / targetActions add action to URL and force POST HTTP
    // method.
    elseif (!empty($request->action)) {
      $url_parts[] = $request->action;
      $request->http_method = 'POST';
    }
    elseif (empty($request->http_method)) {
      $request->http_method = 'POST';
    }

    // Build URL
    $request->url = implode('/', $url_parts);

    // Allow parser to prepare request
    $this->parser->prepareRequest($request);

    // Format request
    $this->formatter->format($request);
  }

  /**
   * Implements processResponse().
   */
  public function processResponse(ServicesClientConnectionResponse &$response) {
    parent::processResponse($response);

    // Try to get error code from response
    if (ServicesClientConnectionHttp::isError($response->response_code)) {
      $response->error_code = $response->response_code;
      $response->error_message = ServicesClientConnectionHttp::getHttpMessage($response->response_code);
    }
    else {
      // Process response and parse data
      $response->data = $this->parser->parse($response->raw_response);
    }
  }
}

class ServicesClientConnectionRestServerFormatter {

  /**
   * List of available request formatters
   */
  protected static $request_formatters = array(
    'php' => array(
      'title' => 'PHP Serialized',
      'method' => 'formatSerialize',
      'content_type' => 'application/vnd.php.serialized',
    ),
    'json' => array(
      'title' => 'JSON',
      'method' => 'formatJson',
      'content_type' => 'application/json',
    ),
    'urlencode' => array(
      'title' => 'URL Encode',
      'method' => 'formatUrlEncode',
      'content_type' => 'application/x-www-form-urlencoded',
    ),
    'formdata'  => array(
      'title' => 'Multipart Data',
      'method' => 'formatMultipart',
      'content_type' => 'multipart/form-data',
    ),
  );

  /**
   * Get list of available formatters
   */
  public static function getFormatters() {
    $output = array();

    foreach (self::$request_formatters as $id => $info) {
      $output[$id] = $info['title'];
    }

    return $output;
  }

  /**
   * Holds formatter type
   */
  protected $formatter;

  /**
   * Create new instance of formatter
   *
   * @param $formatter
   */
  public function __construct($formatter) {
    $this->formatter = $formatter;
  }

  /**
   * Get info about current formatter
   *
   * @param string $property
   *   Optionally property can be specified.
   */
  protected function getFormatterInfo($property = NULL) {
    // Get formatter info
    $formatter = isset(self::$request_formatters[$this->formatter]) ?
      self::$request_formatters[$this->formatter] :
      NULL;

    // If defined property try to retrieve property from formatter info
    if (!empty($property) && !empty($formatter)) {
      return isset($formatter[$property]) ? $formatter[$property] : NULL;
    }

    return $formatter;
  }

  /**
   * Format data for remote site.
   *
   * @param array $data
   * @param ServicesClientConnectionHttpRequest $request
   */
  public function format(&$request) {
    $request->data_raw = $request->data;
    if ($request->http_method != 'GET') {
      // Format data
      if (!empty($request->data) && ($method = $this->getFormatterInfo('method'))) {
        $request->data = call_user_func_array(array($this, $method), array($request->data, $request));
      }
      elseif (empty($request->data)) {
        $request->data = '';
      }

      // Add header Content-Type
      if (($content_type = $this->getFormatterInfo('content_type'))) {
        $request->http_headers['Content-Type'] = $content_type;
      }
    }
  }

  /**
   * Serialize data
   */
  protected function formatSerialize($data) {
    return serialize($data);
  }

  /**
   * JSON encode
   */
  protected function formatJson($data) {
    return json_encode($data);
  }

  /**
   * URL Encode
   */
  protected function formatUrlEncode($data) {
    return http_build_query($data, NULL, '&');
  }

  /**
   * Prepare request for multipart data
   */
  protected function formatMultipart($data) {
    return $data;
  }
}

class ServicesClientConnectionRestServerParser {

  /**
   * Definition of supported parsers
   */
  private static $parsers = array(
    'json' => array(
      'title' => 'JSON',
      'accept' => 'application/json',
      'method' => 'parseJson',
    ),
    'php' => array(
      'title' => 'PHP serialized',
      'accept' => 'application/vnd.php.serialized',
      'method' => 'parseSerialized',
    ),
    'xml' => array(
      'title' => 'XML',
      'accept' => 'application/xml',
      'method' => 'parseXml',
    ),
  );

  /**
   * Get list of supported parsers.
   *
   * @return type
   */
  public static function getParsers() {
    $output = array();

    foreach (self::$parsers as $id => $info) {
      $output[$id] = $info['title'];
    }

    return $output;
  }

  /**
   * Parser type
   */
  private $parser;

  /**
   * Create a new reponse parser
   *
   * @param $parser
   */
  public function __construct($parser) {
    $this->parser = $parser;
  }

  /**
   * Parse retrieved data
   *
   * @param string $data
   */
  public function parse($data) {
    if (($method = $this->getParserInfo('method'))) {
      return call_user_func(array($this, $method), $data);
    }
  }

  /**
   * Prepare request
   *
   * @param ServicesClientConnectionHttpRequest $request
   */
  public function prepareRequest(&$request) {
    if (($accept = $this->getParserInfo('accept'))) {
      $request->http_headers['Accept'] = $accept;
    }
  }

  /**
   * Parse JSON response
   */
  protected function parseJson($data) {
    $parsed = json_decode($data, TRUE);
    // Return array as result of services
    if (is_object($parsed)) {
      $parsed = (array) $parsed;
    }
    return $parsed;
  }

  /**
   * Parse serialized PHP
   */
  protected function parseSerialized($data) {
    return (array) unserialize($data);
  }

  /**
   * Parse XML
   */
  protected function parseXml($data) {
    return (array) simplexml_load_string($data);
  }

  /**
   * Get info about current formatter
   *
   * @param string $property
   *   Optionally property can be specified.
   */
  protected function getParserInfo($property = NULL) {
    // Get formatter info
    $parser = isset(self::$parsers[$this->parser]) ?
      self::$parsers[$this->parser] :
      NULL;

    // If defined property try to retrieve property from formatter info
    if (!empty($property) && !empty($parser)) {
      return isset($parser[$property]) ? $parser[$property] : NULL;
    }

    return $parser;
  }
}

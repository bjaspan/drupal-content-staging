<?php

/**
 * @file
 * XMLRPC server
 */

class ServicesClientConnectionXmlrpcServer extends ServicesClientConnectionServer {

  /**
   * Include Drupal core XML-RPC library
   */
  private function includeXmlrpcLibrary() {
    require_once DRUPAL_ROOT . '/includes/xmlrpc.inc';
  }

  public function prepareRequest(ServicesClientConnectionHttpRequest &$request) {
    parent::prepareRequest($request);

    $this->includeXmlrpcLibrary();

    // No special changes to URL
    $request->url = $this->connection->endpoint;

    // All xml-rpc calls are post
    $request->http_method = 'POST';

    $request->http_headers['Content-Type'] = 'text/xml';

    $method = $request->resource . '.' . $request->action;

    if ($request->id) {
      $data = $request->data;
      $request->data = array('id' => $request->id);
      if (!empty($data)) {
        $request->data += array($data);
      }
    }

    $xml_request = xmlrpc_request($method, $request->data);

    $request->data = $xml_request->xml;
  }

  public function processResponse(ServicesClientConnectionResponse &$response) {
    parent::processResponse($response);
    // Add library
    $this->includeXmlrpcLibrary();
    // Try to retrieve error message
    $message = xmlrpc_message($response->raw_response);

    // Try to get error code from response
    if (ServicesClientConnectionHttp::isError($response->response_code)) {
      $response->error_code = $response->response_code;
      $response->error_message = ServicesClientConnectionHttp::getHttpMessage($response->response_code);
    }
    // Now parse what we've got back
    elseif (!empty($message) && !xmlrpc_message_parse($message)) {
      $response->error_code = -32700;
      $response->error_message = t('Parse error. Not well formed');
    }
    // Is the message a fault?
    elseif ($message->messagetype == 'fault') {
      $response->error_code = $message->fault_code;
      $response->error_message = $message->fault_string;
    }
    else {
      $response->data = $message->params[0];
    }
  }
}

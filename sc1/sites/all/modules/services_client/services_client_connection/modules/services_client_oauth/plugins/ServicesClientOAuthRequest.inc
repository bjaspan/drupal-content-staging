<?php

/**
 * @file
 * 
 */

class ServicesClientOAuthRequest extends OAuthRequest {
  public $force_port = NULL;

  /**
   * pretty much a helper function to set up the request
   */
  public static function from_consumer_and_token($consumer, $token, $http_method, $http_url, $parameters=NULL) {
    $parameters = ($parameters) ?  $parameters : array();
    $defaults = array("oauth_version" => OAuthRequest::$version,
                      "oauth_nonce" => ServicesClientOAuthRequest::generate_nonce(),
                      "oauth_timestamp" => ServicesClientOAuthRequest::generate_timestamp(),
                      "oauth_consumer_key" => $consumer->key);
    if ($token)
      $defaults['oauth_token'] = $token->key;

    $parameters = array_merge($defaults, $parameters);

    return new ServicesClientOAuthRequest($http_method, $http_url, $parameters);
  }

  /**
   * util function: current nonce
   */
  private static function generate_nonce() {
    $mt = microtime();
    $rand = mt_rand();

    return md5($mt . $rand); // md5s look nicer than numbers
  }

  /**
   * util function: current timestamp
   */
  private static function generate_timestamp() {
    return time();
  }

  /**
   * parses the url and rebuilds it to be
   * scheme://host/path
   */
  public function get_normalized_http_url($use_force_port = TRUE) {
    $parts = parse_url($this->http_url);

    $scheme = (isset($parts['scheme'])) ? $parts['scheme'] : 'http';
    $port = (isset($parts['port'])) ? $parts['port'] : (($scheme == 'https') ? '443' : '80');
    $host = (isset($parts['host'])) ? $parts['host'] : '';
    $path = (isset($parts['path'])) ? $parts['path'] : '';

    if (($scheme == 'https' && $port != '443')
        || ($scheme == 'http' && $port != '80')) {
      $host = "$host:$port";
    }
    if ($use_force_port && $this->force_port) {
      $host = "$host:{$this->force_port}";
    }
    return "$scheme://$host$path";
  }
}
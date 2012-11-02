<?php

/**
 * @file
 * Session authentication for 3.x version
 */

class ServicesClientConnectionSessionAuth extends ServicesClientConnectionAuth {

  /**
   * Session ID
   */
  protected $sessid = NULL;

  /**
   * Session name
   */
  protected $session_name = NULL;

  /**
   * Logged in remote user
   */
  protected $user = NULL;

  /**
   * Implements configForm().
   */
  public function configForm(&$form, &$form_state) {
    $form['username'] = array(
      '#type' => 'textfield',
      '#title' => t('Username'),
      '#default_value' => isset($this->config['username']) ? $this->config['username'] : '',
    );

    $form['password'] = array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => isset($this->config['password']) ? $this->config['password'] : '',
    );
  }

  /**
   * Implements configFormSubmit().
   */
  public function configFormSubmit(&$form, &$form_state) {
    parent::configFormSubmit($form, $form_state);

    $form_state['config']['username'] = $form_state['values']['username'];
    $form_state['config']['password'] = $form_state['values']['password'];
  }

  /**
   * Implements login().
   */
  public function login() {
    $response = $this->client->action('user', 'login', array(
      'username' => $this->config['username'],
      'password' => $this->config['password']
    ));

    $this->sessid = $response['sessid'];
    $this->session_name = $response['session_name'];
    $this->user = $response['user'];
  }

  /**
   * Implements logout().
   */
  public function logout() {
    $response = $this->client->action('user', 'logout');

    $this->sessid = NULL;
    $this->session_name = NULL;
    $this->user = NULL;
  }

  /**
   * Implements prepareRequest().
   *
   * @param ServicesClientConnectionHttpRequest $request
   */
  public function prepareRequest(ServicesClientConnectionHttpRequest &$request) {
    parent::prepareRequest($request);

    if ($this->sessid) {
      $request->cookie[] = $this->session_name . '=' . $this->sessid;
    }
  }
}

<?php

class services_client_connection_ui extends ctools_export_ui {
  /**
   * Page callback for the auth page.
   */
  function auth_page($js, $input, $item) {
    drupal_set_title($this->get_page_title('auth', $item));
    return drupal_get_form('services_client_connection_plugin_config', 'auth', $item);
  }

  /**
   * Page callback for the server page.
   */
  function server_page($js, $input, $item) {
    drupal_set_title($this->get_page_title('server', $item));
    return drupal_get_form('services_client_connection_plugin_config', 'server', $item);
  }


  /**
   * Page callback for the authentication page.
   */
  function request_page($js, $input, $item) {
    drupal_set_title($this->get_page_title('request', $item));
    return drupal_get_form('services_client_connection_plugin_config', 'request', $item);
  }
}

/**
 * Plugin configuration form
 */
function services_client_connection_plugin_config($form, &$form_state, $type, $item) {
  // Get plugin name and configuration
  $name = $item->config[$type]['plugin'];
  $config = $item->config[$type]['config'];

  // Get new plugin
  $plugin = services_client_connection_get_plugin($type, $name, $item, $config);

  // Run config form function
  $plugin->configForm($form, $form_state);

  $form_state += array(
    'type' => $type,
    'item' => $item,
    'plugin' => $plugin,
    'config' => $config
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  
  return $form;
}

function services_client_connection_plugin_config_submit($form, &$form_state) {
  $plugin = $form_state['plugin'];
  
  $plugin->configFormSubmit($form, $form_state);

  $item = $form_state['item'];
  $item->config[$form_state['type']]['config'] = $form_state['config'];

  $result = ctools_export_crud_save('services_client_connection', $item);

  if ($result) {
    drupal_set_message(t('Configuration has been saved.'));
  }
}
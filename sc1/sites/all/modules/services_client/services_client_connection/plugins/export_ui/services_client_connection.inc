<?php

/**
 * @file
 * Define Ctools Exportable plugin
 */

$plugin = array(
  // The name of the table as found in the schema in hook_install. This
  // must be an exportable type with the 'export' section defined.
  'schema' => 'services_client_connection',

  // The access permission to use. If not provided it will default to
  // 'administer site configuration'
  'access' => 'administer services client connection',

  // You can actually define large chunks of the menu system here. Nothing
  // is required here. If you leave out the values, the prefix will default
  // to admin/structure and the item will default to the plugin name.
  'menu' => array(
    'menu prefix' => 'admin/structure/services_client',
    'menu item' => 'connection',
    // Title of the top level menu. Note this should not be translated,
    // as the menu system will translate it.
    'menu title' => 'Connection',
    // Description of the top level menu, which is usually needed for
    // menu items in an administration list. Will be translated
    // by the menu system.
    'menu description' => 'Administer services client connections.',
    // Add services specific own menu callbacks.
    'items' => array(
      'auth' => array(
        'path' => 'list/%ctools_export_ui/auth',
        'title' => 'Auth',
        'page callback' => 'ctools_export_ui_switcher_page',
        'page arguments' => array('services_client_connection', 'auth', 5),
        'load arguments' => array('services_client_connection'),
        'access arguments' => array('administer services client connection'),
        'type' => MENU_LOCAL_TASK,

      ),
      'server' => array(
        'path' => 'list/%ctools_export_ui/server',
        'title' => 'Server',
        'page callback' => 'ctools_export_ui_switcher_page',
        'page arguments' => array('services_client_connection', 'server', 5),
        'load arguments' => array('services_client_connection'),
        'access arguments' => array('administer services client connection'),
        'type' => MENU_LOCAL_TASK,
        'weight' => -1,
      ),
      'request' => array(
        'path' => 'list/%ctools_export_ui/request',
        'title' => 'Request',
        'page callback' => 'ctools_export_ui_switcher_page',
        'page arguments' => array('services_client_connection', 'request', 5),
        'load arguments' => array('services_client_connection'),
        'access arguments' => array('administer services client connection'),
        'type' => MENU_LOCAL_TASK,
        'weight' => -1,
      ),
    ),
  ),

  // These are required to provide proper strings for referring to the
  // actual type of exportable. "proper" means it will appear at the
  // beginning of a sentence.
  'title singular' => t('services client connection'),
  'title singular proper' => t('Services client connection'),
  'title plural' => t('services client connection'),
  'title plural proper' => t('Services client connections'),

  // This will provide you with a form for editing the properties on your
  // exportable, with validate and submit handler.
  //
  // The item being edited will be in $form_state['item'].
  //
  // The submit handler is only responsible for moving data from
  // $form_state['values'] to $form_state['item'].
  //
  // All callbacks will accept &$form and &$form_state as arguments.

  // Add our custom operations.
  'allowed operations' => array(
    'auth' => array('title' => t('Edit Authentication')),
    'server'  => array('title' => t('Edit Server')),
    'request'  => array('title' => t('Edit Request')),
  ),
  'form' => array(
    'settings' => 'services_client_connection_export_ui_form',
    'validate' => 'services_client_connection_export_ui_form_validate',
    'submit'   => 'services_client_connection_export_ui_form_submit',
  ),
  'handler' => array(
    'class' => 'services_client_connection_ui',
    'parent' => 'ctools_export_ui',
  ),
);

/**
 * Basic form configuration
 */
function services_client_connection_export_ui_form(&$form, &$form_state) {
  $item = &$form_state['item'];

  // For ajax refreshing
  $version = isset($form_state['values']['version']) ? $form_state['values']['version'] : $item->version;
  $version = $version ? $version : 3;

  $form['version'] = array(
    '#type' => 'select',
    '#title' => t('Services version'),
    '#options' => drupal_map_assoc(range(1,3)),
    '#default_value' => $version,
    '#ajax' => array(
      'callback' => 'services_client_connection_export_ui_ajax',
      'wrapper' => 'scc-plugins',
    ),
  );

  $form['endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint'),
    '#default_value' => $item->endpoint,
    '#description' => t('Remote endpoint URL'),
  );

  $form['debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug'),
    '#default_value' => $item->debug
  );

  $form['plugins'] = array(
    '#tree' => FALSE,
    '#prefix' => '<div id="scc-plugins">',
    '#suffix' => '</div>',
    '#type' => 'item',
  );

  $auth_plugins = services_client_connection_get_plugins('auth', $version, TRUE);
  $none = array('' => '- ' . t('None') . ' -');
  $form['plugins']['auth'] = array(
    '#type' => 'select',
    '#title' => t('Authentication'),
    '#options' => $none + $auth_plugins,
    '#description' => t('Select authentication plugin. If none is selected all requests will be unauthenticated'),
    '#default_value' => $item->config['auth']['plugin'],
  );

  $server_plugins = services_client_connection_get_plugins('server', $version, TRUE);
  $form['plugins']['server'] = array(
    '#type' => 'select',
    '#title' => t('Server'),
    '#options' => $server_plugins,
    '#default_value' => $item->config['server']['plugin'],
  );

  $request_plugin = services_client_connection_get_plugins('request', $version, TRUE);
  $form['plugins']['request'] = array(
    '#type' => 'select',
    '#title' => t('Request'),
    '#options' => $request_plugin,
    '#default_value' => $item->config['request']['plugin'],
  );
}

/**
 * Update list of available plugins when changing services version
 */
function services_client_connection_export_ui_ajax($form, $form_state) {
  return $form['plugins'];
}

/**
 * Validate function
 */
function services_client_connection_export_ui_form_validate($form, &$form_state) {

}

/**
 * Submit function
 */
function services_client_connection_export_ui_form_submit($form, &$form_state) {
  $item = &$form_state['item'];

  $item->config['auth']['plugin'] = $form_state['values']['auth'];
  $item->config['server']['plugin'] = $form_state['values']['server'];
  $item->config['request']['plugin'] = $form_state['values']['request'];
}
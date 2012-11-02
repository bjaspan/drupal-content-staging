<?php

/**
 * @file
 * Administration pages for configuring services client module.
 */

/**
 * Settings page callback
 */
function services_client_settings() {
  $form = array();

  $form['services_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('ID'),
    '#description' => t('Services client id of current site'),
    '#default_value' => variable_get('services_client_id', drupal_get_token('services_client')),
    '#size' => "70",
  );

  $form['services_client_use_queue'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Queue'),
    '#default_value' => variable_get('services_client_use_queue', FALSE),
    '#description' => t('Use queue for processing new objects coming from remote sites.'),
  );

  $form['services_client_process_queue_cron'] = array(
    '#type' => 'checkbox',
    '#title' => t('Process Queue items in cron'),
    '#default_value' => variable_get('services_client_process_queue_cron', FALSE),
    '#description' => t('Process queue items in cron'),
    '#states' => array(
      'visible' => array(
        ':input[name="services_client_use_queue"]' => array('checked' => TRUE),
      )
    ),
  );

  $form['services_client_exclude_users'] = array(
    '#type' => 'textarea',
    '#title' => t('Exclude users'),
    '#default_value' => variable_get('services_client_exclude_users', '1'),
    '#description' => t('Enter which users should not be synced, comma separated i.e. 1,3,475,9949')
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 98,
  );

  $form['error_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Error handling'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#tree' => FALSE,
    '#group' => 'additional_settings',
    '#weight' => 0,
  );

  $form['error_settings']['services_client_notify'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send notification on errors'),
    '#default_value' => variable_get('services_client_notify', FALSE),
    '#description' => t('Send email notifications on services client erros'),
  );

  $form['error_settings']['services_client_notify_recipients'] = array(
    '#type' => 'textarea',
    '#title' => t('Notification recipients'),
    '#default_value' => variable_get('services_client_notify_recipients', ''),
    '#description' => t('Enter one email address per row'),
    '#states' => array(
      'visible' => array(
        ':input[name="services_client_notify"]' => array('checked' => TRUE),
      )
    ),
  );

  $form['#pre_render'][] = 'vertical_tabs_form_pre_render';

  return system_settings_form($form);
}

/**
 * List the all connections and hooks
 */
function services_client_admin_list() {
  // Load all hooks
  $hooks = services_client_get_client_hooks_list(array('num' => 'all'));
  $groupped = array();
  foreach ($hooks as $hook) {
    $groupped[$hook->conn_name][] = $hook;
  }

  // Load all connections
  $connections = services_client_connection_load_all();

  return array(
    '#theme' => 'services_client_admin_list',
    '#connections' => $connections,
    '#groupped_hooks' => $groupped,
  );
}

/**
 * Add/Edit hook form
 */
function services_client_hooks_form($form, &$form_state, $connection, $hook = NULL) {
  $form = array();

  $avail_hooks = array(
    'node_save'   => 'Node Save',
    'node_delete' => 'Node Delete',
    'user_save'   => 'User Save',
    'webform_submission_save' => 'Webform Submission Save',
  );

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Hook title'),
    '#description' => t('Specify a user-friendly name for this hook.'),
    '#size' => 25,
    '#default_value' => !empty($hook['title']) ? $hook['title'] : '',
    '#required' => TRUE,
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#default_value' => !empty($hook['name']) ? $hook['name'] : '',
    '#maxlength' => 32,
    '#description' => t('Specify a machine-name for this hook (lowercase and underscores only).'),
    '#machine_name' => array(
      'exists' => 'services_client_services_connection_hook_exists',
      'source' => array('title'),
      'label' => t('Machine name'),
      'conn_name' => $connection->name,
    ),
  );
  $form['hook'] = array(
    '#type' => 'select',
    '#title' => t('Drupal hook'),
    '#description' => t('Choose the Drupal hook for which to execute the services connection.'),
    '#options' => $avail_hooks,
    '#default_value' => !empty($hook['hook']) ? $hook['hook'] : '',
    '#required' => TRUE,
  );

  $form['connection'] = array(
    '#type' => 'value',
    '#value' => $connection,
  );

  if (!empty($hook)) {
    $form['hid'] = array(
      '#type' => 'value',
      '#value' => $hook['hid'],
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Hook'),
  );

  return $form;
}

/**
 * Submit hook form
 */
function services_client_hooks_form_submit($form, &$form_state) {
  $v = $form_state['values'];

  $conn = $v['connection'];

  if (empty($v['hid'])) {
    $sql = "INSERT INTO {services_client_connection_hook} SET conn_name = ?, title = ?, name = ?, hook = ?, hook_mappings = '', hook_conditions = ''";
    db_query($sql, array($conn->name, $v['title'], $v['name'], $v['hook']));
    drupal_set_message(t('Hook %title successfully added.', array('%title' => $v['title'])));
  }
  elseif (is_numeric($v['hid'])) {
    $sql = "UPDATE {services_client_connection_hook} SET conn_name = ?, title = ?, name = ?, hook = ? WHERE hid = ?";
    db_query($sql, array($conn->name, $v['title'], $v['name'], $v['hook'], $v['hid']));
    drupal_set_message(t('Hook %title successfully updated.', array('%title' => $v['title'])));
  } 

  $form_state['redirect'] = 'admin/structure/services_client';
}

/**
 * Delete hook page
 */
function services_client_hooks_delete($form, &$form_state, $connection, $hook) {
  $form = array();

  $form['hid'] = array(
    '#type' => 'value',
    '#value' => $hook['hid'],
  );

  return confirm_form(
    $form,
    t("Are you sure you want to delete hook %hook", array('%hook' => $hook['title'])),
    'admin/structure/services_client'
  );
}

/**
 * Delete hook submit
 */
function services_client_hooks_delete_submit($form, &$form_state) {
  $hid = $form_state['values']['hid'];

  // Delete hook
  db_query("DELETE FROM {services_client_connection_hook} WHERE hid = :hid", array(':hid' => $hid));

  // Go back to connection hooks page
  $form_state['redirect'] = 'admin/structure/services_client';
}

/**
 * Event execution condition form
 */
function services_client_hooks_condition_form($form, &$form_state, $connection, $hook) {
  $form = array();

  $form['hid'] = array(
    '#type' => 'hidden',
    '#value' => $hook['hid'],
  );

  $cond = unserialize($hook['hook_conditions']);

  foreach(node_type_get_types() as $type => $info) {
    $types[$type] = $info->name;
  }

  if (in_array($hook['hook'], array('node_save', 'node_delete', 'webform_submission_save'))) {
    $form['node_type'] = array(
      '#type' => 'select',
      '#title' => t('Content type'),
      '#description' => 'Select the content type on which this hook is conditional. Because each hook has its own mapping, only one content type per hook is allowed.',
      '#options' => $types,
      '#default_value' => $cond['node_type'],
      '#required' => TRUE,
    );
    $form['webform_nid'] = array(
      '#type' => 'textfield',
      '#title' => t('Node ID of the webform'),
      '#description' => 'Since site can contain more that one webform, let\'s stick to the Node id.',
      '#default_value' => $cond['webform_nid'],
      '#size' => 5,
      '#states' => array(
        'visible' => array(
          ':input[name="node_type"]' => array('value' => 'webform'),
        ),
      ),
    );
    $form['published'] = array(
      '#type' => 'select',
      '#title' => t('Published'),
      '#description' => 'Send only nodes that are published, unpublished, or both.',
      '#options' => array(
        'e' => 'Either',
        '1' => 'Published',
        '0' => 'Unpublished'
      ),
      '#default_value' => $cond['published'],
      '#required' => TRUE,
    );

  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Condition'),
  );

  return $form;
}

/**
 * Condition form submit
 */
function services_client_hooks_condition_form_submit($form, &$form_state) {
  $v = $form_state['values'];

  $sql = "UPDATE services_client_connection_hook SET hook_conditions = ? WHERE hid = ?";
  $servals = serialize(array(
      'node_type'=>$v['node_type'],
      'published'=>$v['published'],
      'webform_nid'=>$v['webform_nid'],
    )); 
  db_query($sql, array($servals, $v['hid']));

  drupal_set_message('Condition successfully saved.');

  $form_state['redirect'] = 'admin/structure/services_client';
}

/**
 * Mapping form
 */
function services_client_hooks_mapping_form($form, &$form_state, $connection, $hook) {
  $form = array();

  $mapping = unserialize($hook['hook_mappings']);
  
  $form['hid'] = array(
    '#type' => 'hidden',
    '#value' => $hook['hid'],
  );

  $form['field_mapping'] = array(
    '#type' => 'textarea',
    '#title' => t('Field Mappings'),
    '#description' => 'Enter mappings for client/master fields. Master on left, client on right, pipe separated, one per line.',
    '#rows' => 15,
    '#size' => 30,
    '#default_value' => !empty($mapping['field_mapping']) ? $mapping['field_mapping'] : '',
  );
  $form['field_mapping_empty'] = array(
    '#type' => 'textarea',
    '#title' => t('Empty mapping'),
    '#rows' => '15',
    '#default_value' => !empty($mapping['field_mapping_empty']) ? $mapping['field_mapping_empty'] : '',
  );
  if ($hook['hook'] == 'webform_submission_save') {
    $form['file_field_name'] = array(
      '#type' => 'textfield',
      '#title' => t('File field name'),
      '#description' => 'Enter file field name for master node types.',
      '#rows' => 5,
      '#size' => 30,
      '#default_value' => !empty($mapping['file_field_name']) ? $mapping['file_field_name'] : '',
    );
  }

  if (in_array($hook['hook'], array('node_save', 'webform_submission_save'))) {
    $form['node_type_mapping'] = array(
      '#type' => 'textarea',
      '#title' => t('Node Type Mappings'),
      '#description' => 'Enter mappings for client/master node types. Master on left, client on right, pipe separated, one per line.',
      '#rows' => 5,
      '#size' => 30,
      '#default_value' => !empty($mapping['node_type_mapping']) ? $mapping['node_type_mapping'] : '',
    );
  }

  if ($hook['hook'] == 'user_save') {
    $form['user_sync_roles'] = array(
      '#type' => 'textarea',
      '#title' => t('Roles sync'),
      '#description' => t('Enter which roles should be synced, one role per line in format local_role|remote_role'),
      '#default_value' => !empty($mapping['user_sync_roles']) ? $mapping['user_sync_roles'] : '',
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Mapping'),
  );

  return $form;
}

/**
 * Mapping form submit
 */
function services_client_hooks_mapping_form_submit($form, &$form_state) {
  $v = $form_state['values'];
  $sql = "UPDATE {services_client_connection_hook} SET hook_mappings = ? WHERE hid = ?";
  $mapping = serialize(array(
    'field_mapping' => $v['field_mapping'],
    'field_mapping_empty' => isset($v['field_mapping_empty']) ? $v['field_mapping_empty'] : '',
    'node_type_mapping' => isset($v['node_type_mapping']) ? $v['node_type_mapping'] : '',
    'user_sync_roles' => isset($v['user_sync_roles']) ? $v['user_sync_roles'] : '',
    'file_field_name' => isset($v['file_field_name']) ? $v['file_field_name'] : '',
  ));
  db_query($sql, array($mapping, $v['hid']));
  drupal_set_message(t('Mapping successfully saved.'));
  $form_state['redirect'] = 'admin/structure/services_client';
}
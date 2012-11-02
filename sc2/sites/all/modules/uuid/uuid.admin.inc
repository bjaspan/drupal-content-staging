<?php

/**
 * @file
 * Administration functions for the uuid module.
 */

/**
 * Menu callback: options for UUID.
 */
function uuid_admin() {
  $form = array();

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
  );

  $form['settings']['uuid_generator'] = array(
    '#type' => 'select',
    '#title' => t('UUID Generator'),
    '#default_value' => variable_get('uuid_generator', array()),
    '#description' => t('The method used to generate the UUID'),
    '#options' => _uuid_admin_generators()
  );

  $form['sync'] = array(
    '#type' => 'fieldset',
    '#title' => t('Synchronization'),
  );
  $form['sync']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create missing UUIDs'),
    '#submit' => array('uuid_sync_all'),
  );

  return system_settings_form($form);
}

/**
 * Prepares a list of UUID generators for select #options in settings form.
 *
 * @return 
 *   List of available UUID generators.
 */
function _uuid_admin_generators() {
  $options = array();
  $implementations = uuid_info();
  foreach ($implementations as $name => $data) {
    $options[$name] = $data['title'];
  }

  return $options;
}

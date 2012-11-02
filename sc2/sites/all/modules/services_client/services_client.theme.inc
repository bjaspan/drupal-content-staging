<?php

/**
 * @file
 * Theme functions
 */

/**
 * Theme connections list page
 * @param $vars
 */
function theme_services_client_admin_list($vars) {
  $output = array();

  // Process each connection
  foreach ($vars['element']['#connections'] as $connection) {
    $output[] = '<h2>' . check_plain($connection->admin_title) . ' (' . l(t('Edit'), 'admin/structure/services_client/connection/list/'. $connection->name .'/edit') . ')</h2>';
    $output[] = '<p>' . l(t('Add new hook'), 'admin/structure/services_client/hook-add/' . $connection->name) . '</p>';

    // List all hook
    $header = array(t('Title'), t('Name'), t('Type'), t('Actions'));
    if (isset($vars['element']['#groupped_hooks'][$connection->name]) && !empty($vars['element']['#groupped_hooks'][$connection->name])) {

      $rows = array();
      foreach ($vars['element']['#groupped_hooks'][$connection->name] as $hook) {
        $actions = array(
          l(t('Edit Conditions'),'admin/structure/services_client/hook-condition/' . $connection->name . '/' . $hook->hid),
          l(t('Edit Mapping'),'admin/structure/services_client/hook-mapping/' . $connection->name . '/' . $hook->hid),
          l(t('Edit Hook'),'admin/structure/services_client/hook-edit/' . $connection->name . '/' . $hook->hid),
          l(t('Delete Hook'),'admin/structure/services_client/hook-delete/' . $connection->name . '/' . $hook->hid),
        );

        $rows[] = array(
          check_plain($hook->title),
          check_plain($hook->name),
          check_plain($hook->hook),
          implode(' | ', $actions),
        );
      }


      if (count($rows)) {
        $output[] = theme('table', array('header' => $header, 'rows' => $rows));
      }
    }
  }

  $output[] = '<p>' . l(t('Add new connection'), 'admin/structure/services_client/connection/add') . '</p>';

  return implode('', $output);
}
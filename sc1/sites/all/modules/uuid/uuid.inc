<?php

/**
 * @file
 * Generic UUID API functions
 */

/**
 * Generates an Universally Unique IDentifier (UUID).
 *
 * @return
 *   An UUID, made up of 32 hex digits and 4 hyphens.
 */
function uuid_uuid() {
  static $callback;
  if (is_null($callback)) {
    $var_name = 'uuid_generator';
    $generators = module_invoke_all('uuid_info');
    $generator = variable_get($var_name, NULL);
    if (!is_null($generator) && isset($generators[$generator])) {
      $config = $generators[$generator];
    }
    else {
      $config = _uuid_uuid_php_info();
      variable_set($var_name, 'php');
    }

    if (isset($config['include'])) {
      drupal_load($config['include']['module'], $config['include']['filename']);
    }

    $callback = $config['callback'];
  }
  return $callback();
}

/**
 * Finds the entity id by looking up the UUID.
 *
 * @param $uuid
 *   The uuid to look up.
 * @param $table
 *   The table to perform the search on.
 * @param $id_column
 *   The column containing the entity id.
 * @param $uuid_column
 *   The column containing the UUIDs.
 *
 * @return
 *   The id for the entity or FALSE if not found
 */
function uuid_find($uuid, $table, $id_column, $uuid_column = 'uuid') {
  return db_select($table, ' t')
    ->fields('t', array($id_column))
    ->condition($uuid_column, $uuid, '=')
    ->execute()
    ->fetchField();
}

/**
 * Generate all missing UUIDs.
 */
function uuid_sync_all() {
  module_invoke_all('uuid_sync');
  drupal_set_message('Generated missing UUIDs.');
}

/**
 * Ensure all rows in a table have a UUID
 */
function uuid_sync($table, $id_column, $uuid_column = 'uuid') {
  $generator = variable_get('uuid_generator', 'php');
  if ('mysql' == $generator) {
    db_update($table)
      ->expression($uuid_column, 'UUID()')
      ->condition(db_or()->condition($uuid_column, '')->isNull($uuid_column))
      ->execute();
    return;
  }

  $result = db_select($table, 't')
    ->fields('t', array($id_column))
    ->condition(db_or()->condition($uuid_column, '')->isNull($uuid_column))
    ->execute();

  foreach ($result as $record) {
    $id = $record->{$id_column};

    db_update($table)
      ->fields(array($uuid_column => uuid_uuid()))
      ->condition($id_column, $id)
      ->execute();
  }
}

/**
 * Check that a string appears to be in the format of a UUID.
 *
 * @param $uuid
 *  The string to test.
 *
 * @return
 *   TRUE if the string is well formed.
 */
function uuid_is_well_formed($uuid) {
  return preg_match("/^[0-9a-f]{8}-([0-9a-f]{4}-){3}[0-9a-f]{12}$/", $uuid);
}

/**
 * Fetches information about the available UUID implementations.
 */
function uuid_info() {
  return module_invoke_all('uuid_info');
}

/**
 * Implements hook_uuid_info().
 */
function uuid_uuid_info() {
  $sources = array();

  // PECL and OSSP are 2 incompatiable implementations in PHP that use the same namespace.
  if (function_exists('uuid_create') && !function_exists('uuid_make')) {
    $sources['pecl'] = array(
      'callback' => 'uuid_pecl',
      'title' => t('PECL Extension'),
    );
  }

  if (Database::getConnection()->databaseType() == 'mysql') {
    $sources['mysql'] = array(
      'callback' => 'uuid_mysql',
      'title' => t('MySQL - SELECT UUID()'),
    );
  }

  if (function_exists('com_create_guid')) {
    $sources['com'] = array(
      'callback' => 'uuid_com',
      'title' => t('Windows COM'),
    );
  }

  $sources['php'] = _uuid_uuid_php_info();

  return $sources;
}

/**
 * Fetches the PHP fallback generator config.
 */
function _uuid_uuid_php_info() {
  return array(
    'callback' => 'uuid_php',
    'title' => t('PHP Fallback'),
  );
}

// UUID generators

/**
 * Generates a UUID using the Windows internal GUID generator.
 *
 * See http://php.net/com_create_guid
 */
function uuid_com() {
  // remove {} wrapper and make lower case to keep result consistent
  return drupal_strtolower(trim(com_create_guid(), '{}'));
}

/**
 * Generates a UUID using MySQL's implementation.
 *
 * See http://dev.mysql.com/doc/refman/5.0/en/miscellaneous-functions.html#function_uuid
 */
function uuid_mysql() {
  return db_query('SELECT UUID()')->fetchField();
}

/**
 * Generates a UUID using the PECL extension.
 *
 * See http://pecl.php.net/package/uuid
 */
function uuid_pecl() {
  return uuid_create(UUID_TYPE_DEFAULT);
}

/**
 * Generates a UUID v4 using PHP code.
 *
 * See http://php.net/uniqid#65879
 */
function uuid_php() {
  // The field names refer to RFC 4122 section 4.1.2.
  return sprintf('%04x%04x-%04x-%03x4-%04x-%04x%04x%04x',
    // 32 bits for "time_low".
    mt_rand(0, 65535), mt_rand(0, 65535),
    // 16 bits for "time_mid".
    mt_rand(0, 65535),
    // 12 bits before the 0100 of (version) 4 for "time_hi_and_version".
    mt_rand(0, 4095),
    bindec(substr_replace(sprintf('%016b', mt_rand(0, 65535)), '01', 6, 2)),
    // 8 bits, the last two of which (positions 6 and 7) are 01, for "clk_seq_hi_res"
    // (hence, the 2nd hex digit after the 3rd hyphen can only be 1, 5, 9 or d)
    // 8 bits for "clk_seq_low" 48 bits for "node".
    mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535)
  );
}

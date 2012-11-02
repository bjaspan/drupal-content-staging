<?php

/**
 * @file
 * Installer helper functions for adding UUIDs to schemas.
 */

/**
 * Processes the new fields and indicies for a table.
 *
 * @param
 *   $schema The schema for the new fields.
 *
 *  @param
 *   $table The name of the database table to alter.
 *
 *  @param
 *    $return The return values from adding the fields.
 */
function uuid_add_field($schema, $table) {
  foreach ($schema[$table]['fields'] as $name => $field) {
    db_add_field($table, $name, $field);
  }

  foreach ($schema[$table]['indexes'] as $name => $fields) {
    db_add_index($table, $name, $fields);
  }
}

/**
 * Removes the UUID fields from the specified table.
 */
function uuid_drop_field($schema, $table) {
  foreach (array_keys($schema[$table]['indexes']) as $index) {
    db_drop_index($table, $index);
  }

  foreach (array_keys($schema[$table]['fields']) as $field) {
    db_drop_field($table, $field);
  }
}

/**
 * Adds a UUID column to the schema for the specifed table.
 *
 * @param
 *   $schema Reference to the schema structure to update.
 *
 * @param
 *   $table The name of the table to add the UUID column to.
 *
 * @param
 *   $col_name The name of the UUID column.
 *
 * @see hook_schema_alter()
 */
function uuid_schema_generate(&$schema, $table, $col_name = 'uuid') {
  $schema[$table]['fields'][$col_name] = array(
    'type' => 'char',
    'length' => 36,
    'not null' => TRUE,
    'default' => '',
    'description' => 'The Universally Unique Identifier.',
  );
  $schema[$table]['indexes']['uuid'] = array($col_name);
}

<?php

/**
 * @file
 * Users sessions lists.
 */

/**
 * Page callback for admin/people/sessions/%session_guard_type.
 */
function sessions_table($session = 'active') {
  $output = l(
    t('Delete all'),
    'admin/people/sessions/' . $session . '/delete-all',
    array('attributes' => array('class' => array('button button-delete-all')))
  );
  $output .= sessions_get_table($session);
  drupal_add_css(drupal_get_path('module', 'session_guard') . '/css/session_guard.css');

  return $output;
}

/**
 * Form content for admin/people/sessions/%session_guard_type/delete-all.
 */
function sessions_list_delete_all_form($form, &$form_state, $session) {
  $form['session'] = array(
    '#type' => 'value',
    '#value' => $session,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete all %session?', array('%session' => $session)),
    'admin/people/sessions/' . $session,
    t('This action cannot be undone.'),
    t('Delete')
  );
}

/**
 * Submit handler for sessions_list_delete_all_form.
 */
function sessions_list_delete_all_form_submit($form, &$form_state) {
  $session = $form_state['values']['session'];
  db_truncate($session . '_session')->execute();
  drupal_set_message('All ' . $session . ' sessions have been deleted.');
  $form_state['redirect'] = 'admin/people/sessions/' . $session;
}

/**
 * Gets the session list table with all records.
 */
function sessions_get_table($session) {
  $header_fields = session_guard_get_header_fields($session);
  $header = array();

  foreach ($header_fields as $header_field => $field) {
    $header[substr($field, 2)] = array('data' => t($header_field), 'field' => $field);
  }

  $header['actions'] = array('data' => t('Actions'));
  $rows = array();
  $records = get_session_table_records($session, $header);

  foreach ($records as $record) {
    $rows[] = create_table_row($session, $record);
  }

  return theme('table', array('header' => $header, 'rows' => $rows)).theme('pager');
}

/**
 * Gets the session table header fields.
 */
function session_guard_get_header_fields($session) {
  $header_fields = array(
    'active' => array(
      'Uid' => 's.uid',
      'User' => 's.name',
      'Email' => 'u.mail',
      'Opened sessions' => 's.opened_sessions',
      'Date' => 's.created',
    ),
    'fishy' => array(
      'Uid' => 's.uid',
      'Email' => 'u.mail',
      'New hostname' => 's.hostname',
      'Previous hostname' => 's.hostname_old',
      'New date' => 's.timestamp',
      'Previous date' => 's.timestamp_old'
    ),
  );

  return $header_fields[$session];
}

/**
 * Gets the session table records.
 */
function get_session_table_records($session, $header) {
  $query = db_select($session . '_session','s');
  $query->join('users', 'u', 's.uid = u.uid');
  $query->fields('s');
  $query->fields('u', array('mail', 'name'));
  $query = $query->extend('TableSort')->orderByHeader($header);
  $query = $query->extend('PagerDefault')->limit(50);
  $query = $query->orderBy('created', 'DESC');
  
  return $query->execute()->fetchAll();
}

/**
 * Creates the table row.
 */
function create_table_row($session, $record) {
  $row = call_user_func('get_' . $session . '_table_row', $record);
  $row[] = array('data' => l(t('Delete'), 'admin/people/sessions/' . $session . '/' . $record->id . '/delete'));

  return $row;
}

/**
 * Gets the active table row.
 */
function get_active_table_row($record) {
  return array(
    array('data' => l($record->uid, 'user/' . $record->uid)),
    array('data' => $record->name),
    array('data' => $record->mail),
    array('data' => $record->opened_sessions),
    array('data' => date('d-m-Y H:i:s', $record->created)),
  );
}

/**
 * Gets the fishy table row.
 */
function get_fishy_table_row($record) {
  return array(
    array('data' => l($record->uid, 'user/' . $record->uid)),
    array('data' => $record->mail),
    array('data' => $record->hostname),
    array('data' => $record->hostname_old),
    array('data' => date('d-m-Y H:i:s', $record->timestamp)),
    array('data' => date('d-m-Y H:i:s', $record->timestamp_old)),
  );
}

/**
 * Page callback for admin/people/sessions/%session_guard_type/%/delete.
 */
function sessions_delete_form($form, &$form_state, $session, $id) {
  $form['session'] = array(
    '#type' => 'value',
    '#value' => $session,
  );
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete this session?'),
    'admin/people/sessions/' . $session,
    t('This action cannot be undone.'),
    t('Delete')
  );
}

/**
 * Submit handler for sessions_delete_form.
 */
function sessions_delete_form_submit($form, &$form_state) {
  $session = $form_state['values']['session'];

  if ($id = $form_state['values']['id']) {
    db_delete($session . '_session')
      ->condition('id', $id, '=')
      ->execute();
  }

  drupal_set_message(t('The %session_guard_type session has been deleted.', array('%session_guard_type' => $session)));
  $form_state['redirect'] = 'admin/people/sessions/' . $session;
}

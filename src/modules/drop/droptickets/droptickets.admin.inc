<?php
/**
 * @file
 * Admin forms for DropTickets.
 */

// ---------------------------------------------------------------------------
// Ticket form
// ---------------------------------------------------------------------------

/**
 * Form: add or edit a ticket.
 */
function droptickets_ticket_form($form, &$form_state, $ticket = NULL) {
  global $user;

  $is_edit = !empty($ticket);

  // Store the entity on the form for use in validate/submit.
  if (!$is_edit) {
    $ticket = entity_create('drop_ticket', array());
  }
  $form['#ticket'] = $ticket;

  $form['#attached']['css'][] = backdrop_get_path('module', 'droptickets') . '/droptickets.css';

  if ($is_edit) {
    backdrop_set_breadcrumb(array(
      l(t('Home'), '<front>'),
      l(t('Drop Admin'), 'admin/drop'),
      l(t('Tickets'), 'admin/drop/tickets'),
      l(droptickets_ticket_label($ticket) . ': ' . check_plain($ticket->title), 'admin/drop/tickets/' . $ticket->tid),
    ));
  }
  else {
    backdrop_set_breadcrumb(array(
      l(t('Home'), '<front>'),
      l(t('Drop Admin'), 'admin/drop'),
      l(t('Tickets'), 'admin/drop/tickets'),
    ));
  }

  // Open two-column layout wrapper and main column.
  $form['title'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Title'),
    '#default_value' => $is_edit ? $ticket->title : '',
    '#required'      => TRUE,
    '#maxlength'     => 255,
    '#prefix'        => '<div class="droptickets-form-layout"><div class="droptickets-form-main">',
    '#weight'        => -20,
  );

  $form['body'] = array(
    '#type'          => 'text_format',
    '#title'         => t('Description'),
    '#default_value' => $is_edit ? $ticket->body : '',
    '#format'        => $is_edit ? $ticket->body_format : NULL,
    '#rows'          => 10,
    '#suffix'        => '</div>',
    '#weight'        => -19,
  );

  // Open sidebar column. Client + Project chained selects are first.
  global $user;

  // Load all active projects with their contact (client) info.
  $project_rows = db_select('drop_project', 'p')
    ->fields('p', array('pid', 'name', 'cid'))
    ->condition('p.active', 1)
    ->orderBy('p.name', 'ASC')
    ->execute()
    ->fetchAll();

  // Build a map of cid => client name for clients that have projects.
  $cids = array_filter(array_unique(array_map(function($r) { return $r->cid; }, $project_rows)));
  $client_names = array();
  if ($cids) {
    $client_names = db_select('crm_contact', 'c')
      ->fields('c', array('cid', 'name'))
      ->condition('c.cid', $cids, 'IN')
      ->execute()
      ->fetchAllKeyed();
    asort($client_names);
  }

  // Build a map of pid => cid for use in JS/AJAX.
  $projects_by_client = array();
  $ungrouped_projects = array();
  foreach ($project_rows as $row) {
    if (!empty($row->cid) && isset($client_names[$row->cid])) {
      $projects_by_client[$row->cid][$row->pid] = $row->name;
    }
    else {
      $ungrouped_projects[$row->pid] = $row->name;
    }
  }

  // Determine the currently selected client from form state or existing ticket.
  $selected_cid = NULL;
  if (!empty($form_state['values']['client_cid'])) {
    $selected_cid = (int) $form_state['values']['client_cid'];
  }
  elseif ($is_edit && !empty($ticket->pid)) {
    // Derive cid from the existing project.
    foreach ($project_rows as $row) {
      if ($row->pid == $ticket->pid) {
        $selected_cid = $row->cid ?: 0;
        break;
      }
    }
  }

  // Client options: sorted clients + an "No client" option for ungrouped.
  $client_options = array();
  foreach ($client_names as $cid => $name) {
    $client_options[$cid] = $name;
  }
  if ($ungrouped_projects) {
    $client_options[0] = t('— No client —');
  }

  $form['client_cid'] = array(
    '#type'          => 'select',
    '#title'         => t('Client'),
    '#options'       => $client_options,
    '#empty_option'  => t('— Select client —'),
    '#default_value' => $selected_cid,
    '#required'      => TRUE,
    '#prefix'        => '<div class="droptickets-form-sidebar">',
    '#weight'        => -19,
    '#ajax'          => array(
      'callback' => 'droptickets_project_ajax',
      'wrapper'  => 'droptickets-project-wrapper',
      'effect'   => 'fade',
    ),
  );

  // Project options filtered by selected client.
  if ($selected_cid !== NULL) {
    $project_options = ($selected_cid == 0)
      ? $ungrouped_projects
      : (isset($projects_by_client[$selected_cid]) ? $projects_by_client[$selected_cid] : array());
  }
  else {
    $project_options = array();
  }

  $form['pid'] = array(
    '#type'          => 'select',
    '#title'         => t('Project'),
    '#options'       => $project_options,
    '#empty_option'  => $selected_cid === NULL ? t('— Select a client first —') : t('— Select —'),
    '#required'      => TRUE,
    '#default_value' => $is_edit ? $ticket->pid : (!empty($_GET['pid']) ? (int) $_GET['pid'] : NULL),
    '#prefix'        => '<div id="droptickets-project-wrapper">',
    '#suffix'        => '</div>',
    '#weight'        => -18,
  );

  $form['type'] = array(
    '#type'          => 'select',
    '#title'         => t('Type'),
    '#options'       => droptickets_type_options(),
    '#default_value' => $is_edit ? $ticket->type : 'task',
    '#required'      => TRUE,
    '#weight'        => -17,
  );

  $form['priority'] = array(
    '#type'          => 'select',
    '#title'         => t('Priority'),
    '#options'       => array_reverse(drop_base_priority_labels(), TRUE),
    '#default_value' => $is_edit ? $ticket->priority : 2,
    '#weight'        => -16,
  );

  if ($is_edit) {
    $status_options = droptickets_status_options();
    if (!user_access('close droptickets') && !user_access('administer site configuration')) {
      unset($status_options['done']);
    }
    $form['status'] = array(
      '#type'          => 'select',
      '#title'         => t('Status'),
      '#options'       => $status_options,
      '#default_value' => $ticket->status,
      '#description'   => t('Not everyone can close tickets.'),
      '#weight'        => -15,
    );
  }

  $team_options = droptickets_assignee_options();

  $form['owner_uid'] = array(
    '#type'          => 'select',
    '#title'         => t('Owner'),
    '#description'   => t('Person responsible for driving this ticket forward.'),
    '#options'       => $team_options,
    '#empty_option'  => t('— None —'),
    '#default_value' => $is_edit ? $ticket->owner_uid : NULL,
    '#weight'        => -15,
  );

  // Assignee options: special modes first, then individual team members.
  $assignee_options = array(
    'team'    => t('— The Team —'),
    'on_hold' => t('— On Hold —'),
  ) + $team_options;

  // Determine current assignee select value.
  if ($is_edit) {
    if ($ticket->assignee_mode === 'team') {
      $default_assignee = 'team';
    }
    elseif ($ticket->assignee_mode === 'on_hold') {
      $default_assignee = 'on_hold';
    }
    else {
      $default_assignee = $ticket->assignee_uid;
    }
  }
  else {
    $default_assignee = NULL;
  }

  $form['assignee_uid'] = array(
    '#type'          => 'select',
    '#title'         => t('Assignee'),
    '#description'   => t('Who needs to act on this right now.'),
    '#options'       => $assignee_options,
    '#empty_option'  => t('— Unassigned —'),
    '#default_value' => $default_assignee,
    '#weight'        => -14,
  );

  $form['due_date'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Due date'),
    '#default_value' => ($is_edit && $ticket->due_date) ? format_date($ticket->due_date, 'custom', 'Y-m-d') : '',
    '#size'          => 12,
    '#maxlength'     => 10,
    '#description'   => t('YYYY-MM-DD format, optional.'),
    '#attributes'    => array('class' => array('droptickets-datepicker')),
    '#weight'        => -13,
  );

  // External link closes the sidebar and layout wrapper.
  $form['external_link'] = array(
    '#type'          => 'textfield',
    '#title'         => t('External link'),
    '#default_value' => ($is_edit && !empty($ticket->external_link)) ? $ticket->external_link : '',
    '#maxlength'     => 512,
    '#description'   => t('Optional URL, e.g. the original JIRA issue.'),
    '#suffix'        => '</div></div>',
    '#weight'        => -12,
  );

  $form['#attached']['js'][] = 'core/misc/ui/jquery.ui.datepicker.min.js';
  $form['#attached']['css'][] = 'core/misc/ui/jquery.ui.core.css';
  $form['#attached']['css'][] = 'core/misc/ui/jquery.ui.theme.css';
  $form['#attached']['js'][]  = backdrop_get_path('module', 'droptickets') . '/droptickets.js';

  if ($is_edit) {
    $form['tid'] = array('#type' => 'hidden', '#value' => $ticket->tid);
  }

  // On create, show Slack silence option if a webhook is configured.
  if (!$is_edit && config_get('droptickets.settings', 'slack_webhook_url')) {
    $form['silence_slack'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Silence Slack notification'),
      '#default_value' => 0,
      '#weight'        => 24,
    );
  }

  // On create, show watcher checkboxes pre-populated from config.
  if (!$is_edit) {
    $team            = droptickets_assignee_options();
    $saved_defaults  = config_get('droptickets.settings', 'default_watchers');
    $default_uids    = ($saved_defaults !== NULL) ? $saved_defaults : array_keys($team);
    $form['watchers'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Notify'),
      '#description' => t('These people will be added as watchers and notified when the ticket is created.'),
      '#collapsible' => FALSE,
      '#weight'      => 25,
    );
    $form['watchers']['watcher_uids'] = array(
      '#type'          => 'checkboxes',
      '#options'       => $team,
      '#default_value' => $default_uids,
    );
  }

  // Attach any Field API fields defined for this entity.
  field_attach_form('drop_ticket', $ticket, $form, $form_state);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => $is_edit ? t('Update ticket') : t('Add ticket'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => ' ' . l(t('Cancel'), $is_edit ? 'admin/drop/tickets/' . $ticket->tid : 'admin/drop/tickets'),
  );

  return $form;
}

/**
 * Validation handler for droptickets_ticket_form().
 */
function droptickets_ticket_form_validate($form, &$form_state) {
  $due = trim($form_state['values']['due_date']);
  if ($due && !preg_match('/^\d{4}-\d{2}-\d{2}$/', $due)) {
    form_set_error('due_date', t('Due date must be in YYYY-MM-DD format.'));
  }
  if (!empty($form_state['values']['status']) && $form_state['values']['status'] === 'done'
      && !user_access('close droptickets') && !user_access('administer site configuration')) {
    form_set_error('status', t('You do not have permission to mark tickets as done.'));
  }
  field_attach_form_validate('drop_ticket', $form['#ticket'], $form, $form_state);
}

/**
 * AJAX callback: returns the project select element after a client is chosen.
 */
function droptickets_project_ajax($form, &$form_state) {
  return $form['pid'];
}

/**
 * Submit handler for droptickets_ticket_form().
 */
function droptickets_ticket_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  if (!empty($values['tid'])) {
    $tickets = entity_load('drop_ticket', array($values['tid']));
    $ticket  = $tickets ? reset($tickets) : new DropTicket();
  }
  else {
    $ticket = new DropTicket();
  }

  $ticket->pid         = (int) $values['pid'];
  $ticket->title       = trim($values['title']);
  $ticket->body        = $values['body']['value'];
  $ticket->body_format = $values['body']['format'];
  $ticket->type        = $values['type'];
  $ticket->priority    = (int) $values['priority'];
  $ticket->owner_uid   = !empty($values['owner_uid']) ? (int) $values['owner_uid'] : NULL;

  // Parse combined assignee select: 'team', 'on_hold', or a numeric UID.
  $assignee_val = $values['assignee_uid'];
  if ($assignee_val === 'team') {
    $ticket->assignee_uid  = NULL;
    $ticket->assignee_mode = 'team';
  }
  elseif ($assignee_val === 'on_hold') {
    $ticket->assignee_uid  = NULL;
    $ticket->assignee_mode = 'on_hold';
  }
  else {
    $ticket->assignee_uid  = !empty($assignee_val) ? (int) $assignee_val : NULL;
    $ticket->assignee_mode = 'user';
  }

  $due = trim($values['due_date']);
  $ticket->due_date = $due ? strtotime($due . ' 00:00:00') : NULL;

  $external_link = trim($values['external_link']);
  $ticket->external_link = $external_link !== '' ? $external_link : NULL;

  if (isset($values['status'])) {
    $ticket->status = $values['status'];
  }

  $is_new = empty($values['tid']);

  field_attach_submit('drop_ticket', $ticket, $form, $form_state);

  // Store the silence flag so hook_entity_insert() can read it.
  $silence = &drupal_static('droptickets_silence_slack', FALSE);
  $silence = !empty($values['silence_slack']);

  $ticket->save();

  // On create: save watchers and notify them.
  if ($is_new && isset($values['watcher_uids'])) {
    global $user;
    $selected = array_keys(array_filter($values['watcher_uids']));
    foreach ($selected as $uid) {
      db_merge('drop_ticket_watcher')
        ->key(array('tid' => $ticket->tid, 'uid' => (int) $uid))
        ->fields(array('tid' => $ticket->tid, 'uid' => (int) $uid))
        ->execute();
      if ($uid != $user->uid) {
        droptickets_notify_create((int) $uid, $ticket->tid, NULL, 'created', $user->uid);
      }
    }
  }

  backdrop_set_message(t('Ticket %label saved.', array('%label' => droptickets_ticket_label($ticket))));
  $form_state['redirect'] = 'admin/drop/tickets/' . $ticket->tid;
}

// ---------------------------------------------------------------------------
// Ticket delete form
// ---------------------------------------------------------------------------

/**
 * Form: confirm deletion of a ticket.
 */
function droptickets_ticket_delete_form($form, &$form_state, DropTicket $ticket) {
  $form['#ticket'] = $ticket;
  return confirm_form(
    $form,
    t('Delete ticket %label?', array('%label' => droptickets_ticket_label($ticket))),
    'admin/drop/tickets/' . $ticket->tid,
    t('All comments on this ticket will also be deleted. This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for droptickets_ticket_delete_form().
 */
function droptickets_ticket_delete_form_submit($form, &$form_state) {
  $ticket = $form['#ticket'];

  // Delete associated comments.
  $tcids = db_select('drop_ticket_comment', 'c')
    ->fields('c', array('tcid'))
    ->condition('c.tid', $ticket->tid)
    ->execute()
    ->fetchCol();
  if ($tcids) {
    entity_delete_multiple('drop_ticket_comment', $tcids);
  }

  // Delete associated relationships.
  db_delete('drop_ticket_relationship')
    ->condition(db_or()
      ->condition('tid_a', $ticket->tid)
      ->condition('tid_b', $ticket->tid)
    )
    ->execute();

  $ticket->delete();
  backdrop_set_message(t('Ticket deleted.'));
  $form_state['redirect'] = 'admin/drop/tickets';
}

// ---------------------------------------------------------------------------
// Relate / unrelate forms
// ---------------------------------------------------------------------------

/**
 * Form: add a relationship between two tickets.
 */
function droptickets_relate_form($form, &$form_state, DropTicket $ticket) {
  $form['#ticket'] = $ticket;

  $form['ticket_ref'] = array(
    '#type'              => 'textfield',
    '#title'             => t('Ticket'),
    '#autocomplete_path' => 'admin/drop/tickets/autocomplete',
    '#required'          => TRUE,
    '#description'       => t('Start typing a ticket number or title.'),
    '#size'              => 50,
  );

  $form['type'] = array(
    '#type'          => 'select',
    '#title'         => t('Relationship'),
    '#options'       => droptickets_relationship_type_options(),
    '#default_value' => 'relates_to',
    '#required'      => TRUE,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Add relation'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => ' ' . l(t('Cancel'), 'admin/drop/tickets/' . $ticket->tid),
  );

  return $form;
}

/**
 * Validation handler for droptickets_relate_form().
 */
function droptickets_relate_form_validate($form, &$form_state) {
  $ticket = $form['#ticket'];
  $ref    = trim($form_state['values']['ticket_ref']);

  // Parse tid from "PREFIX-N: Title [tid]" format.
  if (!preg_match('/\[(\d+)\]$/', $ref, $m)) {
    form_set_error('ticket_ref', t('Please select a ticket from the autocomplete list.'));
    return;
  }

  $tid_b = (int) $m[1];

  if ($tid_b === (int) $ticket->tid) {
    form_set_error('ticket_ref', t('A ticket cannot be related to itself.'));
    return;
  }

  // Check the related ticket exists.
  $related = entity_load('drop_ticket', array($tid_b));
  if (!$related) {
    form_set_error('ticket_ref', t('Ticket not found.'));
    return;
  }

  // Check the relationship doesn't already exist in either direction.
  $exists = db_select('drop_ticket_relationship', 'r')
    ->fields('r', array('trid'))
    ->condition(
      db_or()
        ->condition(db_and()
          ->condition('r.tid_a', $ticket->tid)
          ->condition('r.tid_b', $tid_b))
        ->condition(db_and()
          ->condition('r.tid_a', $tid_b)
          ->condition('r.tid_b', $ticket->tid))
    )
    ->execute()
    ->fetchField();

  if ($exists) {
    form_set_error('ticket_ref', t('These tickets are already related.'));
    return;
  }

  $form_state['values']['tid_b'] = $tid_b;
}

/**
 * Submit handler for droptickets_relate_form().
 */
function droptickets_relate_form_submit($form, &$form_state) {
  $ticket = $form['#ticket'];
  $tid_b  = (int) $form_state['values']['tid_b'];
  $type   = $form_state['values']['type'];

  db_insert('drop_ticket_relationship')
    ->fields(array(
      'tid_a' => $ticket->tid,
      'tid_b' => $tid_b,
      'type'  => $type,
    ))
    ->execute();

  backdrop_set_message(t('Relation added.'));
  $form_state['redirect'] = 'admin/drop/tickets/' . $ticket->tid;
}

/**
 * Form: confirm removal of a ticket relationship.
 */
function droptickets_unrelate_confirm_form($form, &$form_state, DropTicket $ticket, $trid) {
  $trid = (int) $trid;
  $row  = db_select('drop_ticket_relationship', 'r')
    ->fields('r')
    ->condition('r.trid', $trid)
    ->execute()
    ->fetchObject();

  if (!$row || ($row->tid_a != $ticket->tid && $row->tid_b != $ticket->tid)) {
    backdrop_set_message(t('Relationship not found.'), 'error');
    backdrop_goto('admin/drop/tickets/' . $ticket->tid);
  }

  $form['#ticket'] = $ticket;
  $form['#trid']   = $trid;

  return confirm_form(
    $form,
    t('Remove this relationship?'),
    'admin/drop/tickets/' . $ticket->tid,
    t('This action cannot be undone.'),
    t('Remove'),
    t('Cancel')
  );
}

/**
 * Submit handler for droptickets_unrelate_confirm_form().
 */
function droptickets_unrelate_confirm_form_submit($form, &$form_state) {
  $ticket = $form['#ticket'];
  db_delete('drop_ticket_relationship')
    ->condition('trid', $form['#trid'])
    ->execute();
  backdrop_set_message(t('Relationship removed.'));
  $form_state['redirect'] = 'admin/drop/tickets/' . $ticket->tid;
}

// ---------------------------------------------------------------------------
// Comment form
// ---------------------------------------------------------------------------

/**
 * Form: add or edit a comment.
 */
function droptickets_comment_form($form, &$form_state, DropTicket $ticket, $comment = NULL) {
  $is_edit = !empty($comment);

  $form['#ticket'] = $ticket;

  $form['body'] = array(
    '#type'          => 'text_format',
    '#title'         => $is_edit ? t('Edit comment') : t('Add comment'),
    '#default_value' => $is_edit ? $comment->body : '',
    '#format'        => $is_edit ? $comment->body_format : 'filtered_html',
    '#rows'          => 6,
    '#required'      => TRUE,
  );

  if ($is_edit) {
    $form['tcid'] = array('#type' => 'hidden', '#value' => $comment->tcid);
  }
  $form['tid'] = array('#type' => 'hidden', '#value' => $ticket->tid);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => $is_edit ? t('Update comment') : t('Post comment'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => ' ' . l(t('Cancel'), 'admin/drop/tickets/' . $ticket->tid),
  );

  return $form;
}

/**
 * Submit handler for droptickets_comment_form().
 */
function droptickets_comment_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  if (!empty($values['tcid'])) {
    $comments = entity_load('drop_ticket_comment', array($values['tcid']));
    $comment  = $comments ? reset($comments) : new DropTicketComment();
  }
  else {
    $comment = new DropTicketComment();
  }

  $comment->tid         = (int) $values['tid'];
  $comment->body        = $values['body']['value'];
  $comment->body_format = $values['body']['format'];
  $comment->save();

  // Notify watchers of the new comment.
  global $user;
  droptickets_notify_watchers($comment->tid, 'commented', $user->uid, $comment->tcid);

  backdrop_set_message(t('Comment saved.'));
  $form_state['redirect'] = 'admin/drop/tickets/' . $values['tid'];
}

// ---------------------------------------------------------------------------
// Comment delete form
// ---------------------------------------------------------------------------

/**
 * Form: confirm deletion of a comment.
 */
function droptickets_comment_delete_form($form, &$form_state, DropTicketComment $comment) {
  $form['#comment'] = $comment;
  return confirm_form(
    $form,
    t('Delete this comment?'),
    'admin/drop/tickets/' . $comment->tid,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for droptickets_comment_delete_form().
 */
function droptickets_comment_delete_form_submit($form, &$form_state) {
  $comment = $form['#comment'];
  $tid     = $comment->tid;
  $comment->delete();
  backdrop_set_message(t('Comment deleted.'));
  $form_state['redirect'] = 'admin/drop/tickets/' . $tid;
}

// ---------------------------------------------------------------------------
// Watchers form
// ---------------------------------------------------------------------------

/**
 * Form: manage ticket watchers.
 */
function droptickets_watchers_form($form, &$form_state, DropTicket $ticket) {
  $form_state['ticket'] = $ticket;

  $team    = droptickets_assignee_options();
  $current = array_map('intval', droptickets_watchers_get($ticket->tid));

  $form['watcher_uids'] = array(
    '#type'          => 'checkboxes',
    '#options'       => $team,
    '#default_value' => $current,
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Update'),
  );

  return $form;
}

/**
 * Submit: save watcher list.
 */
function droptickets_watchers_form_submit($form, &$form_state) {
  $ticket   = $form_state['ticket'];
  $selected = array_keys(array_filter($form_state['values']['watcher_uids']));

  db_delete('drop_ticket_watcher')
    ->condition('tid', $ticket->tid)
    ->execute();

  foreach ($selected as $uid) {
    db_insert('drop_ticket_watcher')
      ->fields(array('tid' => $ticket->tid, 'uid' => (int) $uid))
      ->execute();
  }

  $form_state['redirect'] = 'admin/drop/tickets/' . $ticket->tid;
}

// ---------------------------------------------------------------------------
// Settings form
// ---------------------------------------------------------------------------

/**
 * Form: DropTickets settings.
 */
function droptickets_settings_form($form, &$form_state) {
  $config = config('droptickets.settings');

  $form['#config'] = 'droptickets.settings';

  $form['notification_lifetime'] = array(
    '#type'          => 'select',
    '#title'         => t('Delete notifications after'),
    '#description'   => t('Notifications older than this will be deleted during cron. Choose "Never" to keep all notifications indefinitely.'),
    '#options'       => array(
      0   => t('Never'),
      7   => t('7 days'),
      30  => t('30 days'),
      60  => t('60 days'),
      90  => t('90 days'),
      365 => t('365 days'),
    ),
    '#default_value' => $config->get('notification_lifetime'),
  );

  $form['jira_base_url'] = array(
    '#type'          => 'textfield',
    '#title'         => t('JIRA base URL'),
    '#description'   => t('Your JIRA instance URL, e.g. <code>https://yourteam.atlassian.net</code>. Used to build ticket links when importing a CSV.'),
    '#default_value' => $config->get('jira_base_url'),
    '#maxlength'     => 255,
  );

  $form['slack_webhook_url'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Slack webhook URL'),
    '#description'   => t('Incoming webhook URL from Slack. When set, a message is posted to Slack whenever a new ticket is created. Leave blank to disable.'),
    '#default_value' => $config->get('slack_webhook_url'),
    '#maxlength'     => 512,
    '#placeholder'   => 'https://hooks.slack.com/services/…',
  );

  $form['stale_days'] = array(
    '#type'          => 'select',
    '#title'         => t('Stale ticket threshold'),
    '#description'   => t('Tickets not yet done and not updated within this many days appear in the Stale tickets report.'),
    '#options'       => array(
      1  => t('1 day'),
      3  => t('3 days'),
      7  => t('7 days'),
      14 => t('14 days'),
      20 => t('20 days'),
      30 => t('30 days'),
      60 => t('60 days'),
      90 => t('90 days'),
    ),
    '#default_value' => $config->get('stale_days'),
  );

  $form['flag_soft_limit'] = array(
    '#type'          => 'select',
    '#title'         => t('Flagged tickets soft limit'),
    '#description'   => t('Show a warning when the number of flagged tickets exceeds this value.'),
    '#options'       => array(
      2  => t('2'),
      4  => t('4'),
      6  => t('6'),
      8  => t('8'),
      10 => t('10'),
      12 => t('12'),
      15 => t('15'),
    ),
    '#default_value' => $config->get('flag_soft_limit'),
  );

  $form['reactions_enabled'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable emoji reactions'),
    '#description'   => t('Allow team members to react to tickets and comments with emoji. Reactions notify the author.'),
    '#default_value' => $config->get('reactions_enabled'),
  );

  $team = droptickets_assignee_options();
  $saved_defaults = $config->get('default_watchers');
  // If nothing saved yet, default to everyone.
  $default_watcher_uids = ($saved_defaults !== NULL) ? $saved_defaults : array_keys($team);

  $form['default_watchers'] = array(
    '#type'        => 'checkboxes',
    '#title'       => t('Default watchers on new tickets'),
    '#description' => t('Team members checked here will be pre-selected as watchers when creating a new ticket.'),
    '#options'     => $team,
    '#default_value' => $default_watcher_uids,
  );

  $form = system_settings_form($form);
  // Normalize checkboxes to a plain array of selected UIDs before saving.
  $form['#submit'][] = 'droptickets_settings_form_normalize_watchers';
  return $form;
}

/**
 * Submit handler: normalize default_watchers checkboxes to a plain array.
 *
 * system_settings_form saves checkboxes as ['uid' => 'uid', 'uid2' => 0].
 * We store a plain indexed array of selected UIDs instead.
 */
function droptickets_settings_form_normalize_watchers($form, &$form_state) {
  if (isset($form_state['values']['default_watchers'])) {
    $form_state['values']['default_watchers'] = array_values(
      array_keys(array_filter($form_state['values']['default_watchers']))
    );
  }
}

// ---------------------------------------------------------------------------
// JIRA CSV import
// ---------------------------------------------------------------------------

/**
 * Form: import tickets from a JIRA CSV export.
 *
 * Also shows a history of past imports with delete links.
 */
function droptickets_import_form($form, &$form_state) {
  $all_projects = db_select('drop_project', 'p')
    ->fields('p', array('pid', 'name'))
    ->condition('p.active', 1)
    ->orderBy('p.name', 'ASC')
    ->execute()
    ->fetchAllKeyed();

  $form['pid'] = array(
    '#type'          => 'select',
    '#title'         => t('Import into project'),
    '#options'       => $all_projects,
    '#empty_option'  => t('— Select a project —'),
    '#required'      => TRUE,
    '#description'   => t('All imported tickets will be added to this project.'),
  );

  $form['csv_file'] = array(
    '#type'        => 'file',
    '#title'       => t('JIRA CSV file'),
    '#description' => t('Export from JIRA using <em>Export Excel CSV (all fields)</em>. The file must contain at minimum a <em>Summary</em> column. A <em>Description</em> column is optional.'),
  );

  $form['#attributes']['enctype'] = 'multipart/form-data';

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Import tickets'),
  );

  // Import history.
  $imports = db_select('drop_ticket_import', 'i')
    ->fields('i')
    ->orderBy('i.created', 'DESC')
    ->execute()
    ->fetchAll();

  if (!empty($imports)) {
    $pids     = array_unique(array_column((array) $imports, 'pid'));
    $projects = entity_load('drop_project', $pids);
    $uids     = array_unique(array_column((array) $imports, 'uid'));
    $accounts = $uids ? user_load_multiple($uids) : array();

    $rows = array();
    foreach ($imports as $imp) {
      $proj_name = isset($projects[$imp->pid]) ? check_plain($projects[$imp->pid]->name) : '—';
      $importer  = isset($accounts[$imp->uid]) ? check_plain($accounts[$imp->uid]->name) : '—';
      $date      = format_date($imp->created, 'custom', 'M j, Y g:i A');
      $delete    = l(t('Delete'), 'admin/drop/tickets/import/' . $imp->import_id . '/delete');
      $rows[] = array($proj_name, $imp->ticket_count, $importer, $date, $delete);
    }

    $history = theme('table', array(
      'header' => array(t('Project'), t('Tickets'), t('Imported by'), t('Date'), t('Actions')),
      'rows'   => $rows,
    ));

    $form['history'] = array(
      '#markup' => '<h3>' . t('Import history') . '</h3>' . $history,
      '#weight' => 10,
    );
  }

  return $form;
}

/**
 * Validate handler: save the uploaded file and store it for the submit handler.
 */
function droptickets_import_form_validate($form, &$form_state) {
  $file = file_save_upload('csv_file', array(
    'file_validate_extensions' => array('csv'),
  ));
  if (!$file) {
    form_set_error('csv_file', t('Please select a CSV file to upload.'));
  }
  else {
    $form_state['storage']['import_file'] = $file;
  }
}

/**
 * Submit handler: parse CSV and create tickets with a batch import ID.
 */
function droptickets_import_form_submit($form, &$form_state) {
  global $user;

  $pid  = (int) $form_state['values']['pid'];
  $file = $form_state['storage']['import_file'];

  $path = backdrop_realpath($file->uri);
  $handle = fopen($path, 'r');
  if (!$handle) {
    backdrop_set_message(t('Could not open the uploaded file.'), 'error');
    return;
  }

  // Read header row and find column positions.
  $header = fgetcsv($handle, 0, ',', '"', '\\');
  if (empty($header)) {
    fclose($handle);
    backdrop_set_message(t('The CSV file appears to be empty.'), 'error');
    return;
  }

  // Normalize header names (trim whitespace, handle UTF-8 BOM).
  $header = array_map(function($col) {
    return trim(str_replace("\xEF\xBB\xBF", '', $col));
  }, $header);

  // Use case-insensitive column matching to handle JIRA export variations.
  $header_lower    = array_map('strtolower', $header);
  $col_summary     = array_search('summary', $header_lower);
  $col_description = array_search('description', $header_lower);
  // JIRA exports 'Issue key' or 'Issue Key' depending on version.
  $col_issue_key = array_search('issue key', $header_lower);
  if ($col_issue_key === FALSE) {
    $col_issue_key = array_search('issue id', $header_lower);
  }

  if ($col_summary === FALSE) {
    fclose($handle);
    backdrop_set_message(t('Could not find a <em>Summary</em> column in the CSV. Make sure you exported from JIRA with "all fields" or with Summary visible. Columns found: @cols', array(
      '@cols' => implode(', ', $header),
    )), 'error');
    return;
  }

  // Build JIRA links from the Issue key column if a base URL is configured.
  $jira_base_url = rtrim((string) config_get('droptickets.settings', 'jira_base_url'), '/');

  // Generate a unique batch ID for this import.
  $import_id = 'jira_' . REQUEST_TIME . '_' . substr(md5(uniqid('', TRUE)), 0, 6);

  $imported = 0;
  $skipped  = 0;

  while (($row = fgetcsv($handle, 0, ',', '"', '\\')) !== FALSE) {
    $title = isset($row[$col_summary]) ? trim($row[$col_summary]) : '';
    if ($title === '') {
      $skipped++;
      continue;
    }

    $body = ($col_description !== FALSE && isset($row[$col_description]))
      ? trim($row[$col_description])
      : '';

    // Build external link from Issue key + base URL.
    // Validate the key looks like a real JIRA key (e.g. PROJ-123) before using it.
    $external_link = NULL;
    if ($col_issue_key !== FALSE && !empty($row[$col_issue_key]) && $jira_base_url !== '') {
      $issue_key = trim($row[$col_issue_key]);
      if (preg_match('/^[A-Z][A-Z0-9]+-\d+$/', $issue_key)) {
        $external_link = $jira_base_url . '/browse/' . $issue_key;
      }
    }

    $ticket = entity_create('drop_ticket', array(
      'pid'           => $pid,
      'title'         => $title,
      'body'          => $body,
      'body_format'   => 'plain_text',
      'type'          => 'task',
      'status'        => 'open',
      'priority'      => 3,
      'uid'           => $user->uid,
      'import_id'     => $import_id,
      'external_link' => $external_link,
    ));
    $ticket->save();
    $imported++;
  }

  fclose($handle);

  if ($imported > 0) {
    // Record the import batch.
    db_insert('drop_ticket_import')
      ->fields(array(
        'import_id'    => $import_id,
        'pid'          => $pid,
        'uid'          => $user->uid,
        'ticket_count' => $imported,
        'created'      => REQUEST_TIME,
      ))
      ->execute();

    backdrop_set_message(format_plural($imported,
      'Imported 1 ticket successfully. You can manage imports on the <a href="@url">Import from JIRA</a> tab.',
      'Imported @count tickets successfully. You can manage imports on the <a href="@url">Import from JIRA</a> tab.',
      array('@url' => url('admin/drop/tickets/import'))
    ));

    $form_state['redirect'] = 'admin/drop/tickets';
  }
  if ($skipped > 0) {
    backdrop_set_message(format_plural($skipped,
      'Skipped 1 row with an empty title.',
      'Skipped @count rows with empty titles.'
    ), 'warning');
  }
}

/**
 * Form: confirm deletion of an import batch.
 */
function droptickets_import_delete_form($form, &$form_state, $import_id) {
  $import = db_select('drop_ticket_import', 'i')
    ->fields('i')
    ->condition('i.import_id', $import_id)
    ->execute()
    ->fetchObject();

  if (!$import) {
    backdrop_set_message(t('Import batch not found.'), 'error');
    backdrop_goto('admin/drop/tickets/import');
  }

  $loaded  = entity_load('drop_project', array($import->pid));
  $project = $loaded ? reset($loaded) : NULL;
  $proj_name = $project ? check_plain($project->name) : t('unknown project');
  $date      = format_date($import->created, 'custom', 'M j, Y g:i A');

  $form['import_id'] = array('#type' => 'value', '#value' => $import_id);

  return confirm_form(
    $form,
    t('Delete @count imported tickets?', array('@count' => $import->ticket_count)),
    'admin/drop/tickets/import',
    t('This will permanently delete the @count tickets imported into %project on @date. This cannot be undone.', array(
      '@count'   => $import->ticket_count,
      '%project' => $proj_name,
      '@date'    => $date,
    )),
    t('Delete all @count tickets', array('@count' => $import->ticket_count)),
    t('Cancel')
  );
}

/**
 * Submit handler: delete all tickets from an import batch.
 */
function droptickets_import_delete_form_submit($form, &$form_state) {
  $import_id = $form_state['values']['import_id'];

  // Load ticket IDs to delete (entity delete handles watchers/notifications cleanup).
  $tids = db_select('drop_ticket', 't')
    ->fields('t', array('tid'))
    ->condition('t.import_id', $import_id)
    ->execute()
    ->fetchCol();

  if (!empty($tids)) {
    entity_delete_multiple('drop_ticket', $tids);
  }

  db_delete('drop_ticket_import')
    ->condition('import_id', $import_id)
    ->execute();

  backdrop_set_message(format_plural(count($tids),
    'Deleted 1 imported ticket.',
    'Deleted @count imported tickets.'
  ));

  $form_state['redirect'] = 'admin/drop/tickets/import';
}

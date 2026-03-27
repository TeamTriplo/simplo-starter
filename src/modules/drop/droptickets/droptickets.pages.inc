<?php
/**
 * @file
 * Page callbacks for DropTickets.
 */

// ---------------------------------------------------------------------------
// Ticket list
// ---------------------------------------------------------------------------

/**
 * Page callback: ticket list with status and project filters.
 */
function droptickets_list_page() {
  backdrop_set_title(t('Tickets'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  // Status filter.
  // 'all'         = all non-done tickets (default)
  // 'all_tickets' = truly everything including done
  // 'imported'    = imported tickets only
  // anything else = a specific status value
  $valid_statuses = array_merge(array('all', 'all_tickets', 'imported'), array_keys(droptickets_status_options()));
  $status = (isset($_GET['status']) && in_array($_GET['status'], $valid_statuses))
    ? $_GET['status']
    : 'all';

  // Search query.
  $search = isset($_GET['search']) ? trim($_GET['search']) : '';

  // Project filter.
  $pid_filter = (!empty($_GET['pid']) && is_numeric($_GET['pid'])) ? (int) $_GET['pid'] : 0;

  // Assignee filter.
  $assignee_filter = (!empty($_GET['assignee']) && is_numeric($_GET['assignee'])) ? (int) $_GET['assignee'] : 0;

  // Tag filter.
  $tag_filter = (!empty($_GET['tag']) && is_numeric($_GET['tag'])) ? (int) $_GET['tag'] : 0;

  // Combined filter row: status dropdown + project dropdown + search + add link.

  // Status select — "Active" (everything except Icebox and Closed) first,
  // then individual statuses grouped, then "All tickets" and "Imported".
  $status_opts_html = '<option value="all"' . ($status === 'all' ? ' selected' : '') . '>' . t('Active') . '</option>';
  $status_opts_html .= '<optgroup label="' . t('By status') . '">';
  foreach (droptickets_status_options() as $skey => $slabel) {
    $status_opts_html .= '<option value="' . check_plain($skey) . '"' . ($status === $skey ? ' selected' : '') . '>' . check_plain($slabel) . '</option>';
  }
  $status_opts_html .= '</optgroup>';
  $status_opts_html .= '<optgroup label="' . t('Other') . '">';
  $status_opts_html .= '<option value="all_tickets"' . ($status === 'all_tickets' ? ' selected' : '') . '>' . t('All tickets') . '</option>';
  $status_opts_html .= '<option value="imported"' . ($status === 'imported' ? ' selected' : '') . '>' . t('Imported') . '</option>';
  $status_opts_html .= '</optgroup>';

  // Project select — only projects that have non-imported tickets.
  $project_pids = db_select('drop_ticket', 't')
    ->fields('t', array('pid'))
    ->isNull('t.import_id')
    ->distinct()
    ->execute()->fetchCol();

  $proj_opts_html = '<option value="0"' . (!$pid_filter ? ' selected' : '') . '>' . t('— All projects —') . '</option>';
  $show_proj_select = FALSE;
  if (!empty($project_pids)) {
    $all_projects = entity_load('drop_project', $project_pids);
    uasort($all_projects, function($a, $b) { return strcasecmp($a->name, $b->name); });
    foreach ($all_projects as $proj) {
      $proj_opts_html .= '<option value="' . (int) $proj->pid . '"' . ($pid_filter === (int) $proj->pid ? ' selected' : '') . '>' . check_plain($proj->name) . '</option>';
    }
    $show_proj_select = TRUE;
  }

  // Active tag badge — shown inline when a tag filter is in use.
  $tag_badge = '';
  if ($tag_filter) {
    $term_load = entity_load('taxonomy_term', array($tag_filter));
    $term      = $term_load ? reset($term_load) : NULL;
    if ($term) {
      $clear_q = array('status' => $status);
      if ($pid_filter) { $clear_q['pid'] = $pid_filter; }
      if ($search !== '') { $clear_q['search'] = $search; }
      $tag_badge = ' <a href="' . url('admin/drop/tickets', array('query' => $clear_q)) . '" class="droptickets-tag-badge">'
        . check_plain($term->name) . ' ×</a>';
    }
  }

  // Assemble the single combined form.
  $filter_form = '<form method="get" action="' . url('admin/drop/tickets') . '" class="droptickets-filter-form">'
    . ($tag_filter ? '<input type="hidden" name="tag" value="' . (int) $tag_filter . '">' : '')
    . '<select name="status" class="droptickets-filter-select" onchange="this.form.submit()">' . $status_opts_html . '</select>';

  if ($show_proj_select) {
    $filter_form .= '<select name="pid" class="droptickets-filter-select" onchange="this.form.submit()">' . $proj_opts_html . '</select>';
  }

  $filter_form .= '<input type="search" name="search" value="' . check_plain($search) . '" placeholder="' . t('Search tickets…') . '" class="droptickets-search-input">'
    . '<button type="submit" class="droptickets-search-btn">' . icon('magnifying-glass', array('attributes' => array('width' => 15, 'height' => 15))) . '</button>'
    . '</form>';

  $output['filters'] = array(
    '#markup' => '<div class="droptickets-filter-row">' . $filter_form . $tag_badge . '</div>',
    '#weight' => -18,
  );

  // Build query.
  $query = db_select('drop_ticket', 't')
    ->fields('t')
    ->orderBy('t.priority', 'DESC')
    ->orderBy('t.created', 'ASC');

  if ($status === 'imported') {
    // Show only imported tickets regardless of status.
    $query->isNotNull('t.import_id');
  }
  else {
    // All normal views exclude imported tickets.
    $query->isNull('t.import_id');

    if ($status === 'all') {
      // Default: all active tickets — everything except Icebox and Closed.
      $query->condition('t.status', droptickets_active_statuses(), 'IN');
    }
    elseif ($status !== 'all_tickets') {
      // Specific status filter; 'all_tickets' applies no condition.
      $query->condition('t.status', $status);
    }
  }

  if ($pid_filter) {
    $query->condition('t.pid', $pid_filter);
  }
  if ($assignee_filter) {
    $query->condition('t.assignee_uid', $assignee_filter);
  }
  if ($tag_filter) {
    $query->join('field_data_field_drop_tags', 'ftag', 'ftag.entity_id = t.tid AND ftag.entity_type = :et AND ftag.deleted = 0', array(':et' => 'drop_ticket'));
    $query->condition('ftag.field_drop_tags_tid', $tag_filter);
  }
  if ($search !== '') {
    $query->condition(
      db_or()
        ->condition('t.title', '%' . db_like($search) . '%', 'LIKE')
        ->condition('t.body',  '%' . db_like($search) . '%', 'LIKE')
    );
  }

  $tickets = $query->extend('PagerDefault')->limit(25)->execute()->fetchAll();

  $empty_msg = $search !== '' ? t('No tickets matched "@search".', array('@search' => $search)) : t('No tickets with this status.');
  if (empty($tickets)) {
    $output['empty'] = array(
      '#markup' => '<p class="droptickets-empty">' . $empty_msg . '</p>',
    );
    return $output;
  }

  // Load comment counts in bulk for all tickets on this page.
  $ticket_tids = array_column((array) $tickets, 'tid');
  $comment_counts = array();
  if ($ticket_tids) {
    $count_query = db_select('drop_ticket_comment', 'c')
      ->fields('c', array('tid'))
      ->condition('c.tid', $ticket_tids, 'IN')
      ->groupBy('c.tid');
    $count_query->addExpression('COUNT(*)', 'cnt');
    foreach ($count_query->execute()->fetchAll() as $cr) {
      $comment_counts[$cr->tid] = (int) $cr->cnt;
    }
  }

  // Load projects and users in bulk.
  $pids     = array_unique(array_column((array) $tickets, 'pid'));
  $projects = entity_load('drop_project', $pids);

  $assignee_uids = array_filter(array_unique(array_column((array) $tickets, 'assignee_uid')));
  $owner_uids    = array_filter(array_unique(array_column((array) $tickets, 'owner_uid')));
  $all_uids      = array_unique(array_merge($assignee_uids, $owner_uids));
  $accounts      = $all_uids ? user_load_multiple($all_uids) : array();

  $type_options     = droptickets_type_options();
  $status_options   = droptickets_status_options();
  $priority_labels  = drop_base_priority_labels();

  // Load current flags for quick lookup.
  $flagged_tids = db_select('drop_ticket_flag', 'f')->fields('f', array('tid'))->execute()->fetchCol();
  $flagged_tids = array_flip($flagged_tids);
  $can_flag     = user_access('flag droptickets');
  $flag_count   = count($flagged_tids);

  // Bulk-load parent ticket titles.
  $list_parent_tids = array_filter(array_unique(array_column((array) $tickets, 'parent_tid')));
  $list_parent_titles = array();
  if (!empty($list_parent_tids)) {
    $list_parent_titles = db_select('drop_ticket', 'p')
      ->fields('p', array('tid', 'title'))
      ->condition('p.tid', $list_parent_tids, 'IN')
      ->execute()->fetchAllKeyed();
  }

  $rows = array();

  foreach ($tickets as $ticket) {
    $project  = isset($projects[$ticket->pid]) ? $projects[$ticket->pid] : NULL;
    $label    = droptickets_ticket_label($ticket, $project);
    if (!$project) {
      $proj_name = t('—');
    }
    elseif (module_exists('dropcrm') && dropcrm_project_access('view', $project)) {
      $proj_name = l($project->name, 'admin/drop/crm/projects/' . $project->pid);
    }
    else {
      $proj_name = check_plain($project->name);
    }
    $assignee  = droptickets_assignee_label($ticket, $accounts);
    $owner     = (!empty($ticket->owner_uid) && isset($accounts[$ticket->owner_uid]))
      ? check_plain($accounts[$ticket->owner_uid]->name) : '—';
    $type_label   = isset($type_options[$ticket->type]) ? $type_options[$ticket->type] : $ticket->type;
    $status_label = isset($status_options[$ticket->status]) ? $status_options[$ticket->status] : $ticket->status;
    $priority_class = 'drop-priority-' . drop_base_priority_class($ticket->priority);

    $is_flagged = isset($flagged_tids[$ticket->tid]);
    if ($can_flag) {
      $flag_icon = $is_flagged
        ? icon('flag-fill', array('attributes' => array('width' => 16, 'height' => 16)))
        : icon('flag', array('attributes' => array('width' => 16, 'height' => 16)));
      $dest_query = array_diff_key($_GET, array('q' => 1));
      $destination = current_path() . ($dest_query ? '?' . http_build_query($dest_query) : '');
      $flag_cell = l($flag_icon, 'admin/drop/tickets/' . $ticket->tid . '/flag', array(
        'html'       => TRUE,
        'query'      => array('destination' => $destination),
        'attributes' => array(
          'class' => array($is_flagged ? 'droptickets-flag-on' : 'droptickets-flag-off'),
          'title' => $is_flagged ? t('Remove flag') : t('Flag as priority'),
        ),
      ));
    }
    else {
      $flag_cell = $is_flagged
        ? icon('flag-fill', array('attributes' => array('width' => 16, 'height' => 16, 'class' => array('droptickets-flag-on'))))
        : '';
    }

    $priority_dot = '<span class="droptickets-priority-dot drop-priority-' . check_plain(drop_base_priority_class($ticket->priority)) . '" title="' . check_plain(isset($priority_labels[$ticket->priority]) ? $priority_labels[$ticket->priority] : '') . '"></span>';

    $parent_indicator = '';
    if (!empty($ticket->parent_tid) && isset($list_parent_titles[$ticket->parent_tid])) {
      $parent_indicator = '<span class="droptickets-parent-indicator">'
        . icon('arrow-bend-up-left', array('attributes' => array('width' => 10, 'height' => 10)))
        . ' ' . l(truncate_utf8(check_plain($list_parent_titles[$ticket->parent_tid]), 50, FALSE, TRUE), 'admin/drop/tickets/' . (int) $ticket->parent_tid)
        . '</span>';
    }

    $row_data = array(
      array('data' => $flag_cell, 'class' => array('droptickets-col-flag')),
      array('data' => '<span class="droptickets-ticket-id">' . check_plain($label) . '</span>', 'class' => array('droptickets-col-id')),
      array('data' => $proj_name, 'class' => array('droptickets-col-project')),
      array('data' => l($ticket->title, 'admin/drop/tickets/' . $ticket->tid) . $parent_indicator, 'class' => array('droptickets-col-title')),
      array('data' => $priority_dot, 'class' => array('droptickets-col-priority')),
      array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($ticket->status) . '">' . check_plain($status_label) . '</span>'),
    );

    $comment_count = isset($comment_counts[$ticket->tid]) ? $comment_counts[$ticket->tid] : 0;
    $comment_cell = $comment_count > 0
      ? '<span class="droptickets-comment-count">' . $comment_count . '</span>'
      : '<span class="droptickets-comment-count droptickets-comment-count--none">—</span>';

    $row_data[] = array('data' => $owner);
    $row_data[] = array('data' => $assignee);
    $row_data[] = array('data' => $comment_cell, 'class' => array('droptickets-col-comments'));
    $row_data[] = array('data' => l(t('Edit'), 'admin/drop/tickets/' . $ticket->tid . '/edit'));

    $rows[] = array(
      'data'  => $row_data,
      'class' => array($priority_class),
    );
  }

  $header = array('', t('ID'), t('Project'), t('Title'), t('P'), t('Status'));
  $header[] = t('Owner');
  $header[] = t('Assignee');
  $header[] = icon('chat-circle', array('attributes' => array('width' => 14, 'height' => 14, 'title' => t('Comments'))));
  $header[] = t('Actions');

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => $header,
    '#rows'       => $rows,
    '#attributes' => array('class' => array('droptickets-list')),
  );

  $output['pager'] = array(
    '#theme'  => 'pager',
    '#weight' => 10,
  );

  return $output;
}

// ---------------------------------------------------------------------------
// Member ticket page
// ---------------------------------------------------------------------------

/**
 * Page callback: tickets owned by or assigned to a specific team member.
 *
 * @param object $account
 *   The fully-loaded user account (from %user wildcard).
 */
function droptickets_member_page($account) {
  backdrop_set_title(t('@name — tickets', array('@name' => $account->name)));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  $output['back'] = array(
    '#markup' => '<p>' . l(t('← Back to team'), 'admin/drop/team') . '</p>',
    '#weight' => -10,
  );

  $status_options   = droptickets_status_options();
  $priority_labels  = drop_base_priority_labels();

  // Fetch owned and assigned tickets (active statuses only).
  $active = droptickets_active_statuses();

  $owned_tickets = db_select('drop_ticket', 't')
    ->fields('t')
    ->condition('t.owner_uid', $account->uid)
    ->condition('t.status', $active, 'IN')
    ->isNull('t.import_id')
    ->orderBy('t.priority', 'DESC')
    ->orderBy('t.created', 'ASC')
    ->execute()->fetchAll();

  $assigned_tickets = db_select('drop_ticket', 't')
    ->fields('t')
    ->condition('t.assignee_uid', $account->uid)
    ->condition('t.status', $active, 'IN')
    ->isNull('t.import_id')
    ->orderBy('t.priority', 'DESC')
    ->orderBy('t.created', 'ASC')
    ->execute()->fetchAll();

  // Bulk-load all referenced projects and user accounts.
  $all_tickets = array_merge($owned_tickets, $assigned_tickets);
  $pids     = array_unique(array_column((array) $all_tickets, 'pid'));
  $projects = $pids ? entity_load('drop_project', $pids) : array();

  $uids = array_filter(array_unique(array_merge(
    array_column((array) $all_tickets, 'owner_uid'),
    array_column((array) $all_tickets, 'assignee_uid')
  )));
  $accounts = $uids ? user_load_multiple($uids) : array();

  // Helper to build a ticket table.
  // $extra_col_header: column header string.
  // $extra_col_type: 'assignee' uses droptickets_assignee_label(); 'owner' does a plain uid lookup.
  $build_table = function($tickets, $projects, $accounts, $extra_col_header, $extra_col_type) use ($status_options, $priority_labels) {
    if (empty($tickets)) {
      return array('#markup' => '<p class="droptickets-empty">' . t('None.') . '</p>');
    }
    $rows = array();
    foreach ($tickets as $ticket) {
      $project = isset($projects[$ticket->pid]) ? $projects[$ticket->pid] : NULL;
      $label   = droptickets_ticket_label($ticket, $project);
      $status_label   = isset($status_options[$ticket->status]) ? $status_options[$ticket->status] : $ticket->status;
      $priority_class = 'drop-priority-' . drop_base_priority_class($ticket->priority);
      $priority_dot   = '<span class="droptickets-priority-dot ' . $priority_class . '" title="' . check_plain(isset($priority_labels[$ticket->priority]) ? $priority_labels[$ticket->priority] : '') . '"></span>';
      if ($extra_col_type === 'assignee') {
        $extra_name = droptickets_assignee_label($ticket, $accounts);
      }
      else {
        $owner_uid  = !empty($ticket->owner_uid) ? (int) $ticket->owner_uid : 0;
        $extra_name = ($owner_uid && isset($accounts[$owner_uid])) ? check_plain($accounts[$owner_uid]->name) : '—';
      }
      $rows[] = array(
        array('data' => '<span class="droptickets-ticket-id">' . check_plain($label) . '</span>'),
        array('data' => l($ticket->title, 'admin/drop/tickets/' . $ticket->tid)),
        array('data' => $priority_dot),
        array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($ticket->status) . '">' . check_plain($status_label) . '</span>'),
        array('data' => $extra_name),
      );
    }
    return array(
      '#theme'  => 'table',
      '#header' => array(t('ID'), t('Title'), t('P'), t('Status'), $extra_col_header),
      '#rows'   => $rows,
      '#attributes' => array('class' => array('droptickets-list')),
    );
  };

  $output['owned_heading'] = array(
    '#markup' => '<h2>' . t('Owner') . '</h2>',
    '#weight' => 10,
  );
  $output['owned'] = $build_table($owned_tickets, $projects, $accounts, t('Assignee'), 'assignee') + array('#weight' => 11);

  $output['assigned_heading'] = array(
    '#markup' => '<h2>' . t('Assigned') . '</h2>',
    '#weight' => 20,
  );
  $output['assigned'] = $build_table($assigned_tickets, $projects, $accounts, t('Owner'), 'owner') + array('#weight' => 21);

  return $output;
}

// ---------------------------------------------------------------------------
// Kanban board
// ---------------------------------------------------------------------------

/**
 * Page callback: Kanban board showing active tickets in 5 columns.
 */
function droptickets_kanban_page() {
  backdrop_set_title(t('Kanban'));

  $module_path = backdrop_get_path('module', 'droptickets');
  $output = array(
    '#attached' => array(
      'css' => array($module_path . '/droptickets.css'),
      'js'  => array(
        $module_path . '/droptickets.kanban.js',
        array(
          'data' => array('droptickets' => array(
            'kanbanStatusUrl' => url('admin/drop/tickets/'),
            'kanbanToken'     => backdrop_get_token('droptickets_kanban'),
          )),
          'type' => 'setting',
        ),
      ),
    ),
  );

  $kanban_statuses = droptickets_kanban_statuses();

  // --- Board filter ---
  $board_filter = (!empty($_GET['board']) && is_numeric($_GET['board'])) ? (int) $_GET['board'] : 0;

  // Load all boards from the drop_boards vocabulary.
  $all_boards = db_select('taxonomy_term_data', 'd')
    ->fields('d', array('tid', 'name'))
    ->condition('d.vocabulary', 'drop_boards')
    ->orderBy('d.weight')
    ->orderBy('d.name')
    ->execute()->fetchAllKeyed();

  // Identify the "General" board TID (first term named "General", case-insensitive).
  $general_tid = 0;
  foreach ($all_boards as $tid => $name) {
    if (strtolower(trim($name)) === 'general') {
      $general_tid = (int) $tid;
      break;
    }
  }

  // Board switcher pills.
  $is_general = (!$board_filter || $board_filter === $general_tid);
  $switcher = '<div class="droptickets-board-switcher">'
    . '<span class="droptickets-board-switcher-label">' . t('Board:') . '</span>';
  $switcher .= '<a href="' . url('admin/drop/tickets/kanban') . '" class="droptickets-board-pill'
    . ($is_general ? ' droptickets-board-pill--active' : '') . '">' . t('General') . '</a>';
  foreach ($all_boards as $tid => $name) {
    if ((int) $tid === $general_tid) { continue; }
    $active    = ($board_filter === (int) $tid);
    $switcher .= '<a href="' . url('admin/drop/tickets/kanban', array('query' => array('board' => $tid))) . '"'
      . ' class="droptickets-board-pill' . ($active ? ' droptickets-board-pill--active' : '') . '">'
      . check_plain($name) . '</a>';
  }
  $switcher .= '</div>';
  $output['board_switcher'] = array('#markup' => $switcher, '#weight' => -20);

  // --- Project filter ---
  $pid_filter = (!empty($_GET['pid']) && is_numeric($_GET['pid'])) ? (int) $_GET['pid'] : 0;

  // Top-level only toggle.
  $toplevel_only = !empty($_GET['toplevel']) ? 1 : 0;

  $all_projects = db_select('drop_project', 'p')
    ->fields('p', array('pid', 'name'))
    ->condition('p.active', 1)
    ->orderBy('p.name')
    ->execute()->fetchAllKeyed();

  $opts = '<option value="0"' . (!$pid_filter ? ' selected' : '') . '>' . t('— All projects —') . '</option>';
  foreach ($all_projects as $pid => $name) {
    $sel   = ($pid_filter === (int) $pid) ? ' selected' : '';
    $opts .= '<option value="' . (int) $pid . '"' . $sel . '>' . check_plain($name) . '</option>';
  }
  // Preserve board and top-level selections when changing project filter.
  $board_hidden    = $board_filter ? '<input type="hidden" name="board" value="' . (int) $board_filter . '">' : '';
  $toplevel_hidden = $toplevel_only ? '<input type="hidden" name="toplevel" value="1">' : '';
  $toplevel_check  = '<label class="droptickets-kanban-toplevel-toggle">'
    . '<input type="checkbox" name="toplevel" value="1" onchange="this.form.submit()"' . ($toplevel_only ? ' checked' : '') . '>'
    . t('Top-level only')
    . '</label>';
  $filter_html  = '<form class="droptickets-kanban-filter" method="get" action="' . url('admin/drop/tickets/kanban') . '">'
    . $board_hidden
    . '<label>' . t('Project:') . ' <select name="pid" onchange="this.form.submit()">' . $opts . '</select></label>'
    . $toplevel_check
    . '</form>';
  $output['filter'] = array('#markup' => $filter_html, '#weight' => -10);

  // --- Base query ---
  $query = db_select('drop_ticket', 't')
    ->fields('t')
    ->condition('t.status', array_keys($kanban_statuses), 'IN')
    ->isNull('t.import_id')
    ->orderBy('t.priority', 'DESC')
    ->orderBy('t.created', 'ASC');

  if ($pid_filter) {
    $query->condition('t.pid', $pid_filter);
  }
  if ($toplevel_only) {
    $query->condition(
      db_or()
        ->condition('t.parent_tid', 0)
        ->isNull('t.parent_tid')
    );
  }

  // --- Board scoping ---
  // Use PHP set operations on pre-fetched TID lists to avoid JOIN/DISTINCT complexity.
  if (!$is_general) {
    // Focused board: tickets explicitly assigned to this board.
    $board_tids = db_select('field_data_field_drop_board', 'fb')
      ->fields('fb', array('entity_id'))
      ->condition('fb.entity_type', 'drop_ticket')
      ->condition('fb.deleted', 0)
      ->condition('fb.field_drop_board_tid', $board_filter)
      ->execute()->fetchCol();
    if (empty($board_tids)) {
      $output['empty'] = array('#markup' => '<p class="droptickets-empty">' . t('No tickets on this board yet.') . '</p>');
      return $output;
    }
    $query->condition('t.tid', $board_tids, 'IN');
  }
  else {
    // General board: tickets with no board assignment, OR explicitly tagged "General".
    // Exclude tickets that have a non-General board assignment but no General assignment.
    $all_boarded = db_select('field_data_field_drop_board', 'fb')
      ->fields('fb', array('entity_id'))
      ->condition('fb.entity_type', 'drop_ticket')
      ->condition('fb.deleted', 0)
      ->execute()->fetchCol();

    if (!empty($all_boarded)) {
      $general_boarded = $general_tid
        ? db_select('field_data_field_drop_board', 'fb')
            ->fields('fb', array('entity_id'))
            ->condition('fb.entity_type', 'drop_ticket')
            ->condition('fb.deleted', 0)
            ->condition('fb.field_drop_board_tid', $general_tid)
            ->execute()->fetchCol()
        : array();

      // Tickets that have board tags but NOT the General tag are excluded.
      $exclude_tids = array_values(array_unique(array_diff($all_boarded, $general_boarded)));
      if (!empty($exclude_tids)) {
        $query->condition('t.tid', $exclude_tids, 'NOT IN');
      }
    }
  }

  $tickets = $query->execute()->fetchAll();

  // Bulk-load related data.
  $pids          = array_unique(array_filter(array_column((array) $tickets, 'pid')));
  $projects      = $pids ? entity_load('drop_project', $pids) : array();
  $assignee_uids = array_filter(array_unique(array_column((array) $tickets, 'assignee_uid')));
  $accounts      = $assignee_uids ? user_load_multiple($assignee_uids) : array();
  $flagged_tids  = array_flip(
    db_select('drop_ticket_flag', 'f')->fields('f', array('tid'))->execute()->fetchCol()
  );
  $type_options  = droptickets_type_options();

  // Bulk-load parent ticket titles for cards that have a parent.
  $parent_tids = array_filter(array_unique(array_column((array) $tickets, 'parent_tid')));
  $parent_titles = array();
  if (!empty($parent_tids)) {
    $parent_rows = db_select('drop_ticket', 'p')
      ->fields('p', array('tid', 'title'))
      ->condition('p.tid', $parent_tids, 'IN')
      ->execute()->fetchAllKeyed();
    $parent_titles = $parent_rows;
  }

  // Group by status.
  $by_status = array_fill_keys(array_keys($kanban_statuses), array());
  foreach ($tickets as $ticket) {
    if (isset($by_status[$ticket->status])) {
      $by_status[$ticket->status][] = $ticket;
    }
  }

  // Load per-column soft limits.
  $kanban_limits = config_get('droptickets.settings', 'kanban_limits') ?: array();

  // Render board.
  $board = '<div class="droptickets-kanban">';
  foreach ($kanban_statuses as $status_key => $status_label) {
    $col_tickets = $by_status[$status_key];
    $count       = count($col_tickets);
    $limit       = isset($kanban_limits[$status_key]) ? (int) $kanban_limits[$status_key] : 0;
    $over_limit  = $limit > 0 && $count > $limit;

    $board .= '<div class="droptickets-kanban-col droptickets-kanban-col--' . check_plain($status_key) . '" data-status="' . check_plain($status_key) . '">';
    $board .= '<div class="droptickets-kanban-col-header">';
    $board .= '<span class="droptickets-status droptickets-status--' . check_plain($status_key) . ' droptickets-kanban-status-label">' . check_plain($status_label) . '</span>';
    $board .= '<span class="droptickets-kanban-col-count' . ($over_limit ? ' droptickets-kanban-col-count--over' : '') . '">' . $count . ($limit ? '/' . $limit : '') . '</span>';
    $board .= '</div>';
    if ($over_limit) {
      $board .= '<div class="droptickets-kanban-limit-warning">' . icon('warning', array('attributes' => array('width' => 13, 'height' => 13))) . ' ' . t('Limit exceeded') . '</div>';
    }
    $board .= '<div class="droptickets-kanban-cards">';

    if (empty($col_tickets)) {
      $board .= '<div class="droptickets-kanban-empty">' . t('No tickets') . '</div>';
    }
    else {
      foreach ($col_tickets as $ticket) {
        $project    = isset($projects[$ticket->pid]) ? $projects[$ticket->pid] : NULL;
        $label      = droptickets_ticket_label($ticket, $project);
        $type_label = isset($type_options[$ticket->type]) ? $type_options[$ticket->type] : $ticket->type;
        $priority_class = 'drop-priority-' . drop_base_priority_class($ticket->priority);
        $is_flagged = isset($flagged_tids[$ticket->tid]);

        if ($ticket->assignee_mode === 'user' && !empty($ticket->assignee_uid) && isset($accounts[$ticket->assignee_uid])) {
          $assignee_name = check_plain($accounts[$ticket->assignee_uid]->name);
        }
        elseif ($ticket->assignee_mode === 'team') {
          $assignee_name = t('Team');
        }
        elseif ($ticket->assignee_mode === 'on_hold') {
          $assignee_name = t('On Hold');
        }
        else {
          $assignee_name = '—';
        }

        $flag_html = $is_flagged
          ? ' <span class="droptickets-kanban-flag">' . icon('flag-fill', array('attributes' => array('width' => 11, 'height' => 11))) . '</span>'
          : '';

        $parent_html = '';
        if (!empty($ticket->parent_tid) && isset($parent_titles[$ticket->parent_tid])) {
          $parent_html = '<div class="droptickets-kanban-parent">'
            . icon('arrow-bend-up-left', array('attributes' => array('width' => 10, 'height' => 10)))
            . ' ' . l(truncate_utf8(check_plain($parent_titles[$ticket->parent_tid]), 40, FALSE, TRUE), 'admin/drop/tickets/' . (int) $ticket->parent_tid)
            . '</div>';
        }

        $board .= '<div class="droptickets-kanban-card ' . $priority_class . '" draggable="true" data-tid="' . (int) $ticket->tid . '">';
        $board .= '<div class="droptickets-kanban-card-id"><span class="droptickets-ticket-id">' . check_plain($label) . '</span>' . $flag_html . '</div>';
        $board .= '<div class="droptickets-kanban-card-title">' . l($ticket->title, 'admin/drop/tickets/' . $ticket->tid) . '</div>';
        $board .= $parent_html;
        $board .= '<div class="droptickets-kanban-card-meta">';
        $board .= '<span class="droptickets-type droptickets-type--' . check_plain($ticket->type) . '">' . check_plain($type_label) . '</span>';
        $board .= '<span class="droptickets-kanban-assignee">' . $assignee_name . '</span>';
        $board .= '</div>';
        $board .= '</div>';
      }
    }

    $board .= '</div>'; // .droptickets-kanban-cards
    $board .= '</div>'; // .droptickets-kanban-col
  }
  $board .= '</div>'; // .droptickets-kanban

  $output['board'] = array('#markup' => $board, '#weight' => 0);
  return $output;
}

/**
 * AJAX callback: inline status update from the ticket view page.
 *
 * Expects POST fields: status, token.
 * Returns JSON: {ok: true, status: 'new_status'} or {ok: false, message: '...'}.
 */
function droptickets_inline_set_status(DropTicket $ticket) {
  backdrop_add_http_header('Content-Type', 'application/json');

  if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'POST required.'));
    exit();
  }

  $status = isset($_POST['status']) ? check_plain($_POST['status']) : '';
  $token  = isset($_POST['token'])  ? $_POST['token'] : '';

  if (!backdrop_valid_token($token, 'droptickets_inline_status')) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'Invalid token.'));
    exit();
  }

  $valid_statuses = array_keys(droptickets_status_options());
  if (!in_array($status, $valid_statuses)) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'Invalid status.'));
    exit();
  }

  if ($status === 'done' && !droptickets_done_access($ticket)) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'You do not have permission to mark tickets as done.'));
    exit();
  }
  if ($status === 'closed' && !droptickets_close_access($ticket)) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'Only a project manager can close tickets.'));
    exit();
  }

  $ticket->status = $status;
  $ticket->save();

  print backdrop_json_encode(array('ok' => TRUE, 'status' => $status));
  exit();
}

/**
 * AJAX callback: update a ticket's status from the Kanban drag-and-drop.
 *
 * Expects POST fields: status, token.
 * Returns JSON: {ok: true} or {ok: false, message: '...'}.
 */
function droptickets_kanban_set_status(DropTicket $ticket) {
  backdrop_add_http_header('Content-Type', 'application/json');

  $status = isset($_POST['status']) ? check_plain($_POST['status']) : '';
  $token  = isset($_POST['token'])  ? $_POST['token'] : '';

  if (!backdrop_valid_token($token, 'droptickets_kanban')) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'Invalid token.'));
    exit();
  }

  $valid_statuses = array_keys(droptickets_kanban_statuses());
  if (!in_array($status, $valid_statuses)) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'Invalid status.'));
    exit();
  }

  // Done requires done permission; closed requires close permission.
  if ($status === 'done' && !droptickets_done_access($ticket)) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'You do not have permission to mark tickets as done.'));
    exit();
  }
  if ($status === 'closed' && !droptickets_close_access($ticket)) {
    print backdrop_json_encode(array('ok' => FALSE, 'message' => 'Only a project manager can close tickets.'));
    exit();
  }

  $ticket->status = $status;
  $ticket->save();

  print backdrop_json_encode(array('ok' => TRUE));
  exit();
}

// ---------------------------------------------------------------------------
// Ticket view
// ---------------------------------------------------------------------------

/**
 * Page callback: view a single ticket with comments.
 */
function droptickets_ticket_view(DropTicket $ticket) {
  $loaded  = !empty($ticket->pid) ? entity_load('drop_project', array($ticket->pid)) : array();
  $project = $loaded ? reset($loaded) : NULL;
  $label   = droptickets_ticket_label($ticket, $project);

  backdrop_set_title($label . ': ' . check_plain($ticket->title), PASS_THROUGH);

  backdrop_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Drop Admin'), 'admin/drop'),
    l(t('Tickets'), 'admin/drop/tickets'),
  ));

  // Mark any unread notifications for this ticket as read.
  global $user;
  if ($user->uid) {
    droptickets_mark_read_for_ticket($ticket->tid, $user->uid);
  }

  $status_options  = droptickets_status_options();
  $type_options    = droptickets_type_options();
  $priority_labels = drop_base_priority_labels();

  $owner_name = '—';
  if (!empty($ticket->owner_uid)) {
    $owner_account = user_load($ticket->owner_uid);
    if ($owner_account) {
      $owner_name = check_plain($owner_account->name);
    }
  }

  $due       = $ticket->due_date ? format_date($ticket->due_date, 'custom', 'M j, Y') : '—';
  $proj_name = $project ? l($project->name, 'admin/drop/crm/projects/' . $project->pid) : '—';
  $type_label   = isset($type_options[$ticket->type]) ? $type_options[$ticket->type] : $ticket->type;
  $status_label = isset($status_options[$ticket->status]) ? $status_options[$ticket->status] : $ticket->status;
  $priority_label = isset($priority_labels[$ticket->priority]) ? $priority_labels[$ticket->priority] : '—';

  // Build tag chips manually to avoid core taxonomy formatter bugs.
  $lang      = LANGUAGE_NONE;
  $tag_items = isset($ticket->field_drop_tags[$lang]) ? $ticket->field_drop_tags[$lang] : array();
  $tag_html  = '';
  if ($tag_items) {
    $tids  = array_filter(array_column($tag_items, 'tid'));
    $terms = $tids ? taxonomy_term_load_multiple($tids) : array();
    $chips = array();
    foreach ($tids as $tid) {
      if (!empty($terms[$tid])) {
        $chips[] = '<span class="droptickets-tag">' . check_plain($terms[$tid]->name) . '</span>';
      }
    }
    if ($chips) {
      $tag_html = '<div class="droptickets-tags">' . implode(' ', $chips) . '</div>';
    }
  }

  // Build sidebar meta items.
  $sidebar  = '<div class="droptickets-view-sidebar">';

  // Actions at the top of the sidebar.
  if (droptickets_ticket_access('update', $ticket)) {
    $sidebar .= '<div class="droptickets-sidebar-actions">';
    $sidebar .= l(t('Edit ticket'), 'admin/drop/tickets/' . $ticket->tid . '/edit', array('attributes' => array('class' => array('droptickets-sidebar-edit'))));
    if (!empty($ticket->import_id)) {
      $sidebar .= l(t('Activate'), 'admin/drop/tickets/' . $ticket->tid . '/activate', array('attributes' => array('class' => array('droptickets-sidebar-activate'))));
    }
    if ($ticket->status !== 'closed' && droptickets_close_access($ticket)) {
      $sidebar .= l(t('Close'), 'admin/drop/tickets/' . $ticket->tid . '/close', array('attributes' => array('class' => array('droptickets-sidebar-close'))));
    }
    if (droptickets_ticket_access('delete', $ticket)) {
      $sidebar .= l(t('Delete'), 'admin/drop/tickets/' . $ticket->tid . '/delete', array('attributes' => array('class' => array('droptickets-sidebar-delete'))));
    }
    $sidebar .= '</div>';
  }

  // Build status field: inline select for editors, static badge for viewers.
  if (droptickets_ticket_access('update', $ticket)) {
    $status_options_all = droptickets_status_options();
    if (!droptickets_done_access($ticket)) {
      unset($status_options_all['done']);
    }
    if (!droptickets_close_access($ticket)) {
      unset($status_options_all['closed']);
    }
    $status_select = '<select class="droptickets-status-select droptickets-status--' . check_plain($ticket->status) . '"'
      . ' data-tid="' . (int) $ticket->tid . '"'
      . ' data-token="' . backdrop_get_token('droptickets_inline_status') . '"'
      . ' data-endpoint="' . url('admin/drop/tickets/' . $ticket->tid . '/inline-status') . '">';
    foreach ($status_options_all as $key => $label) {
      $status_select .= '<option value="' . check_plain($key) . '"' . ($key === $ticket->status ? ' selected' : '') . '>' . check_plain($label) . '</option>';
    }
    $status_select .= '</select>';
    $status_field = $status_select;
  }
  else {
    $status_field = '<span class="droptickets-status droptickets-status--' . check_plain($ticket->status) . '">' . check_plain($status_label) . '</span>';
  }

  // Parent ticket link.
  $parent_item = '';
  if (!empty($ticket->parent_tid)) {
    $parent_load = entity_load('drop_ticket', array($ticket->parent_tid));
    $parent      = $parent_load ? reset($parent_load) : NULL;
    if ($parent) {
      $parent_proj_load = !empty($parent->pid) ? entity_load('drop_project', array($parent->pid)) : array();
      $parent_proj      = $parent_proj_load ? reset($parent_proj_load) : NULL;
      $parent_label = droptickets_ticket_label($parent, $parent_proj);
      $parent_item  = l($parent_label . ': ' . $parent->title, 'admin/drop/tickets/' . $parent->tid);
    }
  }

  $meta_items = array(
    t('Project')  => $proj_name,
    t('Status')   => $status_field,
    t('Type')     => '<span class="droptickets-type droptickets-type--' . check_plain($ticket->type) . '">' . check_plain($type_label) . '</span>',
    t('Priority') => check_plain($priority_label),
    t('Owner')    => $owner_name,
    t('Assignee') => droptickets_assignee_label($ticket),
    t('Due')      => $due,
  );
  if ($parent_item) {
    $meta_items[t('Parent')] = $parent_item;
  }
  if ($tag_html) {
    $meta_items[t('Tags')] = $tag_html;
  }
  if (!empty($ticket->external_link)) {
    $meta_items[t('External link')] = l(t('View in JIRA'), $ticket->external_link, array(
      'attributes' => array('target' => '_blank', 'rel' => 'noopener'),
    ));
  }

  foreach ($meta_items as $meta_label => $meta_value) {
    $sidebar .= '<div class="droptickets-sidebar-item">';
    $sidebar .= '<div class="droptickets-sidebar-label">' . check_plain($meta_label) . '</div>';
    $sidebar .= '<div class="droptickets-sidebar-value">' . $meta_value . '</div>';
    $sidebar .= '</div>';
  }

  // Sub-tickets section (shown when this ticket has children).
  $children = db_select('drop_ticket', 'c')
    ->fields('c', array('tid', 'title', 'status', 'pid', 'ticket_number'))
    ->condition('c.parent_tid', $ticket->tid)
    ->orderBy('c.priority', 'DESC')
    ->orderBy('c.tid', 'ASC')
    ->execute()->fetchAll();

  if (!empty($children)) {
    $child_pids      = array_unique(array_filter(array_column($children, 'pid')));
    $child_projects  = $child_pids ? entity_load('drop_project', $child_pids) : array();
    $done_statuses   = array('done', 'closed');
    $done_count      = count(array_filter($children, function($c) use ($done_statuses) {
      return in_array($c->status, $done_statuses);
    }));
    $total_count     = count($children);
    $progress_pct    = $total_count ? round($done_count / $total_count * 100) : 0;

    $sidebar .= '<div class="droptickets-sidebar-section-label">'
      . t('Sub-tickets')
      . ' <span class="droptickets-children-progress">' . $done_count . '/' . $total_count . '</span>'
      . '</div>';
    $sidebar .= '<div class="droptickets-children-bar"><div class="droptickets-children-bar-fill" style="width:' . $progress_pct . '%"></div></div>';
    $sidebar .= '<ul class="droptickets-children-list">';
    foreach ($children as $child) {
      $child_proj  = isset($child_projects[$child->pid]) ? $child_projects[$child->pid] : NULL;
      $child_label = droptickets_ticket_label($child, $child_proj);
      $is_done     = in_array($child->status, $done_statuses);
      $sidebar .= '<li class="droptickets-child-item' . ($is_done ? ' droptickets-child-item--done' : '') . '">'
        . l($child_label . ': ' . $child->title, 'admin/drop/tickets/' . $child->tid)
        . '</li>';
    }
    $sidebar .= '</ul>';
    if (droptickets_ticket_access('create')) {
      $sidebar .= '<div class="droptickets-child-add">'
        . l(t('+ Add sub-ticket'), 'admin/drop/tickets/add', array('query' => array('parent_tid' => $ticket->tid)))
        . '</div>';
    }
  }

  // Watchers section.
  module_load_include('inc', 'droptickets', 'droptickets.admin');
  $watchers_form = backdrop_get_form('droptickets_watchers_form', $ticket);
  $watchers_html = backdrop_render($watchers_form);
  $sidebar .= '<div class="droptickets-sidebar-watchers">';
  $sidebar .= '<div class="droptickets-sidebar-section-label">' . t('Watchers') . '</div>';
  $sidebar .= $watchers_html;
  $sidebar .= '</div>';

  $sidebar .= '</div>';

  // Pre-load all reactions for this ticket (ticket-level + all comments).
  $reactions_enabled  = config_get('droptickets.settings', 'reactions_enabled');
  $all_reactions      = array();
  $reaction_accounts  = array();
  if ($reactions_enabled) {
    $reaction_rows = db_select('drop_ticket_reaction', 'r')
      ->fields('r', array('uid', 'tcid', 'emoji'))
      ->condition('r.tid', $ticket->tid)
      ->execute()->fetchAll();
    $empty_set = array_fill_keys(array_keys(droptickets_reaction_emojis()), array());
    $all_reactions[0] = $empty_set;
    $reaction_uids = array();
    foreach ($reaction_rows as $rr) {
      $k = (int) $rr->tcid;
      if (!isset($all_reactions[$k])) {
        $all_reactions[$k] = $empty_set;
      }
      if (isset($all_reactions[$k][$rr->emoji])) {
        $all_reactions[$k][$rr->emoji][] = (int) $rr->uid;
        $reaction_uids[] = (int) $rr->uid;
      }
    }
    // Bulk-load reactor accounts for tooltips.
    $reaction_uids = array_unique($reaction_uids);
    if ($reaction_uids) {
      $reaction_accounts = user_load_multiple($reaction_uids);
    }
  }

  // Build main content.
  $main = '<div class="droptickets-view-main">';

  // Body.
  if (!empty($ticket->body)) {
    $main .= '<div class="droptickets-card droptickets-body droptickets-body--collapsible">';
    $main .= '<div class="droptickets-body-inner">' . check_markup($ticket->body, $ticket->body_format) . '<div class="droptickets-body-fade"></div></div>';
    $main .= '<button class="droptickets-body-toggle" aria-expanded="false">' . t('Expand') . '</button>';
    if ($reactions_enabled) {
      $main .= droptickets_render_reactions($ticket->tid, 0, $all_reactions[0], $reaction_accounts);
    }
    $main .= '</div>';
  }

  // Field API fields (rendered separately and appended below).
  // field_attach_prepare_view() must be called first so taxonomy formatters
  // have $item['taxonomy_term'] populated before field_attach_view() runs.
  // field_drop_tags is removed — tags are in the sidebar.
  // field_ticket_files is removed — rendered inline below the body.
  field_attach_prepare_view('drop_ticket', array($ticket->tid => $ticket), 'full');
  $field_output = field_attach_view('drop_ticket', $ticket, 'full');
  unset($field_output['field_drop_tags']);

  // Render file attachments inline, right after the body card.
  if (!empty($field_output['field_ticket_files'])) {
    $attachments_html = backdrop_render($field_output['field_ticket_files']);
    unset($field_output['field_ticket_files']);

    // Append a small "Download" link for image attachments (admins only).
    // Links to the resize/download page where original download is also available.
    $resize_links = '';
    if (user_access('administer droptickets')) {
      $file_items = field_get_items('drop_ticket', $ticket, 'field_ticket_files');
      if ($file_items) {
        $back_param = array('query' => array('back' => current_path()));
        foreach ($file_items as $item) {
          $f = file_load($item['fid']);
          if ($f && strpos($f->filemime, 'image/') === 0) {
            $resize_links .= '<div class="droptickets-attachment-resize">'
              . l(t('Crop'), 'admin/files/' . $f->fid . '/crop', $back_param)
              . ' &middot; '
              . l(t('Resize'), 'admin/files/' . $f->fid . '/resize', $back_param)
              . ' — ' . check_plain($f->filename)
              . '</div>';
          }
        }
      }
    }

    $main .= '<div class="droptickets-card droptickets-attachments">'
      . $attachments_html
      . $resize_links
      . '</div>';
  }
  $has_fields = element_children($field_output);

  // Related tickets.
  $related = droptickets_get_related($ticket->tid);
  $main .= '<div class="droptickets-card droptickets-related">';
  $main .= '<h3 class="droptickets-section-heading">' . t('Related Tickets') . '</h3>';

  if ($related) {
    $related_tids    = array_map(function($r) { return $r->related_tid; }, $related);
    $related_tickets = entity_load('drop_ticket', $related_tids);
    $related_pids    = array_unique(array_map(function($t) { return $t->pid; }, $related_tickets));
    $related_projects = entity_load('drop_project', $related_pids);

    $main .= '<table class="droptickets-related-list">';
    $main .= '<thead><tr>';
    $main .= '<th>' . t('Ticket') . '</th><th>' . t('Title') . '</th><th>' . t('Relationship') . '</th><th>' . t('Status') . '</th><th>' . t('Actions') . '</th>';
    $main .= '</tr></thead><tbody>';

    foreach ($related as $rel) {
      if (!isset($related_tickets[$rel->related_tid])) { continue; }
      $rt      = $related_tickets[$rel->related_tid];
      $rp      = isset($related_projects[$rt->pid]) ? $related_projects[$rt->pid] : NULL;
      $rlabel  = droptickets_ticket_label($rt, $rp);
      $rstatus = isset($status_options[$rt->status]) ? $status_options[$rt->status] : $rt->status;
      $main .= '<tr>';
      $main .= '<td><span class="droptickets-ticket-id">' . check_plain($rlabel) . '</span></td>';
      $main .= '<td>' . l($rt->title, 'admin/drop/tickets/' . $rt->tid) . '</td>';
      $main .= '<td><span class="droptickets-rel-type">' . check_plain($rel->rel_label) . '</span></td>';
      $main .= '<td><span class="droptickets-status droptickets-status--' . check_plain($rt->status) . '">' . check_plain($rstatus) . '</span></td>';
      $main .= '<td>' . l(t('Remove'), 'admin/drop/tickets/' . $ticket->tid . '/unrelate/' . $rel->trid) . '</td>';
      $main .= '</tr>';
    }
    $main .= '</tbody></table>';
  }
  else {
    $main .= '<p class="droptickets-empty">' . t('No related tickets.') . '</p>';
  }

  if (droptickets_ticket_access('update', $ticket)) {
    $main .= '<p class="droptickets-related-add">' . l(t('+ Add relation'), 'admin/drop/tickets/' . $ticket->tid . '/relate') . '</p>';
  }
  $main .= '</div>';

  // Comments.
  $comment_order = config_get('droptickets.settings', 'comment_order') ?: 'asc';
  $comments = db_select('drop_ticket_comment', 'c')
    ->fields('c')
    ->condition('c.tid', $ticket->tid)
    ->orderBy('c.created', strtoupper($comment_order))
    ->execute()
    ->fetchAll();

  $comment_uids     = array_unique(array_filter(array_column((array) $comments, 'uid')));
  $comment_accounts = $comment_uids ? user_load_multiple($comment_uids) : array();

  $main .= '<h3 class="droptickets-section-heading">' . t('Comments (@count)', array('@count' => count($comments))) . '</h3>';

  if ($comments) {
    foreach ($comments as $comment) {
      $author = isset($comment_accounts[$comment->uid]) ? check_plain($comment_accounts[$comment->uid]->name) : t('Unknown');
      $date   = format_date($comment->created, 'custom', 'M j, Y g:i A');
      $main .= '<div class="droptickets-comment">';
      $main .= '<div class="droptickets-comment-meta"><strong>' . $author . '</strong> · ' . $date;
      if (droptickets_comment_access('update', (object) $comment)) {
        $main .= ' · ' . l(t('Edit'), 'admin/drop/tickets/' . $ticket->tid . '/comment/' . $comment->tcid . '/edit');
        $main .= ' · ' . l(t('Delete'), 'admin/drop/tickets/' . $ticket->tid . '/comment/' . $comment->tcid . '/delete');
      }
      $main .= '</div>';
      $main .= '<div class="droptickets-comment-body">' . check_markup($comment->body, $comment->body_format) . '</div>';
      if ($reactions_enabled) {
        $tcid_key = (int) $comment->tcid;
        $comment_reactions = isset($all_reactions[$tcid_key]) ? $all_reactions[$tcid_key] : array_fill_keys(array_keys(droptickets_reaction_emojis()), array());
        $main .= droptickets_render_reactions($ticket->tid, $tcid_key, $comment_reactions, $reaction_accounts);
      }
      $main .= '</div>';
    }
  }

  $main .= '</div>'; // close droptickets-view-main

  // Assemble output.
  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
      'js'  => array(
        backdrop_get_path('module', 'droptickets') . '/droptickets.js',
        array(
          'data' => array('droptickets' => array('reactionEmojis' => droptickets_reaction_emojis())),
          'type' => 'setting',
        ),
      ),
    ),
  );

  // Review Mode: show "Back to Review" bar when the user is in review mode.
  $review_mode = droptickets_review_mode_get($user->uid);
  if ($review_mode) {
    $back_query = array_filter(array(
      'pid'    => !empty($review_mode['pid']) ? $review_mode['pid'] : NULL,
      'tag'    => !empty($review_mode['tag']) ? $review_mode['tag'] : NULL,
      'status' => !empty($review_mode['status']) ? $review_mode['status'] : NULL,
    ));
    $back_url = url('admin/drop/tickets/review', $back_query ? array('query' => $back_query) : array());
    $exit_url  = url('admin/drop/tickets/review', array('query' => array('exit_mode' => 1) + $back_query));
    $output['review_bar'] = array(
      '#markup' => '<div class="droptickets-review-back-bar">'
        . l(
            icon('arrow-left', array('attributes' => array('width' => 14, 'height' => 14))) . ' ' . t('Back to Review'),
            'admin/drop/tickets/review',
            array('query' => $back_query ?: array(), 'html' => TRUE, 'attributes' => array('class' => array('droptickets-review-back-link')))
          )
        . '<span class="droptickets-review-back-mode">' . t('Review Mode') . '</span>'
        . '<a href="' . $exit_url . '" class="droptickets-review-back-exit">' . t('Exit') . '</a>'
        . '</div>',
      '#weight' => -10,
    );
  }

  $output['layout'] = array(
    '#markup' => '<div class="droptickets-view-layout">' . $main . $sidebar . '</div>',
    '#weight' => 0,
    '#prefix' => '<div class="droptickets-view-page">',
    '#suffix' => '</div>',
  );

  // Field API fields render after the layout (they have their own markup).
  if ($has_fields) {
    $output['fields'] = $field_output;
    $output['fields']['#weight'] = 5;
  }

  // Comment form.
  if (user_access('access droptickets')) {
    module_load_include('inc', 'droptickets', 'droptickets.admin');
    $output['comment_form'] = backdrop_get_form('droptickets_comment_form', $ticket, NULL);
    $output['comment_form']['#weight'] = 10;
  }

  return $output;
}

// ---------------------------------------------------------------------------
// Emoji reactions
// ---------------------------------------------------------------------------

/**
 * Renders the reaction button bar for a ticket or comment.
 *
 * @param int $tid
 * @param int $tcid  0 = ticket-level.
 * @param array $reactions  Pre-loaded reactions array from droptickets_reactions_get().
 * @return string HTML
 */
function droptickets_render_reactions($tid, $tcid, array $reactions, array $accounts = array()) {
  global $user;
  $emojis = droptickets_reaction_emojis();
  $html   = '<div class="droptickets-reactions" data-tid="' . $tid . '" data-tcid="' . $tcid . '">';
  foreach ($reactions as $code => $uids) {
    $char   = isset($emojis[$code]) ? $emojis[$code] : $code;
    $count  = count($uids);
    $active = in_array((int) $user->uid, $uids) ? ' is-active' : '';
    $label  = $count > 0 ? $char . ' ' . $count : $char;

    // Build tooltip: "You · Rajib · Tim"
    $names  = array();
    foreach ($uids as $uid) {
      if ($uid == $user->uid) {
        $names[] = t('You');
      }
      elseif (isset($accounts[$uid])) {
        $names[] = check_plain($accounts[$uid]->name);
      }
    }
    $title = $names ? $char . ' ' . implode(' · ', $names) : $char;

    $html .= '<button class="react-btn' . $active . '" data-emoji="' . check_plain($code) . '" title="' . check_plain($title) . '">' . $label . '</button>';
  }
  $html .= '</div>';
  return $html;
}

/**
 * Page callback: toggle an emoji reaction (AJAX).
 *
 * Route: admin/drop/tickets/react/%/% (entity_type, entity_id)
 * POST: emoji
 */
function droptickets_react_ajax($entity_type, $entity_id) {
  if (!config_get('droptickets.settings', 'reactions_enabled')) {
    backdrop_json_output(array('error' => 'disabled'));
    return;
  }

  global $user;
  $entity_id = (int) $entity_id;
  $emoji     = isset($_POST['emoji']) ? trim($_POST['emoji']) : '';

  if (!array_key_exists($emoji, droptickets_reaction_emojis())) {
    backdrop_json_output(array('error' => 'invalid emoji'));
    return;
  }

  if ($entity_type === 'ticket') {
    $tid  = $entity_id;
    $tcid = 0;
  }
  elseif ($entity_type === 'comment') {
    $comment = db_select('drop_ticket_comment', 'c')
      ->fields('c', array('tid', 'tcid', 'uid'))
      ->condition('c.tcid', $entity_id)
      ->execute()->fetchObject();
    if (!$comment) {
      backdrop_json_output(array('error' => 'not found'));
      return;
    }
    $tid  = (int) $comment->tid;
    $tcid = (int) $comment->tcid;
  }
  else {
    backdrop_json_output(array('error' => 'unknown type'));
    return;
  }

  // Check whether this reaction already exists.
  $exists = db_select('drop_ticket_reaction', 'r')
    ->condition('r.uid', $user->uid)
    ->condition('r.tid', $tid)
    ->condition('r.tcid', $tcid)
    ->condition('r.emoji', $emoji)
    ->countQuery()->execute()->fetchField();

  if ($exists) {
    // Toggle off.
    db_delete('drop_ticket_reaction')
      ->condition('uid', $user->uid)
      ->condition('tid', $tid)
      ->condition('tcid', $tcid)
      ->condition('emoji', $emoji)
      ->execute();
  }
  else {
    // Toggle on.
    db_insert('drop_ticket_reaction')
      ->fields(array(
        'tid'     => $tid,
        'tcid'    => $tcid,
        'uid'     => $user->uid,
        'emoji'   => $emoji,
        'created' => REQUEST_TIME,
      ))
      ->execute();

    // Notify the author (throttled: max one 'reacted' notification per
    // reactor per ticket per 5 minutes, regardless of how many reactions).
    $notify_uid = NULL;
    if ($entity_type === 'ticket') {
      $items  = entity_load('drop_ticket', array($tid));
      $t      = $items ? reset($items) : NULL;
      if ($t && $t->uid != $user->uid) {
        $notify_uid = (int) $t->uid;
      }
    }
    else {
      if ($comment->uid && $comment->uid != $user->uid) {
        $notify_uid = (int) $comment->uid;
      }
    }
    if ($notify_uid) {
      $recent = db_select('drop_notification', 'n')
        ->condition('n.uid', $notify_uid)
        ->condition('n.actor_uid', $user->uid)
        ->condition('n.tid', $tid)
        ->condition('n.type', 'reacted')
        ->condition('n.created', REQUEST_TIME - 300, '>=')
        ->countQuery()->execute()->fetchField();
      if (!$recent) {
        droptickets_notify_create($notify_uid, $tid, $tcid, 'reacted', $user->uid);
      }
    }
  }

  // Return fresh reaction counts and reactor names for tooltip updates.
  $reactions = droptickets_reactions_get($tid, $tcid);
  $all_uids  = array();
  foreach ($reactions as $uids) {
    $all_uids = array_merge($all_uids, $uids);
  }
  $accounts = $all_uids ? user_load_multiple(array_unique($all_uids)) : array();

  $result = array();
  foreach ($reactions as $code => $uids) {
    $names = array();
    foreach ($uids as $uid) {
      if ($uid == $user->uid) {
        $names[] = t('You');
      }
      elseif (isset($accounts[$uid])) {
        $names[] = check_plain($accounts[$uid]->name);
      }
    }
    $result[$code] = array(
      'count'       => count($uids),
      'user_reacted' => in_array((int) $user->uid, $uids),
      'names'       => $names,
    );
  }
  backdrop_json_output(array('reactions' => $result));
}

// ---------------------------------------------------------------------------
// Mark done
// ---------------------------------------------------------------------------

/**
 * Page callback: mark a ticket as done (for DropBridge).
 */
function droptickets_ticket_mark_done(DropTicket $ticket) {
  $ticket->status = 'done';
  $ticket->save();
  backdrop_set_message(t('Ticket %label marked as done.', array('%label' => droptickets_ticket_label($ticket))));
  backdrop_goto('admin/drop');
}

/**
 * Page callback: manually close a ticket.
 */
function droptickets_ticket_close(DropTicket $ticket) {
  $ticket->status = 'closed';
  $ticket->save();
  backdrop_set_message(t('Ticket %label has been closed.', array('%label' => droptickets_ticket_label($ticket))));
  backdrop_goto('admin/drop/tickets/' . $ticket->tid);
}

// ---------------------------------------------------------------------------
// Notifications page
// ---------------------------------------------------------------------------

/**
 * Page callback: list of notifications for the current user.
 */
function droptickets_notifications_page() {
  global $user;

  backdrop_set_title(t('Notifications'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  $notifications = db_select('drop_notification', 'n')
    ->fields('n')
    ->condition('n.uid', $user->uid)
    ->orderBy('n.created', 'DESC')
    ->extend('PagerDefault')
    ->limit(50)
    ->execute()
    ->fetchAll();

  if (empty($notifications)) {
    $output['empty'] = array(
      '#markup' => '<p class="droptickets-empty">' . t('No notifications yet.') . '</p>',
    );
    return $output;
  }

  // Show "Mark all as read" link when there are unread notifications.
  $unread_count = droptickets_unread_count($user->uid);
  if ($unread_count > 0) {
    $token = backdrop_get_token('mark-all-read');
    $output['mark_read'] = array(
      '#markup' => l(
        t('Mark all as read'),
        'admin/drop/notifications/mark-all-read',
        array('query' => array('token' => $token))
      ),
      '#prefix' => '<p class="droptickets-mark-all-read">',
      '#suffix' => '</p>',
    );
  }

  // Bulk-load tickets and actors.
  $tids = array_unique(array_column((array) $notifications, 'tid'));
  $tickets = entity_load('drop_ticket', $tids);

  $pids = array_filter(array_unique(array_map(function($t) { return $t->pid; }, $tickets)));
  $projects = $pids ? entity_load('drop_project', $pids) : array();

  $actor_uids = array_unique(array_column((array) $notifications, 'actor_uid'));
  $actors = $actor_uids ? user_load_multiple($actor_uids) : array();

  $status_options = droptickets_status_options();
  $rows = array();
  $unread_nids = array();

  foreach ($notifications as $n) {
    if (!isset($tickets[$n->tid])) {
      continue;
    }
    $ticket  = $tickets[$n->tid];
    $project = isset($projects[$ticket->pid]) ? $projects[$ticket->pid] : NULL;
    $label   = droptickets_ticket_label($ticket, $project);
    $actor   = isset($actors[$n->actor_uid]) ? check_plain($actors[$n->actor_uid]->name) : t('Someone');
    $date    = format_date($n->created, 'custom', 'M j, Y g:i A');

    $type_messages = array(
      'assigned'  => t('@actor assigned this ticket to you', array('@actor' => $actor)),
      'commented' => t('@actor added a comment', array('@actor' => $actor)),
      'updated'   => t('@actor updated this ticket', array('@actor' => $actor)),
      'created'   => t('@actor created this ticket', array('@actor' => $actor)),
      'reacted'   => t('@actor reacted to a comment on this ticket', array('@actor' => $actor)),
      'mentioned' => t('@actor mentioned you in a comment', array('@actor' => $actor)),
    );
    $message = isset($type_messages[$n->type]) ? $type_messages[$n->type] : check_plain($n->type);

    $status_label = isset($status_options[$ticket->status]) ? $status_options[$ticket->status] : $ticket->status;

    $dot = !$n->is_read
      ? '<span class="drop-notif-dot" title="' . t('Unread') . '">●</span>'
      : '<span class="drop-notif-dot drop-notif-dot--read">●</span>';

    $rows[] = array(
      'data' => array(
        array('data' => $dot, 'class' => array('drop-notif-col-dot')),
        array('data' => '<span class="droptickets-ticket-id">' . check_plain($label) . '</span>'),
        array('data' => l($ticket->title, 'admin/drop/tickets/' . $ticket->tid)),
        array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($ticket->status) . '">' . check_plain($status_label) . '</span>'),
        array('data' => $message),
        array('data' => $date),
      ),
      'class' => array($n->is_read ? 'drop-notif-read' : 'drop-notif-unread'),
    );
  }

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => array('', t('Ticket'), t('Title'), t('Status'), t('Activity'), t('Date')),
    '#rows'       => $rows,
    '#attributes' => array('class' => array('drop-notifications-list')),
  );

  $output['pager'] = array('#theme' => 'pager', '#weight' => 10);

  return $output;
}

// ---------------------------------------------------------------------------
// Admin notification log
// ---------------------------------------------------------------------------

/**
 * Page callback: admin view of all notifications across all users.
 */
function droptickets_notification_log_page() {
  backdrop_set_title(t('Notification log'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  $total = db_select('drop_notification', 'n')->countQuery()->execute()->fetchField();
  $output['summary'] = array(
    '#markup' => '<p>' . t('@count total notification records.', array('@count' => $total)) . '</p>',
  );

  $notifications = db_select('drop_notification', 'n')
    ->fields('n')
    ->orderBy('n.created', 'DESC')
    ->extend('PagerDefault')
    ->limit(50)
    ->execute()
    ->fetchAll();

  if (empty($notifications)) {
    $output['empty'] = array(
      '#markup' => '<p class="droptickets-empty">' . t('No notifications in the log.') . '</p>',
    );
    return $output;
  }

  // Bulk-load related records.
  $tids = array_unique(array_column((array) $notifications, 'tid'));
  $tickets = entity_load('drop_ticket', $tids);

  $pids = array_filter(array_unique(array_map(function($t) { return $t->pid; }, $tickets)));
  $projects = $pids ? entity_load('drop_project', $pids) : array();

  $all_uids = array_unique(array_merge(
    array_column((array) $notifications, 'uid'),
    array_column((array) $notifications, 'actor_uid')
  ));
  $accounts = $all_uids ? user_load_multiple($all_uids) : array();

  $rows = array();
  foreach ($notifications as $n) {
    $ticket    = isset($tickets[$n->tid]) ? $tickets[$n->tid] : NULL;
    $project   = ($ticket && isset($projects[$ticket->pid])) ? $projects[$ticket->pid] : NULL;
    $label     = $ticket ? droptickets_ticket_label($ticket, $project) : '#' . $n->tid;
    $recipient = isset($accounts[$n->uid]) ? check_plain($accounts[$n->uid]->name) : 'uid:' . $n->uid;
    $actor     = isset($accounts[$n->actor_uid]) ? check_plain($accounts[$n->actor_uid]->name) : 'uid:' . $n->actor_uid;
    $date      = format_date($n->created, 'custom', 'M j, Y g:i A');
    $ticket_link = $ticket
      ? l($label, 'admin/drop/tickets/' . $n->tid)
      : check_plain($label);

    $rows[] = array(
      'data' => array(
        array('data' => $n->nid),
        array('data' => $ticket_link),
        array('data' => $recipient),
        array('data' => $actor),
        array('data' => check_plain($n->type)),
        array('data' => $n->is_read ? t('Read') : '<strong>' . t('Unread') . '</strong>'),
        array('data' => $date),
      ),
      'class' => array($n->is_read ? 'drop-notif-read' : 'drop-notif-unread'),
    );
  }

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => array(t('ID'), t('Ticket'), t('Recipient'), t('Actor'), t('Type'), t('Read'), t('Date')),
    '#rows'       => $rows,
    '#attributes' => array('class' => array('drop-notifications-list')),
  );

  $output['pager'] = array('#theme' => 'pager', '#weight' => 10);

  return $output;
}

// ---------------------------------------------------------------------------
// Flagged tickets
// ---------------------------------------------------------------------------

/**
 * Page callback: toggle a ticket's flagged state and redirect back.
 */
function droptickets_ticket_flag_toggle(DropTicket $ticket) {
  $is_flagged = db_select('drop_ticket_flag', 'f')
    ->condition('tid', $ticket->tid)
    ->countQuery()->execute()->fetchField();

  if ($is_flagged) {
    db_delete('drop_ticket_flag')->condition('tid', $ticket->tid)->execute();
  }
  else {
    // Soft limit warning.
    $count = db_select('drop_ticket_flag', 'f')->countQuery()->execute()->fetchField();
    $soft_limit = (int) config_get('droptickets.settings', 'flag_soft_limit');
    if ($soft_limit > 0 && $count >= $soft_limit) {
      backdrop_set_message(t('You now have more than @limit flagged tickets. Consider clearing some to keep the list focused.', array('@limit' => $soft_limit)), 'warning');
    }
    db_insert('drop_ticket_flag')
      ->fields(array('tid' => $ticket->tid, 'created' => REQUEST_TIME))
      ->execute();
  }

  $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/drop/tickets/flagged';
  backdrop_goto($destination);
}

/**
 * Page callback: show all currently flagged tickets.
 */
// ---------------------------------------------------------------------------
// Recent activity
// ---------------------------------------------------------------------------

/**
 * Page callback: tickets sorted by most recent edit or comment.
 */
function droptickets_recent_page() {
  backdrop_set_title(t('Recent tickets'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  // Join with the most recent comment per ticket, sort by whichever is later.
  $query = db_select('drop_ticket', 't');
  $query->fields('t', array('tid', 'title', 'status', 'pid', 'assignee_uid', 'owner_uid', 'priority', 'type'));
  $query->addExpression('t.changed', 'ticket_changed');
  $last_comment_subquery = db_select('drop_ticket_comment', 'c');
  $last_comment_subquery->fields('c', array('tid'));
  $last_comment_subquery->addExpression('MAX(c.created)', 'last_comment');
  $last_comment_subquery->groupBy('c.tid');
  $query->leftJoin($last_comment_subquery, 'lc', 'lc.tid = t.tid');
  $query->addExpression('GREATEST(t.changed, COALESCE(lc.last_comment, 0))', 'last_activity');
  $query->addExpression('lc.last_comment', 'last_comment_ts');
  $query->isNull('t.import_id');
  $query->orderBy('last_activity', 'DESC');
  $query->range(0, 50);

  $results = $query->execute()->fetchAll();

  if (empty($results)) {
    $output['empty'] = array('#markup' => '<p>' . t('No tickets found.') . '</p>');
    return $output;
  }

  // Load supporting data.
  $tids     = array_column((array) $results, 'tid');
  $pids     = array_filter(array_unique(array_column((array) $results, 'pid')));
  $uids     = array_filter(array_unique(array_merge(
    array_column((array) $results, 'assignee_uid'),
    array_column((array) $results, 'owner_uid')
  )));
  $tickets  = entity_load('drop_ticket', $tids);
  $projects = $pids ? entity_load('drop_project', $pids) : array();
  $accounts = $uids ? user_load_multiple($uids) : array();

  // Get the uid of the most recent commenter per ticket.
  $max_comment_sub = db_select('drop_ticket_comment', 'ci')
    ->fields('ci', array('tid'))
    ->condition('ci.tid', $tids, 'IN');
  $max_comment_sub->addExpression('MAX(ci.created)', 'mc');
  $max_comment_sub->groupBy('ci.tid');
  $recent_comment_query = db_select('drop_ticket_comment', 'co');
  $recent_comment_query->fields('co', array('tid', 'uid', 'created'));
  $recent_comment_query->join($max_comment_sub, 'mx', 'co.tid = mx.tid AND co.created = mx.mc');
  $recent_comment_query->condition('co.tid', $tids, 'IN');
  $recent_commenters = $recent_comment_query->execute()->fetchAllAssoc('tid');

  // Collect commenter uids to load.
  $commenter_uids = array_filter(array_unique(array_map(function($c) { return $c->uid; }, (array) $recent_commenters)));
  $commenter_accounts = $commenter_uids ? user_load_multiple($commenter_uids) : array();
  $accounts = $accounts + $commenter_accounts;

  $status_options   = droptickets_status_options();
  $priority_labels  = drop_base_priority_labels();
  $flagged_tids     = db_select('drop_ticket_flag', 'f')->fields('f', array('tid'))->execute()->fetchCol();
  $flagged_tids     = array_flip($flagged_tids);

  $rows = array();
  foreach ($results as $row) {
    $ticket = isset($tickets[$row->tid]) ? $tickets[$row->tid] : NULL;
    if (!$ticket) { continue; }
    $project  = ($row->pid && isset($projects[$row->pid])) ? $projects[$row->pid] : NULL;
    $label    = droptickets_ticket_label($ticket, $project);

    $proj_name = '—';
    if ($project) {
      $proj_name = module_exists('dropcrm') && dropcrm_project_access('view', $project)
        ? l($project->name, 'admin/drop/crm/projects/' . $project->pid)
        : check_plain($project->name);
    }

    $assignee = droptickets_assignee_label($ticket, $accounts);
    $status_label = isset($status_options[$row->status]) ? $status_options[$row->status] : $row->status;
    $priority_class = 'drop-priority-' . drop_base_priority_class($ticket->priority);
    $priority_dot = '<span class="droptickets-priority-dot ' . $priority_class . '" title="' . check_plain(isset($priority_labels[$ticket->priority]) ? $priority_labels[$ticket->priority] : '') . '"></span>';
    $flag_icon = isset($flagged_tids[$row->tid])
      ? icon('flag-fill', array('attributes' => array('width' => 16, 'height' => 16, 'class' => array('droptickets-flag-on'))))
      : '';

    // Determine who made the most recent activity.
    $activity_by = '—';
    if (!empty($row->last_comment_ts) && $row->last_comment_ts >= $row->ticket_changed) {
      // Most recent activity was a comment.
      if (isset($recent_commenters[$row->tid])) {
        $cuid = $recent_commenters[$row->tid]->uid;
        $activity_by = isset($accounts[$cuid]) ? check_plain($accounts[$cuid]->name) : '—';
      }
    }

    $rows[] = array(
      'data' => array(
        array('data' => $flag_icon,       'class' => array('droptickets-col-flag')),
        array('data' => '<span class="droptickets-ticket-id">' . check_plain($label) . '</span>', 'class' => array('droptickets-col-id')),
        array('data' => $proj_name,       'class' => array('droptickets-col-project')),
        array('data' => l($row->title, 'admin/drop/tickets/' . $row->tid), 'class' => array('droptickets-col-title')),
        array('data' => $priority_dot,    'class' => array('droptickets-col-priority')),
        array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($row->status) . '">' . check_plain($status_label) . '</span>'),
        array('data' => $assignee),
        array('data' => format_date($row->last_activity, 'custom', 'M j, Y g:ia')),
        array('data' => $activity_by),
      ),
      'class' => array($priority_class),
    );
  }

  $output['table'] = array(
    '#theme'  => 'table',
    '#header' => array('', t('ID'), t('Project'), t('Title'), t('P'), t('Status'), t('Assignee'), t('Last activity'), t('By')),
    '#rows'   => $rows,
    '#attributes' => array('class' => array('droptickets-list')),
  );

  return $output;
}

// ---------------------------------------------------------------------------
// Flagged tickets
// ---------------------------------------------------------------------------

function droptickets_flagged_page() {
  backdrop_set_title(t('Flagged tickets'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  $flags = db_select('drop_ticket_flag', 'f')
    ->fields('f')
    ->orderBy('f.created', 'ASC')
    ->execute()->fetchAll();

  $count = count($flags);

  $soft_limit = (int) config_get('droptickets.settings', 'flag_soft_limit');
  if ($soft_limit > 0 && $count > $soft_limit) {
    $output['warning'] = array(
      '#markup' => '<p class="messages warning">' . t('You have @count flagged tickets. Try to keep it to @limit or fewer.', array('@count' => $count, '@limit' => $soft_limit)) . '</p>',
    );
  }

  if (empty($flags)) {
    $output['empty'] = array(
      '#markup' => '<p class="droptickets-empty">' . t('No tickets are currently flagged. Click the flag icon on any ticket to add it here.') . '</p>',
    );
    return $output;
  }

  if (user_access('administer droptickets')) {
    $output['clear'] = array(
      '#markup' => '<p>' . l(t('Clear all flags'), 'admin/drop/tickets/flags/clear') . '</p>',
      '#weight' => -5,
    );
  }

  $tids     = array_column((array) $flags, 'tid');
  $tickets  = entity_load('drop_ticket', $tids);

  // Preserve flag order (oldest first = order they were added).
  $ordered = array();
  foreach ($tids as $tid) {
    if (isset($tickets[$tid])) {
      $ordered[] = $tickets[$tid];
    }
  }

  $pids          = array_unique(array_map(function($t) { return $t->pid; }, $ordered));
  $projects      = entity_load('drop_project', $pids);
  $assignee_uids = array_filter(array_unique(array_map(function($t) { return $t->assignee_uid; }, $ordered)));
  $owner_uids    = array_filter(array_unique(array_map(function($t) { return $t->owner_uid; }, $ordered)));
  $all_uids      = array_unique(array_merge($assignee_uids, $owner_uids));
  $accounts      = $all_uids ? user_load_multiple($all_uids) : array();

  $type_options   = droptickets_type_options();
  $status_options = droptickets_status_options();
  $can_flag       = user_access('flag droptickets');

  $rows = array();
  $i    = 1;
  foreach ($ordered as $ticket) {
    $project   = isset($projects[$ticket->pid]) ? $projects[$ticket->pid] : NULL;
    $label     = droptickets_ticket_label($ticket, $project);
    $proj_name = $project ? l($project->name, 'admin/drop/crm/projects/' . $project->pid) : '—';
    $assignee  = droptickets_assignee_label($ticket, $accounts);
    $owner     = (!empty($ticket->owner_uid) && isset($accounts[$ticket->owner_uid]))
      ? check_plain($accounts[$ticket->owner_uid]->name) : '—';
    $type_label   = isset($type_options[$ticket->type]) ? $type_options[$ticket->type] : $ticket->type;
    $status_label = isset($status_options[$ticket->status]) ? $status_options[$ticket->status] : $ticket->status;

    $unflag = $can_flag ? ' ' . l(
      icon('flag-fill', array('attributes' => array('width' => 16, 'height' => 16))),
      'admin/drop/tickets/' . $ticket->tid . '/flag',
      array('html' => TRUE, 'query' => array('destination' => 'admin/drop/tickets/flagged'), 'attributes' => array('class' => array('droptickets-flag-on'), 'title' => t('Remove flag')))
    ) : '';

    $rows[] = array(
      'data' => array(
        array('data' => '<strong>' . $i . '</strong>', 'class' => array('droptickets-flag-rank')),
        array('data' => '<span class="droptickets-ticket-id">' . check_plain($label) . '</span>'),
        array('data' => $proj_name),
        array('data' => l($ticket->title, 'admin/drop/tickets/' . $ticket->tid) . $unflag, 'class' => array('droptickets-col-title')),
        array('data' => '<span class="droptickets-type droptickets-type--' . check_plain($ticket->type) . '">' . check_plain($type_label) . '</span>'),
        array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($ticket->status) . '">' . check_plain($status_label) . '</span>'),
        array('data' => $owner),
        array('data' => $assignee),
      ),
    );
    $i++;
  }

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => array(t('#'), t('ID'), t('Project'), t('Title'), t('Type'), t('Status'), t('Owner'), t('Assignee')),
    '#rows'       => $rows,
    '#attributes' => array('class' => array('droptickets-list', 'droptickets-flagged-list')),
  );

  return $output;
}

/**
 * Confirm form: clear all flags.
 */
function droptickets_flags_clear_form($form, &$form_state) {
  return confirm_form(
    $form,
    t('Clear all flagged tickets?'),
    'admin/drop/tickets/flagged',
    t('This will remove all flags. Your team will no longer see any priority tickets until you flag new ones.'),
    t('Clear all flags'),
    t('Cancel')
  );
}

/**
 * Submit handler: delete all flags.
 */
function droptickets_flags_clear_form_submit($form, &$form_state) {
  db_delete('drop_ticket_flag')->execute();
  backdrop_set_message(t('All flags cleared.'));
  $form_state['redirect'] = 'admin/drop/tickets/flagged';
}

// ---------------------------------------------------------------------------
// Activate imported ticket
// ---------------------------------------------------------------------------

/**
 * Page callback: clear import_id on a ticket, moving it into the normal flow.
 */
function droptickets_ticket_activate(DropTicket $ticket) {
  if (empty($ticket->import_id)) {
    backdrop_set_message(t('This ticket is already active.'), 'warning');
    backdrop_goto('admin/drop/tickets/' . $ticket->tid);
  }

  $ticket->import_id = NULL;
  $ticket->save();

  backdrop_set_message(t('Ticket %label is now active and will appear in the normal ticket list.', array(
    '%label' => droptickets_ticket_label($ticket),
  )));
  backdrop_goto('admin/drop/tickets/' . $ticket->tid);
}

// ---------------------------------------------------------------------------
// Stale tickets report
// ---------------------------------------------------------------------------

/**
 * Page callback: Reports parent tab — delegates to stale report.
 */
function droptickets_reports_page() {
  return droptickets_stale_report_page();
}

/**
 * Page callback: tickets not done and not updated in N+ days, oldest first.
 */
function droptickets_stale_report_page() {
  $stale_days = (int) config_get('droptickets.settings', 'stale_days');
  if ($stale_days < 1) {
    $stale_days = 20;
  }

  backdrop_set_title(t('Stale tickets'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
    ),
  );

  $cutoff = REQUEST_TIME - ($stale_days * 86400);

  $tickets = db_select('drop_ticket', 't')
    ->fields('t')
    ->condition('t.status', 'done', '!=')
    ->condition('t.changed', $cutoff, '<')
    ->isNull('t.import_id')
    ->orderBy('t.changed', 'ASC')
    ->execute()
    ->fetchAll();

  $count = count($tickets);
  $output['summary'] = array(
    '#markup' => '<p>' . format_plural($count,
      '1 ticket has not been updated in more than @days day.',
      '@count tickets have not been updated in more than @days days.',
      array('@days' => $stale_days)
    ) . '</p>',
  );

  if (empty($tickets)) {
    return $output;
  }

  // Bulk-load projects and users.
  $pids          = array_unique(array_column((array) $tickets, 'pid'));
  $projects      = entity_load('drop_project', $pids);
  $assignee_uids = array_filter(array_unique(array_column((array) $tickets, 'assignee_uid')));
  $owner_uids    = array_filter(array_unique(array_column((array) $tickets, 'owner_uid')));
  $all_uids      = array_unique(array_merge($assignee_uids, $owner_uids));
  $accounts      = $all_uids ? user_load_multiple($all_uids) : array();

  $type_options   = droptickets_type_options();
  $status_options = droptickets_status_options();
  $now            = REQUEST_TIME;

  $rows = array();
  foreach ($tickets as $ticket) {
    $project   = isset($projects[$ticket->pid]) ? $projects[$ticket->pid] : NULL;
    $label     = droptickets_ticket_label($ticket, $project);
    $proj_name = $project ? l($project->name, 'admin/drop/crm/projects/' . $project->pid) : '—';
    $assignee  = droptickets_assignee_label($ticket, $accounts);
    $owner     = (!empty($ticket->owner_uid) && isset($accounts[$ticket->owner_uid]))
      ? check_plain($accounts[$ticket->owner_uid]->name) : '—';
    $type_label   = isset($type_options[$ticket->type]) ? $type_options[$ticket->type] : $ticket->type;
    $status_label = isset($status_options[$ticket->status]) ? $status_options[$ticket->status] : $ticket->status;

    $days_stale = (int) floor(($now - $ticket->changed) / 86400);
    $days_label = format_plural($days_stale, '1 day', '@count days');

    $rows[] = array(
      'data' => array(
        array('data' => '<span class="droptickets-ticket-id">' . check_plain($label) . '</span>'),
        array('data' => $proj_name),
        array('data' => l($ticket->title, 'admin/drop/tickets/' . $ticket->tid), 'class' => array('droptickets-col-title')),
        array('data' => '<span class="droptickets-type droptickets-type--' . check_plain($ticket->type) . '">' . check_plain($type_label) . '</span>'),
        array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($ticket->status) . '">' . check_plain($status_label) . '</span>'),
        array('data' => $owner),
        array('data' => $assignee),
        array('data' => $days_label),
      ),
    );
  }

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => array(
      t('ID'), t('Project'), t('Title'), t('Type'), t('Status'),
      t('Owner'), t('Assignee'), t('Last updated'),
    ),
    '#rows'       => $rows,
    '#attributes' => array('class' => array('droptickets-list')),
  );

  return $output;
}

// ---------------------------------------------------------------------------
// Review / triage
// ---------------------------------------------------------------------------

/**
 * Page callback: personal ticket review / triage queue.
 *
 * Shows all active tickets sorted unreviewed-first. Each row has an AJAX
 * toggle button the current user can click to mark a ticket as reviewed.
 * Reviewed tickets are shown below, visually dimmed, until flags are cleared.
 */
function droptickets_review_page() {
  global $user;

  backdrop_set_title(t('Review'));

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptickets') . '/droptickets.css'),
      'js'  => array(backdrop_get_path('module', 'droptickets') . '/droptickets.js'),
    ),
  );

  // --- Review mode + filter handling ---

  $review_mode = droptickets_review_mode_get($user->uid);

  // Parse filter params from URL.
  $active_status_keys = array_keys(droptickets_status_options());
  $pid_filter    = (!empty($_GET['pid']) && is_numeric($_GET['pid'])) ? (int) $_GET['pid'] : 0;
  $tag_filter    = (!empty($_GET['tag']) && is_numeric($_GET['tag'])) ? (int) $_GET['tag'] : 0;
  $status_filter = (isset($_GET['status']) && in_array($_GET['status'], $active_status_keys))
    ? $_GET['status'] : '';

  // Handle mode toggle actions (GET params trigger a state change + redirect).
  if (isset($_GET['enter_mode'])) {
    droptickets_review_mode_set($user->uid, $pid_filter, $tag_filter, $status_filter);
    $rq = array_filter(array('pid' => $pid_filter ?: NULL, 'tag' => $tag_filter ?: NULL, 'status' => $status_filter ?: NULL));
    backdrop_goto('admin/drop/tickets/review', array('query' => $rq));
    return;
  }
  if (isset($_GET['exit_mode'])) {
    droptickets_review_mode_clear($user->uid);
    $rq = array_filter(array('pid' => $pid_filter ?: NULL, 'tag' => $tag_filter ?: NULL, 'status' => $status_filter ?: NULL));
    backdrop_goto('admin/drop/tickets/review', array('query' => $rq));
    return;
  }

  // If in review mode and no URL filter params, load saved filter values.
  if ($review_mode && !isset($_GET['pid']) && !isset($_GET['tag']) && !isset($_GET['status'])) {
    $pid_filter    = $review_mode['pid'];
    $tag_filter    = $review_mode['tag'];
    $status_filter = isset($review_mode['status']) ? $review_mode['status'] : '';
  }
  // If in review mode and user changed filters via URL, save updated values.
  elseif ($review_mode && (isset($_GET['pid']) || isset($_GET['tag']) || isset($_GET['status']))) {
    droptickets_review_mode_set($user->uid, $pid_filter, $tag_filter, $status_filter);
    $review_mode = droptickets_review_mode_get($user->uid);
  }

  // Build project dropdown (all projects that have tickets).
  $all_project_pids = db_select('drop_ticket', 't')
    ->fields('t', array('pid'))
    ->distinct()
    ->execute()->fetchCol();
  $proj_opts_html = '<option value="0"' . (!$pid_filter ? ' selected' : '') . '>' . t('— All projects —') . '</option>';
  if (!empty($all_project_pids)) {
    $all_projects = entity_load('drop_project', $all_project_pids);
    uasort($all_projects, function($a, $b) { return strcasecmp($a->name, $b->name); });
    foreach ($all_projects as $proj) {
      $proj_opts_html .= '<option value="' . (int) $proj->pid . '"'
        . ($pid_filter === (int) $proj->pid ? ' selected' : '') . '>'
        . check_plain($proj->name) . '</option>';
    }
  }

  // Build tag dropdown.
  $tag_opts_html = '<option value="0"' . (!$tag_filter ? ' selected' : '') . '>' . t('— All tags —') . '</option>';
  $tags_vocab = taxonomy_vocabulary_load('drop_tags');
  if ($tags_vocab) {
    $all_terms = db_select('taxonomy_term_data', 'd')
      ->fields('d', array('tid', 'name'))
      ->condition('d.vocabulary', 'drop_tags')
      ->orderBy('d.name')
      ->execute()->fetchAllKeyed();
    foreach ($all_terms as $tid => $tname) {
      $tag_opts_html .= '<option value="' . (int) $tid . '"'
        . ($tag_filter === (int) $tid ? ' selected' : '') . '>'
        . check_plain($tname) . '</option>';
    }
  }

  // Build status dropdown.
  $status_opts_html = '<option value=""' . (!$status_filter ? ' selected' : '') . '>' . t('— All active —') . '</option>';
  foreach (droptickets_status_options() as $skey => $slabel) {
    $status_opts_html .= '<option value="' . check_plain($skey) . '"'
      . ($status_filter === $skey ? ' selected' : '') . '>'
      . check_plain($slabel) . '</option>';
  }

  // Mode button — Enter or Exit.
  $mode_query = array_filter(array(
    'pid'    => $pid_filter ?: NULL,
    'tag'    => $tag_filter ?: NULL,
    'status' => $status_filter ?: NULL,
  ));
  if ($review_mode) {
    $exit_url = url('admin/drop/tickets/review', array('query' => array('exit_mode' => 1) + $mode_query));
    $mode_btn = '<a href="' . $exit_url . '" class="droptickets-review-mode-btn droptickets-review-mode-btn--active">'
      . icon('x-circle', array('attributes' => array('width' => 14, 'height' => 14)))
      . ' ' . t('Exit Review Mode') . '</a>';
  }
  else {
    $enter_url = url('admin/drop/tickets/review', array('query' => array('enter_mode' => 1) + $mode_query));
    $mode_btn = '<a href="' . $enter_url . '" class="droptickets-review-mode-btn">'
      . icon('play-circle', array('attributes' => array('width' => 14, 'height' => 14)))
      . ' ' . t('Enter Review Mode') . '</a>';
  }

  // Filter form.
  $filter_form = '<form method="get" action="' . url('admin/drop/tickets/review') . '" class="droptickets-filter-form">'
    . '<select name="status" class="droptickets-filter-select" onchange="this.form.submit()">' . $status_opts_html . '</select>'
    . '<select name="pid" class="droptickets-filter-select" onchange="this.form.submit()">' . $proj_opts_html . '</select>'
    . '<select name="tag" class="droptickets-filter-select" onchange="this.form.submit()">' . $tag_opts_html . '</select>'
    . '</form>';

  $output['filters'] = array(
    '#markup' => '<div class="droptickets-filter-row">' . $filter_form . $mode_btn . '</div>',
    '#weight' => -20,
  );

  // Load active tickets, applying any filters.
  $query = db_select('drop_ticket', 't')
    ->fields('t', array('tid', 'title', 'status', 'pid', 'assignee_uid', 'priority'))
    ->orderBy('t.priority', 'DESC')
    ->orderBy('t.tid', 'ASC');

  if ($status_filter) {
    $query->condition('t.status', $status_filter);
  }
  else {
    $query->condition('t.status', droptickets_active_statuses(), 'IN');
  }
  if ($pid_filter) {
    $query->condition('t.pid', $pid_filter);
  }
  if ($tag_filter) {
    $query->join('field_data_field_drop_tags', 'ftag',
      'ftag.entity_id = t.tid AND ftag.entity_type = :et AND ftag.deleted = 0',
      array(':et' => 'drop_ticket'));
    $query->condition('ftag.field_drop_tags_tid', $tag_filter);
  }

  $tickets_raw = $query->execute()->fetchAll();

  if (empty($tickets_raw)) {
    $output['empty'] = array(
      '#markup' => '<p class="droptickets-empty">' . t('No active tickets to review.') . '</p>',
    );
    return $output;
  }

  $reviewed_tids = droptickets_review_get($user->uid);
  $reviewed_set  = array_flip($reviewed_tids);
  $reviewed_count = count($reviewed_tids);

  // Controls bar.
  $clear_link = $reviewed_count
    ? l(t('Clear @count reviewed flag(s)', array('@count' => $reviewed_count)),
        'admin/drop/tickets/review/clear',
        array('query' => array('token' => backdrop_get_token('review-clear'))))
    : '';
  $output['controls'] = array(
    '#markup' => '<div class="droptickets-review-controls">'
      . '<span class="droptickets-review-summary">'
      . format_plural(count($tickets_raw), '1 ticket', '@count tickets')
      . ($reviewed_count ? ', ' . format_plural($reviewed_count, '1 reviewed', '@count reviewed') : '')
      . '</span>'
      . ($clear_link ? ' &nbsp;·&nbsp; ' . $clear_link : '')
      . '</div>',
    '#weight' => -10,
  );

  // Bulk-load related data.
  $tids = array_column($tickets_raw, 'tid');
  $pids = array_filter(array_unique(array_column($tickets_raw, 'pid')));
  $auids = array_filter(array_unique(array_column($tickets_raw, 'assignee_uid')));
  $projects = $pids  ? entity_load('drop_project', $pids)  : array();
  $accounts = $auids ? user_load_multiple($auids) : array();
  $status_options = droptickets_status_options();

  // Index tickets by tid for easy lookup.
  $tickets_by_tid = array();
  foreach ($tickets_raw as $t) {
    $tickets_by_tid[$t->tid] = $t;
  }

  // Split into unreviewed (first) and reviewed (last).
  $unreviewed = array();
  $reviewed   = array();
  foreach ($tickets_raw as $t) {
    if (isset($reviewed_set[$t->tid])) {
      $reviewed[] = $t;
    }
    else {
      $unreviewed[] = $t;
    }
  }
  $ordered = array_merge($unreviewed, $reviewed);

  $token = backdrop_get_token('review-toggle');
  $rows  = array();
  foreach ($ordered as $t) {
    $is_reviewed = isset($reviewed_set[$t->tid]);
    $project     = isset($projects[$t->pid]) ? $projects[$t->pid] : NULL;
    $proj_name   = $project ? check_plain($project->name) : '—';
    $assignee    = (!empty($t->assignee_uid) && isset($accounts[$t->assignee_uid]))
      ? check_plain($accounts[$t->assignee_uid]->name) : '—';
    $status_label = isset($status_options[$t->status]) ? $status_options[$t->status] : $t->status;

    $btn_class  = 'droptickets-review-btn' . ($is_reviewed ? ' droptickets-review-btn--done' : '');
    $btn_title  = $is_reviewed ? t('Reviewed — click to unmark') : t('Mark as reviewed');
    $toggle_url = url('admin/drop/tickets/review/toggle/' . $t->tid,
      array('query' => array('token' => $token)));
    // Both icons are always rendered; CSS toggles which one is visible.
    $icon_off = icon('circle',       array('attributes' => array('width' => 20, 'height' => 20)));
    $icon_on  = icon('check-circle', array('attributes' => array('width' => 20, 'height' => 20)));
    $btn = '<button type="button" class="' . $btn_class . '" '
      . 'data-url="' . check_plain($toggle_url) . '" '
      . 'data-tid="' . (int) $t->tid . '" '
      . 'title="' . $btn_title . '">'
      . '<span class="droptickets-review-icon-off">' . $icon_off . '</span>'
      . '<span class="droptickets-review-icon-on">' . $icon_on . '</span>'
      . '</button>';

    $rows[] = array(
      'data' => array(
        array('data' => $btn,   'class' => array('droptickets-review-col-btn')),
        array('data' => l($t->title, 'admin/drop/tickets/' . $t->tid)),
        array('data' => $proj_name),
        array('data' => $assignee),
        array('data' => '<span class="droptickets-status droptickets-status--' . check_plain($t->status) . '">' . check_plain($status_label) . '</span>'),
      ),
      'class' => array(
        'droptickets-review-row',
        $is_reviewed ? 'droptickets-review-row--done' : '',
        drop_base_priority_class($t->priority) ? 'drop-priority-' . drop_base_priority_class($t->priority) : '',
      ),
      'data-tid' => (int) $t->tid,
    );
  }

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => array('', t('Title'), t('Project'), t('Assignee'), t('Status')),
    '#rows'       => $rows,
    '#attributes' => array('class' => array('droptickets-review-table')),
    '#empty'      => t('No active tickets.'),
  );

  return $output;
}

/**
 * AJAX callback: toggles the review flag for a ticket.
 *
 * Returns JSON: {tid, reviewed}.
 */
function droptickets_review_toggle($tid) {
  global $user;

  backdrop_add_http_header('Content-Type', 'application/json');

  $token = isset($_GET['token']) ? $_GET['token'] : '';
  if (!backdrop_valid_token($token, 'review-toggle')) {
    print backdrop_json_encode(array('error' => 'invalid token'));
    exit();
  }

  $tid      = (int) $tid;
  $reviewed = droptickets_review_toggle_state($user->uid, $tid);
  print backdrop_json_encode(array('tid' => $tid, 'reviewed' => $reviewed));
  exit();
}

/**
 * Page callback: clears all review flags for the current user.
 */
function droptickets_review_clear() {
  global $user;

  if (!backdrop_valid_token($_GET['token'], 'review-clear')) {
    backdrop_access_denied();
    return;
  }

  droptickets_review_clear_all($user->uid);
  backdrop_set_message(t('Review flags cleared.'));
  backdrop_goto('admin/drop/tickets/review');
}


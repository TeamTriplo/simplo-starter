<?php
/**
 * @file
 * Page callbacks for DropTasks.
 */

/**
 * Page callback: display the task list.
 *
 * @param string $filter
 *   One of 'active', 'today', 'upcoming', 'completed'.
 *
 * @return array
 *   A render array.
 */
function droptasks_task_list($filter = 'active') {
  global $user;

  backdrop_set_title(t('Tasks'));

  $today_start    = strtotime('today midnight');
  $today_end      = strtotime('tomorrow midnight');
  $upcoming_end   = strtotime('+7 days midnight');

  // Optional tag filter via ?tag=name query parameter.
  $tag_filter = isset($_GET['tag']) ? trim($_GET['tag']) : '';

  $query = db_select('drop_task', 't')
    ->fields('t')
    ->condition('t.uid', $user->uid);

  if ($tag_filter !== '') {
    $query->join('field_data_field_drop_tags', 'ft', "ft.entity_id = t.tid AND ft.entity_type = 'drop_task'");
    $query->join('taxonomy_term_data', 'td', 'td.tid = ft.field_drop_tags_tid');
    $query->condition('td.name', $tag_filter);
  }

  switch ($filter) {
    case 'today':
      // Overdue + due today.
      $query->condition('t.status', 'pending');
      $query->condition('t.due_date', $today_end, '<');
      $query->isNotNull('t.due_date');
      break;

    case 'upcoming':
      // Pending, due within the next 7 days (excludes overdue).
      $query->condition('t.status', 'pending');
      $query->condition('t.due_date', $today_start, '>=');
      $query->condition('t.due_date', $upcoming_end, '<');
      break;

    case 'completed':
      $query->condition('t.status', 'complete');
      $query->orderBy('t.changed', 'DESC');
      break;

    case 'active':
    default:
      $filter = 'active';
      $query->condition('t.status', 'pending');
      break;
  }

  if ($filter !== 'completed') {
    // Sort: overdue first, then by due date (nulls last), then by priority desc.
    $query->orderBy('t.due_date', 'ASC');
    $query->orderBy('t.priority', 'DESC');
  }

  $results = $query->execute()->fetchAllAssoc('tid');
  $tasks = array();
  if ($results) {
    $tids = array_keys($results);
    $tasks = entity_load('drop_task', $tids);
    // Re-order to match query order.
    $ordered = array();
    foreach ($tids as $tid) {
      if (isset($tasks[$tid])) {
        $ordered[$tid] = $tasks[$tid];
      }
    }
    $tasks = $ordered;
  }

  $output = array(
    '#theme'      => 'droptasks_task_list',
    '#tasks'      => $tasks,
    '#filter'     => $filter,
    '#tag_filter' => $tag_filter,
    '#attached'   => array(
      'css' => array(backdrop_get_path('module', 'droptasks') . '/droptasks.css'),
    ),
  );

  if (module_exists('dropcrm') && user_access('access crm')) {
    $output['#suffix'] = '<p>' . l(t('← Go to CRM'), 'admin/drop/crm') . '</p>';
  }

  return $output;
}

/**
 * Theme function: render the task list table.
 *
 * @param array $variables
 *   - tasks: array of DroptTask entities.
 *   - filter: the active filter string.
 *
 * @return string
 *   Rendered HTML.
 */
function theme_droptasks_task_list($variables) {
  $tasks      = $variables['tasks'];
  $filter     = $variables['filter'];
  $tag_filter = $variables['tag_filter'];

  $priority_labels = drop_base_priority_labels();

  // Active tag filter banner.
  $prefix = '';
  if ($tag_filter !== '') {
    $clear_url = url('admin/drop/tasks/' . $filter);
    $prefix = '<p class="droptasks-tag-filter-banner">'
      . t('Showing tasks tagged: <strong>@tag</strong>', array('@tag' => $tag_filter))
      . ' &nbsp;·&nbsp; '
      . '<a href="' . $clear_url . '">' . t('clear filter') . '</a>'
      . '</p>';
  }

  $rows = array();
  foreach ($tasks as $task) {
    $due = '';
    $row_class = '';
    if (!empty($task->due_date)) {
      $due = format_date($task->due_date, 'custom', 'M j, Y');
      if ($task->isOverdue()) {
        $due = '<span class="drop-overdue">' . $due . ' (' . t('overdue') . ')</span>';
        $row_class = 'drop-row-overdue';
      }
      elseif ($task->isDueToday()) {
        $due = '<span class="drop-due-today">' . $due . '</span>';
      }
    }

    $priority_label = isset($priority_labels[$task->priority]) ? $priority_labels[$task->priority] : '';
    $priority_class = 'drop-priority-' . drop_base_priority_class($task->priority);

    // Title + tag chips stacked in the first cell.
    $tags_html  = _droptasks_render_tags($task);
    $title_cell = l(check_plain($task->title), 'admin/drop/tasks/' . $task->tid);
    if ($tags_html) {
      $title_cell .= '<br>' . $tags_html;
    }

    $complete_label = ($task->status === 'complete') ? t('Reopen') : t('Complete');
    $actions = l(t('Edit'), 'admin/drop/tasks/' . $task->tid . '/edit')
      . ' &nbsp;·&nbsp; '
      . l($complete_label, 'admin/drop/tasks/' . $task->tid . '/complete')
      . ' &nbsp;·&nbsp; '
      . l(t('Delete'), 'admin/drop/tasks/' . $task->tid . '/delete');

    $row = array(
      'data' => array(
        array('data' => $title_cell),
        array('data' => $priority_label, 'class' => array($priority_class)),
        $due,
        $actions,
      ),
    );
    if ($row_class) {
      $row['class'] = array($row_class);
    }
    $rows[] = $row;
  }

  $empty_messages = array(
    'active'    => t('No active tasks. <a href="!url">Add one</a>.', array('!url' => url('admin/drop/tasks/add'))),
    'today'     => t('Nothing due today.'),
    'upcoming'  => t('Nothing due in the next 7 days.'),
    'completed' => t('No completed tasks.'),
  );
  $empty = isset($empty_messages[$filter]) ? $empty_messages[$filter] : t('No tasks found.');

  $header = array(t('Task'), t('Priority'), t('Due Date'), t('Actions'));

  return $prefix . theme('table', array(
    'header'     => $header,
    'rows'       => $rows,
    'empty'      => $empty,
    'attributes' => array('class' => array('droptasks-list')),
  ));
}

/**
 * Page callback: view a single task.
 *
 * @param DroptTask $task
 *   The task entity.
 *
 * @return array
 *   A render array.
 */
function droptasks_task_view(DroptTask $task) {
  backdrop_set_title(check_plain($task->title));

  $priority_labels = drop_base_priority_labels();

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'droptasks') . '/droptasks.css'),
    ),
  );

  // Actions bar.
  $actions = array();
  if (droptasks_task_access('update', $task)) {
    $complete_label = ($task->status === 'complete') ? t('Reopen') : t('Mark complete');
    $actions[] = l(t('Edit task'), 'admin/drop/tasks/' . $task->tid . '/edit');
    $actions[] = l($complete_label, 'admin/drop/tasks/' . $task->tid . '/complete');
  }
  if (droptasks_task_access('delete', $task)) {
    $actions[] = l(t('Delete'), 'admin/drop/tasks/' . $task->tid . '/delete');
  }

  // "Log time" link — links to DropTime daily page with note pre-filled.
  if (module_exists('droptime') && user_access('access droptime')) {
    $actions[] = l(t('+ Log time'), 'admin/drop/time', array(
      'query' => array(
        'notes'  => $task->title,
        'return' => 'admin/drop/tasks/' . $task->tid,
      ),
    ));
  }
  $output['actions'] = array(
    '#markup' => drop_base_page_actions($actions),
    '#weight' => -100,
  );

  // Detail rows.
  $rows = array();

  $status_label = ($task->status === 'complete') ? t('Complete') : t('Pending');
  $rows[] = array(t('Status'), $status_label);

  if (isset($priority_labels[$task->priority])) {
    $rows[] = array(t('Priority'), $priority_labels[$task->priority]);
  }

  if (!empty($task->due_date)) {
    $due = format_date($task->due_date, 'custom', 'M j, Y');
    if ($task->isOverdue()) {
      $due .= ' <span class="drop-overdue">(' . t('overdue') . ')</span>';
    }
    $rows[] = array(t('Due'), $due);
  }

  $rows[] = array(t('Created'), format_date($task->created, 'custom', 'M j, Y'));

  // Tags.
  $tags_html = _droptasks_render_tags($task);
  if ($tags_html) {
    $rows[] = array(t('Tags'), $tags_html);
  }

  // Linked contact.
  if (!empty($task->contact_id) && module_exists('dropcrm')) {
    $contacts = entity_load('crm_contact', array($task->contact_id));
    $contact  = $contacts ? reset($contacts) : NULL;
    if ($contact) {
      $rows[] = array(t('Contact'), l(entity_label('crm_contact', $contact), 'crm/contacts/' . $task->contact_id));
    }
  }

  // Source activity.
  if (!empty($task->source_activity_id) && module_exists('dropcrm')) {
    $activities = entity_load('crm_activity', array($task->source_activity_id));
    $activity   = $activities ? reset($activities) : NULL;
    if ($activity) {
      $rows[] = array(t('From activity'), l(entity_label('crm_activity', $activity), 'crm/activities/' . $task->source_activity_id));
    }
  }

  $output['details_heading'] = array(
    '#markup' => drop_base_section_header(t('Details')),
    '#weight' => 0,
  );
  $output['details'] = array(
    '#theme'      => 'table',
    '#rows'       => $rows,
    '#attributes' => array('class' => array('droptasks-detail-table')),
    '#weight'     => 1,
  );

  if (!empty($task->notes)) {
    $output['notes_heading'] = array(
      '#markup' => drop_base_section_header(t('Notes')),
      '#weight' => 10,
    );
    $output['notes'] = array(
      '#markup' => check_markup($task->notes),
      '#weight' => 11,
    );
  }

  return $output;
}

/**
 * Page callback: toggle task complete/pending.
 *
 * @param DroptTask $task
 *   The task entity.
 */
function droptasks_task_toggle_complete(DroptTask $task) {
  $task->status = ($task->status === 'complete') ? 'pending' : 'complete';
  $task->save();

  $label = ($task->status === 'complete') ? t('marked complete') : t('reopened');
  backdrop_set_message(t('Task %title @label.', array('%title' => $task->title, '@label' => $label)));

  backdrop_goto('admin/drop/tasks');
}

// Priority labels and CSS class helpers are provided by drop_base:
// drop_base_priority_labels() and drop_base_priority_class().

// ---------------------------------------------------------------------------
// Tag helpers
// ---------------------------------------------------------------------------

/**
 * Returns rendered tag chips HTML for a task, or an empty string.
 *
 * Each chip links to the task list filtered by that tag.
 *
 * @param DroptTask $task
 * @param string $base_path
 *   The list path to link chips to (defaults to 'admin/drop/tasks').
 *
 * @return string
 */
function _droptasks_render_tags(DroptTask $task, $base_path = 'admin/drop/tasks') {
  $items = field_get_items('drop_task', $task, 'field_drop_tags');
  if (!$items) {
    return '';
  }
  $tids  = array_column($items, 'tid');
  $terms = taxonomy_term_load_multiple($tids);

  $chips = '';
  foreach ($items as $item) {
    if (!empty($terms[$item['tid']])) {
      $name   = $terms[$item['tid']]->name;
      $chips .= l(check_plain($name), $base_path, array(
        'query'      => array('tag' => $name),
        'attributes' => array('class' => array('droptasks-tag-chip')),
      ));
    }
  }
  return $chips ? '<span class="droptasks-tags">' . $chips . '</span>' : '';
}

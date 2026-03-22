<?php
/**
 * @file
 * Form callbacks for DropTasks.
 */

/**
 * Form: add or edit a task.
 *
 * @param array $form
 * @param array $form_state
 * @param DroptTask|null $task
 *   Existing task for edit, or NULL for add.
 *
 * @return array
 */
function droptasks_task_form($form, &$form_state, $task = NULL) {
  if (empty($task)) {
    $task = new DroptTask();
  }
  $form_state['task'] = $task;

  // Pre-fill title from GET param when creating from an activity.
  $title_default = $task->title;
  if (empty($task->tid) && !empty($_GET['title'])) {
    $title_default = check_plain($_GET['title']);
  }

  $form['title'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Task'),
    '#default_value' => $title_default,
    '#required'      => TRUE,
    '#maxlength'     => 255,
    '#autofocus'     => TRUE,
  );

  // Linked contact — pre-filled from GET param or existing task value.
  $contact_id = !empty($task->contact_id) ? $task->contact_id : NULL;
  if (empty($task->tid) && !empty($_GET['contact'])) {
    $contact_id = (int) $_GET['contact'];
  }
  if ($contact_id && module_exists('dropcrm')) {
    $contacts = entity_load('crm_contact', array($contact_id));
    $contact  = $contacts ? reset($contacts) : NULL;
    if ($contact) {
      $form['contact_display'] = array(
        '#markup' => '<p class="droptasks-linked-contact">'
          . t('Linked contact:') . ' '
          . l(entity_label('crm_contact', $contact), 'crm/contacts/' . $contact_id)
          . '</p>',
      );
    }
  }
  $form['contact_id'] = array(
    '#type'  => 'hidden',
    '#value' => $contact_id ? $contact_id : '',
  );

  // Source activity — stored silently from GET param or existing task value.
  $activity_id = !empty($task->source_activity_id) ? $task->source_activity_id : NULL;
  if (empty($task->tid) && !empty($_GET['activity'])) {
    $activity_id = (int) $_GET['activity'];
  }
  $form['source_activity_id'] = array(
    '#type'  => 'hidden',
    '#value' => $activity_id ? $activity_id : '',
  );

  $form['status'] = array(
    '#type'          => 'select',
    '#title'         => t('Status'),
    '#options'       => array(
      'pending'  => t('Pending'),
      'complete' => t('Complete'),
    ),
    '#default_value' => $task->status,
  );

  $form['priority'] = array(
    '#type'          => 'select',
    '#title'         => t('Priority'),
    '#options'       => array(
      2 => t('High'),
      1 => t('Normal'),
      0 => t('Low'),
    ),
    '#default_value' => $task->priority,
  );

  $due_default = !empty($task->due_date) ? format_date($task->due_date, 'custom', 'Y-m-d') : '';
  $form['due_date'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Due date'),
    '#default_value' => $due_default,
    '#description'   => t('Format: YYYY-MM-DD. Leave blank for no due date.'),
    '#size'          => 12,
    '#maxlength'     => 10,
    '#attributes'    => array('class' => array('droptasks-datepicker'), 'autocomplete' => 'off'),
  );

  $form['#attached']['js'][]  = 'core/misc/ui/jquery.ui.datepicker.min.js';
  $form['#attached']['css'][] = 'core/misc/ui/jquery.ui.core.css';
  $form['#attached']['css'][] = 'core/misc/ui/jquery.ui.datepicker.css';
  $form['#attached']['css'][] = 'core/misc/ui/jquery.ui.theme.css';
  $form['#attached']['js'][]  = backdrop_get_path('module', 'droptasks') . '/droptasks.datepicker.js';

  $form['notes'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Notes'),
    '#default_value' => $task->notes,
    '#rows'          => 5,
    '#required'      => FALSE,
  );

  // Attach field widgets (includes field_drop_tags taxonomy autocomplete).
  field_attach_form('drop_task', $task, $form, $form_state);

  // Push tags below all hand-coded fields.
  if (isset($form['field_drop_tags'])) {
    $form['field_drop_tags']['#weight'] = 50;
  }

  $is_new = empty($task->tid);
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => $is_new ? t('Add task') : t('Save task'),
  );
  if (!$is_new) {
    $form['actions']['cancel'] = array(
      '#type'       => 'link',
      '#title'      => t('Cancel'),
      '#href'       => 'admin/drop/tasks/' . $task->tid,
      '#attributes' => array('class' => array('button', 'button-secondary')),
    );
  }

  // "Back to activity" link when creating a follow-up from an activity view.
  if ($is_new && $activity_id) {
    $form['actions']['back'] = array(
      '#markup' => ' ' . l(t('← Back to activity'), 'crm/activities/' . $activity_id),
    );
  }

  return $form;
}

/**
 * Validate handler for droptasks_task_form().
 */
function droptasks_task_form_validate($form, &$form_state) {
  $due = trim($form_state['values']['due_date']);
  if ($due !== '') {
    $ts = strtotime($due . ' midnight');
    if ($ts === FALSE || !preg_match('/^\d{4}-\d{2}-\d{2}$/', $due)) {
      form_set_error('due_date', t('Due date must be in YYYY-MM-DD format.'));
    }
  }
}

/**
 * Submit handler for droptasks_task_form().
 */
function droptasks_task_form_submit($form, &$form_state) {
  $task = $form_state['task'];

  $task->title    = $form_state['values']['title'];
  $task->status   = $form_state['values']['status'];
  $task->priority = (int) $form_state['values']['priority'];
  $task->notes    = $form_state['values']['notes'];

  $due = trim($form_state['values']['due_date']);
  $task->due_date = ($due !== '') ? strtotime($due . ' midnight') : NULL;

  $cid = $form_state['values']['contact_id'];
  $task->contact_id = ($cid !== '') ? (int) $cid : NULL;

  $aid = $form_state['values']['source_activity_id'];
  $task->source_activity_id = ($aid !== '') ? (int) $aid : NULL;

  // Copy field widget values (tags etc.) onto the entity before saving.
  field_attach_submit('drop_task', $task, $form, $form_state);

  $task->save();

  backdrop_set_message(t('Task saved.'));
  // Return to the source activity so follow-up tasks can be added easily.
  if (!empty($task->source_activity_id)) {
    $form_state['redirect'] = 'crm/activities/' . $task->source_activity_id;
  }
  else {
    $form_state['redirect'] = 'admin/drop/tasks';
  }
}

/**
 * Form: confirm task deletion.
 *
 * @param array $form
 * @param array $form_state
 * @param DroptTask $task
 *
 * @return array
 */
function droptasks_task_delete_form($form, &$form_state, DroptTask $task) {
  $form_state['task'] = $task;
  return confirm_form(
    $form,
    t('Are you sure you want to delete the task %title?', array('%title' => $task->title)),
    'tasks',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for droptasks_task_delete_form().
 */
function droptasks_task_delete_form_submit($form, &$form_state) {
  $task = $form_state['task'];
  $task->delete();
  backdrop_set_message(t('Task %title deleted.', array('%title' => $task->title)));
  $form_state['redirect'] = 'admin/drop/tasks';
}

<?php
/**
 * @file
 * Settings form for DropBridge.
 */

/**
 * Form: DropBridge settings.
 */
function dropbridge_settings_form($form, &$form_state) {
  $config = config('dropbridge.settings');

  $form['calendar'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Global Calendars'),
    '#description' => t('These iCal feeds appear on <strong>everyone\'s</strong> dashboard. For personal calendars that only appear on one person\'s dashboard, each person can add them under their own account settings. Both private and public iCal URLs are supported.'),
  );

  $form['calendar']['calendar_ics_url'] = array(
    '#type'          => 'textarea',
    '#title'         => t('iCal URL(s)'),
    '#default_value' => $config->get('calendar_ics_url'),
    '#rows'          => 5,
    '#description'   => t('One URL per line. Optionally add a label after a pipe character: <code>URL | Label</code> (e.g. <code>https://…/basic.ics | Wolves</code>). Without a label, events show as "Event". Private and public iCal URLs both work.'),
  );

  $form['calendar']['calendar_cache_seconds'] = array(
    '#type'          => 'select',
    '#title'         => t('Refresh interval'),
    '#default_value' => $config->get('calendar_cache_seconds') ?: 1800,
    '#options'       => array(
      300   => t('5 minutes'),
      900   => t('15 minutes'),
      1800  => t('30 minutes'),
      3600  => t('1 hour'),
      10800 => t('3 hours'),
      86400 => t('24 hours'),
    ),
    '#description'   => t('How often to re-fetch each calendar feed.'),
  );

  $form['slack'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Slack Integration'),
    '#description' => t('Used for direct-message notifications to individual team members. Create a Slack app at <a href="https://api.slack.com/apps" target="_blank">api.slack.com/apps</a>, add the <code>chat:write</code> scope under Bot Token Scopes, install it to your workspace, and paste the bot token here. Each person sets their own Slack member ID in their account settings.'),
  );

  $form['slack']['slack_bot_token'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Bot token'),
    '#default_value' => $config->get('slack_bot_token'),
    '#description'   => t('Starts with <code>xoxb-</code>. Leave blank to disable all Slack notifications.'),
    '#placeholder'   => 'xoxb-…',
    '#attributes'    => array('autocomplete' => 'off'),
  );

  $form['slack']['slack_debug_mode'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Debug mode'),
    '#default_value' => $config->get('slack_debug_mode'),
    '#description'   => t('When enabled, reminder checks ignore the scheduled time window and the "already sent today" guard — every cron run (or the test button below) will fire if no time has been logged. Disable when done testing.'),
  );

  $form['announcement'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Site Announcement'),
    '#description' => t('Displays a dismissible banner at the top of every page for all team members. Clear the message to remove the banner. Each new message resets dismissals — everyone will see it again.'),
  );

  $form['announcement']['announcement_text'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Message'),
    '#default_value' => $config->get('announcement_text'),
    '#rows'          => 3,
    '#description'   => t('Leave blank to show no announcement. Plain text only.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save settings'),
  );
  $form['actions']['fetch'] = array(
    '#type'   => 'submit',
    '#value'  => t('Clear cache & fetch now'),
    '#submit' => array('dropbridge_settings_form_fetch'),
  );
  $form['actions']['test_reminder'] = array(
    '#type'   => 'submit',
    '#value'  => t('Send test reminder now'),
    '#submit' => array('dropbridge_settings_form_test_reminder'),
  );

  return $form;
}

/**
 * Submit handler for dropbridge_settings_form() — save settings.
 */
function dropbridge_settings_form_submit($form, &$form_state) {
  $config = config('dropbridge.settings');
  $config->set('calendar_ics_url', trim($form_state['values']['calendar_ics_url']));
  $config->set('calendar_cache_seconds', (int) $form_state['values']['calendar_cache_seconds']);
  $config->set('announcement_text', trim($form_state['values']['announcement_text']));
  $config->set('slack_bot_token', trim($form_state['values']['slack_bot_token']));
  $config->set('slack_debug_mode', !empty($form_state['values']['slack_debug_mode']));
  $config->save();

  cache_clear_all('dropbridge_calendar_', 'cache', TRUE);

  backdrop_set_message(t('Settings saved.'));
  $form_state['redirect'] = 'drop';
}

/**
 * Submit handler for the "Send test reminder now" button.
 *
 * Runs the time reminder check immediately for the current user, forcing
 * debug mode on regardless of the saved setting. Does not save form values.
 */
function dropbridge_settings_form_test_reminder($form, &$form_state) {
  global $user;
  _dropbridge_cron_time_reminders(TRUE);
  backdrop_set_message(t('Test reminder check complete. Check watchdog for details, and your Slack DM if a reminder was sent.'));
  $form_state['redirect'] = 'drop/settings';
}

/**
 * Submit handler for the "Clear cache & fetch now" button.
 *
 * Clears cached ICS data without saving any form changes, then sends the
 * user to the daily list where a fresh fetch happens immediately.
 */
function dropbridge_settings_form_fetch($form, &$form_state) {
  cache_clear_all('dropbridge_calendar_', 'cache', TRUE);
  backdrop_set_message(t('Calendar cache cleared. Fetching fresh data now.'));
  $form_state['redirect'] = 'drop';
}

/**
 * Form: permissions for all enabled Drop modules.
 *
 * Reuses core's user_admin_permissions theme and save logic, filtered to
 * modules whose machine name starts with "drop".
 */
function dropbridge_permissions_form($form, &$form_state) {
  $roles = user_roles(FALSE, NULL, TRUE);
  $form['roles'] = array('#type' => 'value', '#value' => $roles);

  $module_info = system_get_info('module');
  $options     = array();
  $status      = array();
  $idx         = 0;

  // Collect permissions from enabled drop* modules, sorted by display name.
  $drop_modules = array();
  foreach (module_implements('permission') as $module) {
    if ($module === 'dropbridge' || strpos($module, 'drop') === 0) {
      $drop_modules[$module] = isset($module_info[$module]['name']) ? $module_info[$module]['name'] : $module;
    }
  }
  asort($drop_modules);

  foreach ($drop_modules as $module => $display_name) {
    $permissions = module_invoke($module, 'permission');
    if (empty($permissions)) {
      continue;
    }

    // Module heading row (numeric key).
    $form['permission'][$idx] = array(
      '#markup' => $display_name,
      '#id'     => $module,
    );
    $idx++;

    foreach ($permissions as $perm => $perm_item) {
      $perm_item += array(
        'description'     => '',
        'restrict access' => FALSE,
        'warning'         => '',
      );
      $options[$perm] = '';
      $form['permission'][$perm] = array(
        '#type'        => 'item',
        '#markup'      => $perm_item['title'],
        '#description' => theme('user_permission_description', array('permission_item' => $perm_item)),
        '#id'          => drupal_html_id('permission-' . $perm),
      );
      foreach ($roles as $role_name => $role) {
        if (in_array($perm, $role->permissions)) {
          $status[$role_name][] = $perm;
        }
      }
    }
  }

  foreach ($roles as $role_name => $role) {
    $form['checkboxes'][$role_name] = array(
      '#type'          => 'checkboxes',
      '#options'       => $options,
      '#default_value' => isset($status[$role_name]) ? $status[$role_name] : array(),
      '#attributes'    => array('class' => array('role-' . $role_name)),
    );
    $form['role_names'][$role_name] = array(
      '#markup'      => check_plain($role->label),
      '#description' => filter_xss_admin($role->description),
      '#tree'        => TRUE,
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save permissions'),
  );

  $form['#theme']    = 'user_admin_permissions';
  $form['#attached']['js'][] = backdrop_get_path('module', 'user') . '/js/user.permissions.js';

  return $form;
}

/**
 * Submit handler for dropbridge_permissions_form().
 */
function dropbridge_permissions_form_submit($form, &$form_state) {
  foreach ($form_state['values']['roles'] as $role_name => $role) {
    user_role_change_permissions($role->name, $form_state['values'][$role->name]);
  }
  backdrop_set_message(t('Permissions saved.'));
}

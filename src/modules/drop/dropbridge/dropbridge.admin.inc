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

  return $form;
}

/**
 * Submit handler for dropbridge_settings_form() — save settings.
 */
function dropbridge_settings_form_submit($form, &$form_state) {
  $config = config('dropbridge.settings');
  $config->set('calendar_ics_url', trim($form_state['values']['calendar_ics_url']));
  $config->set('calendar_cache_seconds', (int) $form_state['values']['calendar_cache_seconds']);
  $config->save();

  cache_clear_all('dropbridge_calendar_', 'cache', TRUE);

  backdrop_set_message(t('Settings saved.'));
  $form_state['redirect'] = 'admin/drop';
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
  $form_state['redirect'] = 'admin/drop';
}

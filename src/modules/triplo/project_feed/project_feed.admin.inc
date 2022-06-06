<?php
/**
 * @file
 * Administration pages provided by Project Feed module.
 */

/**
 * Menu callback for admin/config/system/projects/settings.
 */
function project_feed_settings() {
  $config = config('project_feed.settings');

  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('User Name'),
    '#description' => t('Github User Name.'),
    '#default_value' => $config->get('username'),
    '#disabled' => !empty($client_id) ? TRUE : FALSE,
  );

  $form['github_auth_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Github Auth Token'),
    '#description' => t('Your token from Github'),
    '#default_value' => $config->get('github_auth_token'),
    '#disabled' => !empty($client_id) ? TRUE : FALSE,
  );

  $form['queries_per_pull'] = array(
    '#type' => 'textfield',
    '#title' => t('Queries per pull'),
    '#description' => t('How many requests to make per pull. This will include queries for content that already exists and does not create new nodes.'),
    '#default_value' => $config->get('queries_per_pull'),
    '#disabled' => !empty($client_id) ? TRUE : FALSE,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Submit handler for the githubapi_admin_settings() form.
 */
function project_feed_settings_submit($form, &$form_state) {
  $config = config('project_feed.settings');

  // Store only if value not stored in settings.php.
  $config->set('username', $form_state['values']['username']);
  $config->set('github_auth_token', $form_state['values']['github_auth_token']);
  $config->set('queries_per_pull', $form_state['values']['queries_per_pull']);

  $config->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}
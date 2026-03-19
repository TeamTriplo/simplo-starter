<?php
/**
 * @file
 * Administration pages for the Project Feed module.
 */

/**
 * Settings and action form at admin/config/services/projects/settings.
 */
function project_feed_settings($form, &$form_state) {
  $pf_config = config('project_feed.settings');

  // --- GitHub credentials --------------------------------------------------
  $form['github'] = array(
    '#type'  => 'fieldset',
    '#title' => t('GitHub credentials'),
  );
  $form['github']['username'] = array(
    '#type'          => 'textfield',
    '#title'         => t('GitHub username'),
    '#description'   => t('Your GitHub username, used as the API User-Agent string.'),
    '#default_value' => $pf_config->get('username'),
  );
  $form['github']['github_auth_token'] = array(
    '#type'          => 'textfield',
    '#title'         => t('GitHub personal access token'),
    '#description'   => t('A token with <em>public_repo</em> scope. Raises the rate limit to 5,000 requests/hour.'),
    '#default_value' => $pf_config->get('github_auth_token'),
  );
  $form['github']['save_github'] = array(
    '#type'   => 'submit',
    '#value'  => t('Save GitHub credentials'),
    '#submit' => array('project_feed_settings_submit'),
  );

  // --- Anthropic credentials -----------------------------------------------
  $form['anthropic'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Anthropic API credentials'),
  );
  $form['anthropic']['claude_api_key'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Anthropic API key'),
    '#description'   => t('Your API key from <a href="https://console.anthropic.com">console.anthropic.com</a>. Used for AI tag generation and maintainer analysis.'),
    '#default_value' => $pf_config->get('claude_api_key'),
  );
  $form['anthropic']['save_anthropic'] = array(
    '#type'   => 'submit',
    '#value'  => t('Save Anthropic credentials'),
    '#submit' => array('project_feed_settings_submit'),
  );

  // --- Automatic fetch schedule --------------------------------------------
  $last_fetch = state_get('project_feed_last_fetch', 0);
  $last_fetch_display = $last_fetch
    ? t('Last fetched @time ago (@date).', array(
        '@time' => format_interval(REQUEST_TIME - $last_fetch),
        '@date' => format_date($last_fetch, 'short'),
      ))
    : t('No automatic fetch has run yet.');

  $form['schedule'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Automatic fetch schedule'),
    '#description' => $last_fetch_display,
  );
  $form['schedule']['fetch_frequency'] = array(
    '#type'          => 'select',
    '#title'         => t('Fetch frequency'),
    '#description'   => t('How often cron should automatically fetch all projects from GitHub. Set to "Never" to disable automatic fetching.'),
    '#options'       => array(
      'never'   => t('Never'),
      'daily'   => t('Once a day'),
      'weekly'  => t('Once a week'),
      'monthly' => t('Once a month'),
    ),
    '#default_value' => $pf_config->get('fetch_frequency') ?: 'weekly',
  );
  $form['schedule']['save_schedule'] = array(
    '#type'   => 'submit',
    '#value'  => t('Save schedule'),
    '#submit' => array('project_feed_settings_submit'),
  );

  // Show queue status and abort button if a fetch is in progress.
  $queue_count = db_query("SELECT COUNT(*) FROM {queue} WHERE name = 'project_feed_fetch'")->fetchField();
  if ($queue_count > 0) {
    $form['schedule']['queue_status'] = array(
      '#type'   => 'markup',
      '#markup' => '<p>' . t('Fetch in progress: @count page(s) remaining in queue.', array('@count' => $queue_count)) . '</p>',
    );
    $form['schedule']['abort_fetch'] = array(
      '#type'   => 'submit',
      '#value'  => t('Abort fetch'),
      '#submit' => array('project_feed_abort_fetch_submit'),
    );
  }

  // --- Single project fetch ------------------------------------------------
  $form['fetch_single'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Fetch a single project'),
    '#description' => t('Enter the exact GitHub repository name to fetch or refresh one project immediately.'),
  );
  $form['fetch_single']['repo_name'] = array(
    '#type'        => 'textfield',
    '#title'       => t('Repository name'),
    '#description' => t('The exact repo name in the backdrop-contrib organization, e.g. <code>webform</code>.'),
    '#size'        => 40,
  );
  $form['fetch_single']['fetch_single_submit'] = array(
    '#type'     => 'submit',
    '#value'    => t('Fetch single project'),
    '#submit'   => array('project_feed_fetch_single_submit'),
    '#validate' => array('project_feed_fetch_single_validate'),
  );

  // --- Batch 1: GitHub data fetch ------------------------------------------
  $form['fetch'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Step 1: Fetch project data from GitHub'),
    '#description' => t(
      'Fetches all repositories from the <em>backdrop-contrib</em> GitHub organization '
      . 'and creates or updates local project nodes. Updates only GitHub-sourced fields — '
      . 'manually entered data (maintainer, community tags, notes, etc.) is never overwritten. '
      . 'Makes ~4,300 GitHub API calls (~29 pages × 150 calls/page). Save credentials above before running.'
    ),
  );
  $form['fetch']['fetch_submit'] = array(
    '#type'     => 'submit',
    '#value'    => t('Fetch all projects from GitHub'),
    '#submit'   => array('project_feed_start_fetch'),
    '#validate' => array(),
  );

  // --- Batch 2: README analysis --------------------------------------------
  $form['readme'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Step 2: Analyze READMEs with AI'),
    '#description' => t(
      'Fetches the README for every project node, parses maintainer information, '
      . 'and asks Claude (Haiku) to generate category tags. '
      . 'Results are saved to <em>Maintainers (parsed from README)</em> and <em>Auto Tags</em> fields — '
      . 'separate from existing hand-curated data so you can compare and decide what to keep. '
      . 'Requires the Anthropic API key above. Makes ~1,400 GitHub calls + ~1,400 Claude calls.'
    ),
  );
  $form['readme']['readme_submit'] = array(
    '#type'     => 'submit',
    '#value'    => t('Analyze READMEs with AI'),
    '#submit'   => array('project_feed_start_readme_analysis'),
    '#validate' => array(),
  );

  return $form;
}

/**
 * Validate handler: ensure a repo name was entered.
 */
function project_feed_fetch_single_validate($form, &$form_state) {
  if (empty(trim($form_state['values']['repo_name']))) {
    form_set_error('repo_name', t('Please enter a repository name.'));
  }
}

/**
 * Submit handler: fetch and save a single project from GitHub.
 */
function project_feed_fetch_single_submit($form, &$form_state) {
  $pf_config = config('project_feed.settings');
  $user  = $pf_config->get('username');
  $token = $pf_config->get('github_auth_token');

  if (empty($user) || empty($token)) {
    backdrop_set_message(t('Please configure your GitHub credentials before fetching.'), 'error');
    return;
  }

  $repo_name = trim($form_state['values']['repo_name']);

  // Fetch the repo metadata from GitHub.
  $repo_response = project_feed_github_request(
    'https://api.github.com/repos/backdrop-contrib/' . $repo_name,
    $user, $token
  );
  if ($repo_response['code'] == 404) {
    backdrop_set_message(t('Repository %name not found in backdrop-contrib.', array('%name' => $repo_name)), 'error');
    return;
  }
  if ($repo_response['code'] != 200) {
    backdrop_set_message(t('GitHub API returned HTTP @code. Check your credentials.', array('@code' => $repo_response['code'])), 'error');
    return;
  }
  $repo = json_decode($repo_response['body'], TRUE);

  // Hand off to the same shared helper used by the batch process.
  try {
    $result = project_feed_process_repo($repo, $user, $token);
    backdrop_set_message(t('Project %name successfully @result.', array('%name' => $repo_name, '@result' => $result)));
  }
  catch (Exception $e) {
    backdrop_set_message(t('Error saving %name: @msg', array('%name' => $repo_name, '@msg' => $e->getMessage())), 'error');
  }
}

/**
 * Submit handler: clear the fetch queue to abort an in-progress fetch.
 */
function project_feed_abort_fetch_submit($form, &$form_state) {
  $deleted = db_delete('queue')->condition('name', 'project_feed_fetch')->execute();
  backdrop_set_message(t('Fetch aborted — @count page(s) removed from the queue.', array('@count' => $deleted)));
}

/**
 * Submit handler: save all credentials from whichever save button was clicked.
 */
function project_feed_settings_submit($form, &$form_state) {
  $pf_config = config('project_feed.settings');
  $pf_config->set('username', $form_state['values']['username']);
  $pf_config->set('github_auth_token', $form_state['values']['github_auth_token']);
  $pf_config->set('claude_api_key', $form_state['values']['claude_api_key']);
  $pf_config->set('fetch_frequency', $form_state['values']['fetch_frequency']);
  $pf_config->save();
  backdrop_set_message(t('Credentials saved.'));
}

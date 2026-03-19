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
 * Submit handler: save all credentials from whichever save button was clicked.
 */
function project_feed_settings_submit($form, &$form_state) {
  $pf_config = config('project_feed.settings');
  $pf_config->set('username', $form_state['values']['username']);
  $pf_config->set('github_auth_token', $form_state['values']['github_auth_token']);
  $pf_config->set('claude_api_key', $form_state['values']['claude_api_key']);
  $pf_config->save();
  backdrop_set_message(t('Credentials saved.'));
}

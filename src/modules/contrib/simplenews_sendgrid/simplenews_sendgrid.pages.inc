<?php
/**
 * @file
 * Page callbacks for SimpleNews SendGrid stats.
 */

/**
 * Page callback: per-newsletter stats report.
 */
function simplenews_sendgrid_stats_page() {
  $stored    = state_get('simplenews_sendgrid.stats', NULL);
  $fetched   = state_get('simplenews_sendgrid.stats_fetched', NULL);
  $api_key   = config_get('sendgrid_integration.settings', 'sendgrid_integration_apikey');

  $output = array(
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'simplenews_sendgrid') . '/simplenews_sendgrid.css'),
    ),
  );

  // Header row: last-fetched time + refresh button.
  $refresh_url = url('admin/config/services/simplenews/stats/refresh',
    array('query' => array('token' => backdrop_get_token('sn-sendgrid-refresh'))));

  $last_fetched_html = $fetched
    ? t('Last updated: @time', array('@time' => format_date($fetched, 'short')))
    : t('No data yet.');

  $output['header'] = array(
    '#markup' => '<div class="sn-sendgrid-stats-header">'
      . '<span class="sn-sendgrid-last-fetched">' . $last_fetched_html . '</span>'
      . '<a href="' . $refresh_url . '" class="sn-sendgrid-refresh-btn button">'
      . t('Refresh data') . '</a>'
      . '</div>',
    '#weight' => -10,
  );

  if (empty($api_key)) {
    $output['no_key'] = array(
      '#markup' => '<p class="messages warning">'
        . t('No SendGrid API key found. Configure it at <a href="@url">Admin → Config → Services → SendGrid</a>.',
            array('@url' => url('admin/config/services/sendgrid')))
        . '</p>',
      '#weight' => 0,
    );
    return $output;
  }

  if (empty($stored)) {
    $output['empty'] = array(
      '#markup' => '<p>' . t('Click <strong>Refresh data</strong> to load stats from SendGrid.') . '</p>',
      '#weight' => 0,
    );
    return $output;
  }

  // Build the stats table.
  $rows = array();
  foreach ($stored as $item) {
    if (empty($item['name']) || empty($item['metrics'])) {
      continue;
    }
    $m = $item['metrics'];

    $delivered     = isset($m['delivered'])     ? (int) $m['delivered']     : 0;
    $opens         = isset($m['opens'])         ? (int) $m['opens']         : 0;
    $unique_opens  = isset($m['unique_opens'])  ? (int) $m['unique_opens']  : 0;
    $clicks        = isset($m['clicks'])        ? (int) $m['clicks']        : 0;
    $unique_clicks = isset($m['unique_clicks']) ? (int) $m['unique_clicks'] : 0;
    $unsubscribes  = isset($m['unsubscribes'])  ? (int) $m['unsubscribes']  : 0;
    $bounces       = isset($m['bounces'])       ? (int) $m['bounces']       : 0;
    $spam          = isset($m['spam_reports'])  ? (int) $m['spam_reports']  : 0;

    $open_rate  = $delivered > 0 ? round(($unique_opens  / $delivered) * 100, 1) . '%' : '—';
    $click_rate = $delivered > 0 ? round(($unique_clicks / $delivered) * 100, 1) . '%' : '—';

    $rows[] = array(
      check_plain($item['name']),
      number_format($delivered),
      number_format($unique_opens) . ' <span class="sn-sendgrid-rate">(' . $open_rate . ')</span>',
      number_format($unique_clicks) . ' <span class="sn-sendgrid-rate">(' . $click_rate . ')</span>',
      number_format($unsubscribes),
      number_format($bounces),
      $spam > 0 ? '<span class="sn-sendgrid-spam">' . number_format($spam) . '</span>' : '0',
    );
  }

  if (empty($rows)) {
    $output['empty'] = array(
      '#markup' => '<p>' . t('No category stats found. Make sure newsletters have been sent with category tagging enabled.') . '</p>',
      '#weight' => 0,
    );
    return $output;
  }

  $output['table'] = array(
    '#theme'      => 'table',
    '#header'     => array(
      t('Newsletter / Issue'),
      t('Delivered'),
      t('Unique opens'),
      t('Unique clicks'),
      t('Unsubscribes'),
      t('Bounces'),
      t('Spam'),
    ),
    '#rows'       => $rows,
    '#attributes' => array('class' => array('sn-sendgrid-stats-table')),
    '#weight'     => 0,
  );

  $output['note'] = array(
    '#markup' => '<p class="sn-sendgrid-note">'
      . t('Stats cover the last 365 days. Each newsletter category and individual issue title appear as separate rows.')
      . '</p>',
    '#weight' => 10,
  );

  return $output;
}

/**
 * Page callback: refresh stats from SendGrid API and redirect back.
 */
function simplenews_sendgrid_stats_refresh() {
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'sn-sendgrid-refresh')) {
    backdrop_access_denied();
    return;
  }

  $data = simplenews_sendgrid_fetch_stats();

  if ($data !== FALSE) {
    state_set('simplenews_sendgrid.stats', $data);
    state_set('simplenews_sendgrid.stats_fetched', REQUEST_TIME);
    backdrop_set_message(t('Newsletter stats refreshed from SendGrid.'));
  }
  else {
    backdrop_set_message(t('Failed to fetch stats from SendGrid. Check the API key and try again.'), 'error');
  }

  backdrop_goto('admin/config/services/simplenews/stats');
}

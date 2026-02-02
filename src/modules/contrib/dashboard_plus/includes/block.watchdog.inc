<?php
/**
 * @file
 * Dashboard block showing recent watchdog messages.
 */
class DashboardWatchdogBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return t('Recent log messages');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Recent log messages');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Shows the 10 most recent watchdog log messages.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('access site reports');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!user_access('access site reports')) {
      return array();
    }
    // Determine count from GET param, default 10, allowed: 10, 25, 50, 100
    $allowed = array(10, 25, 50, 100);
    $count = isset($_GET['watchdog_count']) && in_array((int) $_GET['watchdog_count'], $allowed) ? (int) $_GET['watchdog_count'] : 10;
    // Dropdown form
    $form = '<form method="get" style="margin-bottom:8px;display:inline;white-space:nowrap;">'
      . '<label for="watchdog-count-select" style="display:inline;">' . t('Show') . '</label> '
      . '<select id="watchdog-count-select" name="watchdog_count" onchange="this.form.submit()" style="display:inline;">';
    foreach ($allowed as $option) {
      $selected = ($option == $count) ? ' selected' : '';
      $form .= '<option value="' . $option . '"' . $selected . '>' . $option . '</option>';
    }
    $form .= '</select> ' . t('messages') . '</form>';
    $query = db_select('watchdog', 'w')
      ->fields('w')
      ->orderBy('wid', 'DESC')
      ->range(0, $count);
    $result = $query->execute();
    $rows = array();
    foreach ($result as $row) {
      $rows[] = array(
        'data' => array(
          format_date($row->timestamp, 'short'),
          check_plain($row->type),
          check_plain($row->message),
        ),
      );
    }
    $header = array(
      array('data' => t('Date')),
      array('data' => t('Type')),
      array('data' => t('Message')),
    );
    $panel = array(
      '#theme' => 'dashboard_panel__watchdog',
    );
    $panel['form'] = array(
      '#markup' => $form,
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No log messages found.'),
    );
    $panel['link'] = array(
      '#theme' => 'link',
      '#path' => 'admin/reports/dblog',
      '#text' => t('View all log messages'),
    );
    return $panel;
  }
} 
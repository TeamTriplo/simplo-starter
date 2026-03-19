<?php
/**
 * @file
 * Dashboard block showing cron status.
 */
class DashboardCronStatusBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Cron status');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Cron status');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Shows when cron last ran and provides a link to run it manually.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('administer site configuration');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!$this->checkAccess()) {
      return array();
    }

    $cron_last = state_get('cron_last');
    if (!is_numeric($cron_last)) {
      $cron_last = state_get('install_time', 0);
    }

    $config = config('system.core');
    $threshold_warning = $config->get('cron_threshold_warning') ?: 172800;
    $threshold_error = $config->get('cron_threshold_error') ?: 1209600;

    $elapsed = REQUEST_TIME - $cron_last;

    if ($elapsed > $threshold_error) {
      $status_class = 'error';
    }
    elseif ($elapsed > $threshold_warning) {
      $status_class = 'warning';
    }
    else {
      $status_class = 'status';
    }

    $panel = array(
      '#theme' => 'dashboard_panel__cron_status',
    );
    $panel['status'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('messages', $status_class)),
    );
    $panel['status']['message'] = array(
      '#markup' => t('Last run !time ago.', array('!time' => format_interval($elapsed))),
    );
    $panel['link'] = array(
      '#theme' => 'link',
      '#path' => 'admin/reports/status/run-cron',
      '#text' => t('Run cron manually'),
    );

    return $panel;
  }
}

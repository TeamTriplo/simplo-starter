<?php
/**
 * @file
 * Architecture summary block for Dashboard Plus.
 */
class DashboardPlusArchitectureSummaryBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Site architecture summary');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Architecture summary');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Compact snapshot of site architecture: content, modules, recipes, and cron.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('view architecture report');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!$this->checkAccess()) {
      return array();
    }

    // Total node count.
    $total_nodes = db_query('SELECT COUNT(*) FROM {node}')->fetchField();

    // Content type count.
    $type_count = count(node_type_get_types());

    // Enabled module count.
    $modules = system_list('module_enabled');
    $module_count = count($modules);

    // Recipe count (modules with package = Recipes).
    $recipe_count = 0;
    if (module_exists('recipes')) {
      foreach ($modules as $name => $module) {
        if (isset($module->info['package']) && $module->info['package'] === 'Recipes') {
          $recipe_count++;
        }
      }
    }

    // Cron last run.
    $cron_last = state_get('cron_last');
    $cron_display = $cron_last ? format_interval(REQUEST_TIME - $cron_last) . ' ' . t('ago') : t('Never');

    $rows = array(
      array(t('Total content'), number_format($total_nodes)),
      array(t('Content types'), $type_count),
      array(t('Enabled modules'), $module_count),
      array(t('Installed recipes'), $recipe_count),
      array(t('Cron last run'), $cron_display),
    );

    $output = array(
      'table' => array(
        '#theme' => 'table',
        '#rows' => $rows,
        '#attributes' => array('class' => array('arch-summary-table')),
      ),
    );

    if (user_access('view architecture report')) {
      $output['link'] = array(
        '#markup' => '<p>' . l(t('View full Architecture report'), 'admin/dashboard/architecture') . '</p>',
      );
    }

    return $output;
  }
}

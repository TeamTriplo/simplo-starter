<?php
/**
 * @file
 * Dashboard block listing recently released Backdrop modules.
 */
class DashboardRecentModulesBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'count' => 10,
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('New Module Releases');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - New module releases');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Lists newly published modules on BackdropCMS.org (first releases only, not updates).');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('access administration pages');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!$this->checkAccess()) {
      return array();
    }

    $count = (int) $this->settings['count'];
    $modules = $this->fetchRecentModules();

    if ($modules === NULL) {
      return array(
        '#markup' => '<p>' . t('Unable to fetch release information at this time. Please try again later.') . '</p>',
      );
    }

    $modules = array_slice($modules, 0, $count);

    $rows = array();
    foreach ($modules as $module) {
      $rows[] = array(
        'data' => array(
          l($module['title'], $module['link'], array('attributes' => array('target' => '_blank'))),
          t('!time ago', array('!time' => format_interval(REQUEST_TIME - $module['timestamp']))),
        ),
      );
    }

    $panel = array(
      '#theme' => 'table',
      '#header' => array(t('Module'), t('Released')),
      '#rows' => $rows,
      '#attributes' => array('class' => array('dashboard-recent-modules')),
    );

    $footer = array(
      '#markup' => '<p>' . l(t('Browse all releases on BackdropCMS.org'), 'https://backdropcms.org/modules/releases', array('attributes' => array('target' => '_blank'))) . '</p>',
    );

    return array('table' => $panel, 'footer' => $footer);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['count'] = array(
      '#type' => 'select',
      '#title' => t('Number of releases to display'),
      '#options' => array(5 => 5, 10 => 10, 15 => 15, 20 => 20),
      '#default_value' => $this->settings['count'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['count'] = (int) $form_state['values']['count'];
  }

  /**
   * Fetches recent module releases from the BackdropCMS.org RSS feed.
   *
   * Cached for one hour. Returns up to 20 items (the feed maximum); the
   * caller slices to the configured count.
   *
   * @return array|NULL
   *   Array of associative arrays with keys: title, link, timestamp.
   *   Returns NULL on failure.
   */
  protected function fetchRecentModules() {
    $cid = 'dashboard_plus:recent_modules_rss';
    $cache = cache()->get($cid);
    if ($cache && REQUEST_TIME - $cache->created < 3600) {
      return $cache->data;
    }

    $response = backdrop_http_request('https://backdropcms.org/project/rss.xml/releases', array('timeout' => 10));
    if ($response->code != '200' || empty($response->data)) {
      return NULL;
    }

    // Suppress XML parsing errors and parse the feed.
    $previous = libxml_use_internal_errors(TRUE);
    $xml = simplexml_load_string($response->data);
    libxml_use_internal_errors($previous);

    if (!$xml || !isset($xml->channel->item)) {
      return NULL;
    }

    $modules = array();
    foreach ($xml->channel->item as $item) {
      // Strip the "New release for project: " prefix from the title, then
      // decode any HTML entities the feed may have double-encoded.
      $title = preg_replace('/^New release for project:\s*/i', '', (string) $item->title);
      $title = html_entity_decode($title, ENT_QUOTES | ENT_HTML5, 'UTF-8');
      $timestamp = strtotime((string) $item->pubDate);

      if ($title && $timestamp && !empty($item->link)) {
        $modules[] = array(
          'title'     => $title,
          'link'      => (string) $item->link,
          'timestamp' => $timestamp,
        );
      }
    }

    if (empty($modules)) {
      return NULL;
    }

    // Sort newest first — the feed order is not guaranteed.
    usort($modules, function($a, $b) {
      return $b['timestamp'] - $a['timestamp'];
    });

    cache()->set($cid, $modules);
    return $modules;
  }

}

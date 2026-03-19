<?php
/**
 * @file
 * Dashboard block displaying a randomly selected Backdrop module each day.
 */
class DashboardModuleOfDayBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Module of the Day');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Module of the day');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Displays a randomly selected Backdrop CMS module each day.');
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

    backdrop_add_js(backdrop_get_path('module', 'dashboard_plus') . '/js/module-of-day.js');

    $ajax_url = check_plain(url('dashboard-plus/motd-next'));
    $output = '<div class="dashboard-module-of-day-wrapper" data-motd-url="' . $ajax_url . '">';
    $output .= '<div class="dashboard-module-of-day-content">';
    $output .= $this->renderModuleHtml();
    $output .= '</div>';
    $output .= '<div style="margin-top:10px;text-align:right;">';
    $output .= '<a href="#" class="dashboard-motd-next button button-secondary">' . t('See another') . '</a>';
    $output .= '</div>';
    $output .= '</div>';

    return array('#markup' => $output);
  }

  /**
   * Renders the inner module display HTML.
   *
   * Public so the AJAX menu callback can call it after updating the session
   * skip counter.
   *
   * @return string
   *   Rendered HTML for the current module of the day.
   */
  public function renderModuleHtml() {
    $module = $this->fetchModuleOfDay();

    if (!$module) {
      return '<p>' . t('Unable to fetch module information at this time. Please try again later.') . '</p>';
    }

    $project_icon = icon('puzzle-piece', array(
      'attributes' => array('style' => 'width:20px;height:20px;vertical-align:middle;margin-right:6px;'),
    ));

    $output = '<div class="dashboard-module-of-day">';

    // Title linked to project page. Construct URL from machine name, matching
    // the pattern used by Backdrop's installer module.
    $output .= '<div class="module-title" style="font-size:1.15em;font-weight:bold;margin-bottom:6px;">';
    $output .= $project_icon;
    if (!empty($module['name'])) {
      $project_url = 'https://backdropcms.org/project/' . check_plain($module['name']);
      $output .= l(check_plain($module['title']), $project_url, array(
        'attributes' => array('target' => '_blank'),
      ));
    }
    else {
      $output .= check_plain($module['title']);
    }
    $output .= '</div>';

    // Description, truncated with a Read more toggle.
    if (!empty($module['description'])) {
      $output .= $this->buildDescriptionHtml($module['description']);
    }

    // Meta: install count and maintenance status.
    $meta = array();
    if (!empty($module['usage'])) {
      $buildings_icon = icon('buildings', array(
        'attributes' => array('style' => 'width:13px;height:13px;vertical-align:middle;'),
      ));
      $meta[] = $buildings_icon . ' ' . t('!count installs', array('!count' => number_format((int) $module['usage'])));
    }
    if (!empty($module['maintenance status'])) {
      $meta[] = check_plain($module['maintenance status']);
    }
    if (!empty($meta)) {
      $output .= '<div class="module-meta" style="font-size:0.85em;color:#666;margin-top:6px;">';
      $output .= implode(' &middot; ', $meta);
      $output .= '</div>';
    }

    $output .= '</div>';
    return $output;
  }

  /**
   * Builds the description HTML with truncation and Read more toggle.
   *
   * @param string $description
   *   Raw description text from the API.
   *
   * @return string
   *   HTML markup for the description area.
   */
  protected function buildDescriptionHtml($description) {
    $plain = strip_tags($description);
    $limit = 160;

    if (backdrop_strlen($plain) <= $limit) {
      return '<div class="module-description" style="margin-bottom:6px;">' . filter_xss($description) . '</div>';
    }

    $short = truncate_utf8($plain, $limit, TRUE, TRUE);

    $output = '<div class="module-description" style="margin-bottom:6px;">';
    $output .= '<span class="module-desc-short">' . check_plain($short) . '</span>';
    $output .= '<span class="module-desc-full" style="display:none;">' . filter_xss($description) . '</span>';
    $output .= ' <a href="#" class="module-desc-toggle">' . t('Read more') . '</a>';
    $output .= '</div>';
    return $output;
  }

  /**
   * Fetches the module to feature based on today's date and the skip counter.
   *
   * The skip counter is stored in the user's session and incremented each time
   * "See another" is clicked. This gives a consistent result on page refresh
   * but a different module on each click.
   *
   * @return array|NULL
   *   A project array as returned by the projects server, or NULL on failure.
   */
  protected function fetchModuleOfDay() {
    $today = date('Y-m-d');
    $skip = isset($_SESSION['dashboard_plus_motd_skip'][$today])
      ? (int) $_SESSION['dashboard_plus_motd_skip'][$today]
      : 0;

    $cid = 'dashboard_plus:module_of_day:' . $today . ':' . $skip;
    $cache = cache()->get($cid);
    if ($cache) {
      return $cache->data;
    }

    $total = $this->fetchTotalModuleCount();
    if (!$total) {
      return NULL;
    }

    $daily_offset = (int) (hexdec(substr(md5($today . ':' . $skip), 0, 8)) % $total);

    $module = $this->fetchModuleAtOffset($daily_offset);
    if ($module) {
      cache()->set($cid, $module, strtotime('tomorrow'));
    }
    return $module;
  }

  /**
   * Fetches the total count of available Backdrop modules.
   *
   * Cached for 24 hours.
   *
   * @return int|NULL
   *   Total number of modules, or NULL on failure.
   */
  protected function fetchTotalModuleCount() {
    $cid = 'dashboard_plus:module_total_count';
    $cache = cache()->get($cid);
    if ($cache && REQUEST_TIME - $cache->created < 86400) {
      return $cache->data;
    }

    $result = $this->queryProjectsServer(array('items_per_page' => 1, 'page' => 0));
    if ($result && isset($result['total']) && (int) $result['total'] > 0) {
      $total = (int) $result['total'];
      cache()->set($cid, $total);
      return $total;
    }
    return NULL;
  }

  /**
   * Fetches the module at a specific offset from the projects server.
   *
   * @param int $offset
   *   Zero-based index of the module to fetch.
   *
   * @return array|NULL
   *   Project array or NULL on failure.
   */
  protected function fetchModuleAtOffset($offset) {
    $result = $this->queryProjectsServer(array('items_per_page' => 1, 'page' => $offset));
    if ($result && !empty($result['projects'])) {
      return reset($result['projects']);
    }
    return NULL;
  }

  /**
   * Sends a query to the Backdrop projects server.
   *
   * @param array $filters
   *   Query parameters to merge with the defaults.
   *
   * @return array|NULL
   *   Decoded JSON response array, or NULL on failure.
   */
  protected function queryProjectsServer(array $filters = array()) {
    $backdrop_version = explode('.', BACKDROP_CORE_COMPATIBILITY);
    $filters += array(
      'method'  => 'query',
      'type'    => 'module',
      'version' => $backdrop_version[0],
    );

    $url = 'https://projects.backdropcms.org/query/module/1?' . http_build_query($filters, '', '&');
    $response = backdrop_http_request($url, array('timeout' => 10));

    if ($response->code == '200') {
      return backdrop_json_decode($response->data);
    }
    return NULL;
  }

}

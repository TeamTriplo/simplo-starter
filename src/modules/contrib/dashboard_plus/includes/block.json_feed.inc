<?php
/**
 * @file
 * Dashboard block that displays items from any JSON Feed-compliant URL.
 * @see https://www.jsonfeed.org/version/1.1/
 */
class DashboardJsonFeedBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'feed_url'       => '',
      'item_count'     => 10,
      'show_date'      => TRUE,
      'show_content'   => FALSE,
      'cache_minutes'  => 60,
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    if (!empty($this->settings['title'])) {
      return check_plain($this->settings['title']);
    }
    // Fall back to the feed's own title if we have a cached copy.
    $data = $this->fetchFeed();
    if ($data && !empty($data['title'])) {
      return check_plain($data['title']);
    }
    return t('JSON Feed');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - JSON Feed');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Displays items from a JSON Feed-compliant URL.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (empty($this->settings['feed_url'])) {
      if (user_access('administer blocks')) {
        return array('#markup' => '<p>' . t('No feed URL configured. Edit this block to add one.') . '</p>');
      }
      return array();
    }

    $data = $this->fetchFeed();

    if ($data === FALSE) {
      if (user_access('administer blocks')) {
        return array('#markup' => '<p>' . t('Could not retrieve feed. Check the URL and try again.') . '</p>');
      }
      return array();
    }

    if (empty($data['items'])) {
      return array('#markup' => '<p>' . t('No items found in this feed.') . '</p>');
    }

    $items = array_slice($data['items'], 0, (int) $this->settings['item_count']);
    $output = '';

    foreach ($items as $item) {
      // Prefer external_url (linked articles) over the feed item permalink.
      $url   = !empty($item['external_url']) ? $item['external_url'] : (isset($item['url']) ? $item['url'] : '');
      $title = isset($item['title']) ? $item['title'] : t('(Untitled)');

      $link = $url ? l($title, $url, array('attributes' => array('target' => '_blank', 'rel' => 'noopener'))) : check_plain($title);

      $meta = '';
      if ($this->settings['show_date'] && !empty($item['date_published'])) {
        $timestamp = strtotime($item['date_published']);
        if ($timestamp) {
          $meta = '<span class="json-feed-date">' . format_date($timestamp, 'custom', 'M j, Y') . '</span>';
        }
      }

      $content = '';
      if ($this->settings['show_content'] && !empty($item['content_html'])) {
        $content = '<div class="json-feed-content">' . filter_xss($item['content_html'], array('p', 'a', 'em', 'strong', 'ul', 'ol', 'li', 'blockquote', 'br', 'code', 'pre')) . '</div>';
      }

      $output .= '<div class="json-feed-item">';
      $output .= '<div class="json-feed-title">' . $link . '</div>';
      if ($meta) {
        $output .= $meta;
      }
      $output .= $content;
      $output .= '</div>';
    }

    backdrop_add_css(backdrop_get_path('module', 'dashboard_plus') . '/css/json-feed.css');

    $build = array(
      '#markup' => '<div class="json-feed-list">' . $output . '</div>',
    );

    // Link to the feed's home page in a footer if available.
    if (!empty($data['home_page_url'])) {
      $build['#markup'] .= '<div class="json-feed-footer">' . l(t('Visit @title', array('@title' => isset($data['title']) ? $data['title'] : $data['home_page_url'])), $data['home_page_url'], array('attributes' => array('target' => '_blank', 'rel' => 'noopener'))) . '</div>';
    }

    return $build;
  }

  /**
   * Fetches and caches the JSON feed data.
   *
   * @return array|false
   *   Decoded feed array, or FALSE on failure.
   */
  protected function fetchFeed() {
    $url = $this->settings['feed_url'];
    if (empty($url)) {
      return FALSE;
    }

    $cid = 'dashboard_plus:json_feed:' . md5($url);
    $cached = cache_get($cid);
    if ($cached && $cached->expire > REQUEST_TIME) {
      return $cached->data;
    }

    $result = backdrop_http_request($url, array('timeout' => 10));
    if ($result->code != 200 || empty($result->data)) {
      return FALSE;
    }

    $data = json_decode($result->data, TRUE);
    if (!is_array($data) || empty($data['version']) || strpos($data['version'], 'jsonfeed.org') === FALSE) {
      return FALSE;
    }

    $ttl = max(5, (int) $this->settings['cache_minutes']) * 60;
    cache_set($cid, $data, 'cache', REQUEST_TIME + $ttl);

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['feed_url'] = array(
      '#type' => 'textfield',
      '#title' => t('JSON Feed URL'),
      '#default_value' => $this->settings['feed_url'],
      '#description' => t('URL of a <a href="https://www.jsonfeed.org/" target="_blank">JSON Feed</a>-compliant endpoint. Example: https://daringfireball.net/feeds/json'),
      '#required' => TRUE,
      '#maxlength' => 512,
    );
    $form['item_count'] = array(
      '#type' => 'select',
      '#title' => t('Number of items to display'),
      '#options' => array(5 => 5, 10 => 10, 15 => 15, 20 => 20, 25 => 25),
      '#default_value' => $this->settings['item_count'],
    );
    $form['show_date'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show publication date'),
      '#default_value' => $this->settings['show_date'],
    );
    $form['show_content'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show item content'),
      '#description' => t('Display the HTML content of each item below its title. Use with caution on busy feeds.'),
      '#default_value' => $this->settings['show_content'],
    );
    $form['cache_minutes'] = array(
      '#type' => 'select',
      '#title' => t('Cache duration'),
      '#options' => array(
        15  => t('15 minutes'),
        30  => t('30 minutes'),
        60  => t('1 hour'),
        180 => t('3 hours'),
        360 => t('6 hours'),
      ),
      '#default_value' => $this->settings['cache_minutes'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $v = $form_state['values'];
    $this->settings['feed_url']      = trim($v['feed_url']);
    $this->settings['item_count']    = (int) $v['item_count'];
    $this->settings['show_date']     = !empty($v['show_date']);
    $this->settings['show_content']  = !empty($v['show_content']);
    $this->settings['cache_minutes'] = (int) $v['cache_minutes'];

    // Bust the cache when settings change.
    $cid = 'dashboard_plus:json_feed:' . md5($this->settings['feed_url']);
    cache_clear_all($cid, 'cache');
  }
}

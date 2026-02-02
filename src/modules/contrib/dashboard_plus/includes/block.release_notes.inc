<?php
/**
 * @file
 * Dashboard block showing the latest Backdrop CMS release notes from GitHub.
 */
class DashboardBackdropReleaseNotesBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return t('Backdrop CMS Latest Release Notes');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Backdrop release notes');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Shows the release notes for the latest version of Backdrop CMS.');
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
    if (!$this->checkAccess()) {
      return array();
    }
    // Use Backdrop's cache system to avoid excessive API calls.
    $cid = 'dashboard_plus:backdrop_release_notes_list';
    $cache = cache()->get($cid);
    if ($cache && REQUEST_TIME - $cache->created < 3600) {
      $releases = $cache->data;
    }
    else {
      // Fetch the list of releases from GitHub API.
      $releases = NULL;
      $url = 'https://api.github.com/repos/backdrop/backdrop/releases';
      $context = stream_context_create([
        'http' => [
          'user_agent' => 'BackdropDashboardPlus/1.0',
          'timeout' => 5,
        ],
      ]);
      $json = @file_get_contents($url, false, $context);
      if ($json) {
        $data = json_decode($json);
        if ($data && is_array($data)) {
          $releases = [];
          foreach ($data as $release) {
            if (isset($release->name) && isset($release->body) && isset($release->html_url)) {
              // Improved Markdown to HTML for headings, bullet points, and links.
              $body = $release->body;
              // Links: [text](url) with safety
              $body = preg_replace_callback(
                '/\[([^\]]+)\]\(([^\)]+)\)/',
                function ($matches) {
                  return '<a href="' . check_url($matches[2]) . '" target="_blank">' . check_plain($matches[1]) . '</a>';
                },
                $body
              );
              // Headings: ## and ### (escape content)
              $body = preg_replace_callback('/^### (.+)$/m', function($m) { return '<h4>' . check_plain($m[1]) . '</h4>'; }, $body);
              $body = preg_replace_callback('/^## (.+)$/m', function($m) { return '<h3>' . check_plain($m[1]) . '</h3>'; }, $body);
              // Split into lines for list and paragraph handling
              $lines = preg_split('/\r?\n/', $body);
              $in_list = false;
              $html = '';
              foreach ($lines as $line) {
                $trimmed = trim($line);
                if (preg_match('/^<h[34]>/', $trimmed)) {
                  if ($in_list) {
                    $html .= '</ul>';
                    $in_list = false;
                  }
                  $html .= $trimmed;
                } elseif (preg_match('/^([*-]) (.+)$/', $trimmed, $matches)) {
                  if (!$in_list) {
                    $html .= '<ul>';
                    $in_list = true;
                  }
                  $html .= '<li>' . filter_xss($matches[2], ['a']) . '</li>';
                } elseif ($trimmed === '') {
                  if ($in_list) {
                    $html .= '</ul>';
                    $in_list = false;
                  }
                } else {
                  if ($in_list) {
                    $html .= '</ul>';
                    $in_list = false;
                  }
                  $html .= '<p>' . filter_xss($trimmed, ['a']) . '</p>';
                }
              }
              if ($in_list) {
                $html .= '</ul>';
              }
              $body = $html;
              $releases[] = [
                'name' => check_plain($release->name),
                'body' => $body,
                'url' => check_url($release->html_url),
              ];
            }
          }
          cache()->set($cid, $releases);
        }
      }
    }
    if (!$releases || !is_array($releases) || count($releases) == 0) {
      return array('#markup' => t('Unable to fetch the release notes at this time.'));
    }
    // Find the latest minor and latest bugfix releases.
    $latest_minor = NULL;
    $latest_bugfix = NULL;
    foreach ($releases as $release) {
      if (!$latest_bugfix) {
        $latest_bugfix = $release;
      }
      if (!$latest_minor) {
        // Minor release: ends with .0 (e.g., 1.30.0)
        if (preg_match('/\\.0$/', $release['name'])) {
          $latest_minor = $release;
        }
      }
      if ($latest_bugfix && $latest_minor) break;
    }
    // Determine which release to show based on GET parameter.
    $type = (isset($_GET['release_type']) && $_GET['release_type'] === 'bugfix') ? 'bugfix' : 'minor';
    $selected_release = ($type === 'bugfix' && $latest_bugfix) ? $latest_bugfix : $latest_minor;
    // Build the dropdown form.
    $form = '<form method="get" style="margin-bottom:8px;display:inline;white-space:nowrap;">'
      . '<label for="release-type-select" style="display:inline;">' . t('Show') . '</label> '
      . '<select id="release-type-select" name="release_type" onchange="this.form.submit()" style="display:inline;">'
      . '<option value="minor"' . ($type === 'minor' ? ' selected' : '') . '>' . t('Latest minor release') . '</option>'
      . '<option value="bugfix"' . ($type === 'bugfix' ? ' selected' : '') . '>' . t('Latest bugfix release') . '</option>'
      . '</select> '
      . t('notes') . '</form>';
    // Style for the light box.
    $box_style = 'background:#f9f9fa;border:1px solid #e0e0e0;padding:18px 20px 12px 20px;border-radius:8px;margin-bottom:8px;';
    $output = $form;
    $output .= '<div class="backdrop-release-notes-box" style="' . $box_style . '">';
    $output .= '<h3 style="margin-top:0;">' . t('Backdrop CMS') . ' ' . $selected_release['name'] . '</h3>';
    $output .= '<div class="backdrop-release-notes" style="margin-bottom:10px;">' . $selected_release['body'] . '</div>';
    $output .= '<p style="margin-bottom:0;"><a href="' . $selected_release['url'] . '" target="_blank">' . t('View full release notes on GitHub') . '</a></p>';
    $output .= '</div>';
    return array('#markup' => $output);
  }
} 
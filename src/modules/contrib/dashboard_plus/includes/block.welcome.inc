<?php
/**
 * @file
 * Welcome block with embedded block installer for Dashboard Plus.
 */

// =============================================================================
// Block class
// =============================================================================

class DashboardPlusWelcomeBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Dashboard Plus — Customize your dashboard');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Welcome');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Lets you add Dashboard Plus blocks to any dashboard region without leaving the dashboard. Remove when no longer needed.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('administer layouts');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!$this->checkAccess()) {
      return array();
    }

    $output = array();

    $output['intro'] = array(
      '#markup' => '<p>' . t('Add blocks to your dashboard without visiting the layout manager. Choose a block and a region, then click <em>Add to dashboard</em>.') . '</p>',
    );

    $output['form'] = backdrop_get_form('dashboard_plus_block_installer_form');

    $output['layout_link'] = array(
      '#markup' => '<p class="dashboard-plus-layout-link">'
        . t('For full layout control, visit the !link.', array(
          '!link' => l(t('layout manager'), 'admin/structure/layouts/manage/dashboard', array(
            'query' => array('destination' => 'admin/dashboard'),
          )),
        ))
        . '</p>',
    );

    if (user_access('administer layouts')) {
      $output['reorganize_link'] = array(
        '#markup' => '<p>' . l(t('Reorganize layout'), '#', array(
          'attributes' => array('class' => array('dashboard-plus-reorganize-trigger')),
          'external' => TRUE,
        )) . '</p>',
      );
    }

    $output['more_info'] = array(
      '#markup' => _dashboard_plus_welcome_more_info(),
    );

    return $output;
  }

}

// =============================================================================
// More info block list
// =============================================================================

/**
 * Returns a <details> element listing all Dashboard Plus blocks with descriptions.
 */
function _dashboard_plus_welcome_more_info() {
  $blocks = array(
    array(
      'name' => t('Architecture summary'),
      'desc' => t('At-a-glance stats: content count, content types, enabled modules, installed recipes, and time since cron last ran. Links to the full Architecture report.'),
    ),
    array(
      'name' => t('Content block'),
      'desc' => t('Displays a specific piece of content. Pick an existing node or create a new Card directly from the dashboard. Supports multiple instances.'),
    ),
    array(
      'name' => t('Cron status'),
      'desc' => t('Shows when cron last ran with a warning if it is overdue, plus a link to run it manually.'),
    ),
    array(
      'name' => t('Current weather'),
      'desc' => t('Current conditions for a configured location — temperature, humidity, and wind speed. Powered by Open-Meteo, no API key required.'),
    ),
    array(
      'name' => t('JSON Feed'),
      'desc' => t('Displays items from any JSON Feed-compliant URL. Configurable item count, date display, and cache duration. Supports multiple instances.'),
    ),
    array(
      'name' => t('Manage content'),
      'desc' => t('Links to create or manage content for every content type on the site. A more useful replacement for the core "Create content" block.'),
    ),
    array(
      'name' => t('Manage taxonomy'),
      'desc' => t('All taxonomy vocabularies with a term count and links to add or manage terms.'),
    ),
    array(
      'name' => t('Module of the day'),
      'desc' => t('A randomly selected Backdrop module pulled from BackdropCMS.org each day, with stats and a "See another" button.'),
    ),
    array(
      'name' => t('New module releases'),
      'desc' => t('The most recently published modules on BackdropCMS.org — first releases only, not updates.'),
    ),
    array(
      'name' => t('Quick links'),
      'desc' => t('A customizable set of links to pages, forms, or configuration you use often. Add links inline; they display in a responsive multi-column grid. Supports multiple instances.'),
    ),
    array(
      'name' => t('Recent log messages'),
      'desc' => t('The most recent watchdog log entries with a link to the full log.'),
    ),
    array(
      'name' => t('Recent unpublished comments'),
      'desc' => t('Comments awaiting moderation with a link to the moderation queue.'),
    ),
    array(
      'name' => t('Resources'),
      'desc' => t('Links to key Backdrop CMS resources: forums, user guide, and the core issue queue.'),
    ),
  );

  $items = '';
  foreach ($blocks as $block) {
    $items .= '<li><strong>' . $block['name'] . '</strong> — ' . $block['desc'] . '</li>';
  }

  return '<details class="dashboard-plus-more-info">'
    . '<summary>' . t('What blocks does Dashboard Plus provide?') . '</summary>'
    . '<ul>' . $items . '</ul>'
    . '</details>';
}

// =============================================================================
// Block installer form
// =============================================================================

/**
 * Form: Add a block to the dashboard layout.
 */
function dashboard_plus_block_installer_form($form, &$form_state) {
  $form['#attached']['css'][] = backdrop_get_path('module', 'dashboard_plus') . '/css/block-installer.css';

  // Load the active dashboard layout.
  $layouts = layout_load_multiple_by_path('admin/dashboard');
  $layout = reset($layouts);

  if (!$layout) {
    return array(
      '#markup' => '<p>' . t('Could not load the dashboard layout.') . '</p>',
    );
  }

  // Build a set of already-placed module:delta keys.
  $placed = array();
  foreach ($layout->content as $block) {
    if (isset($block->module, $block->delta)) {
      $placed[$block->module . ':' . $block->delta] = TRUE;
    }
  }

  // Modules whose blocks are appropriate for the dashboard.
  // Page-component modules (system, layout) are intentionally excluded —
  // their blocks (breadcrumb, page title, tabs, etc.) require full page context.
  $allowed_modules = array(
    'dashboard',       // Core dashboard blocks.
    'dashboard_plus',  // Dashboard Plus blocks (includes Existing content picker).
    'block',           // Custom blocks created by site architects.
    'views',           // Views-generated blocks.
    'user',            // User account / login blocks.
    'search',          // Search form block.
    'statistics',      // Popular content block.
    'poll',            // Poll block.
    'forum',           // Forum statistics block.
  );

  // Build block options grouped by module, excluding already-placed blocks.
  $all_block_info = layout_get_block_info();
  $module_list = system_list('module_enabled');

  $block_options = array('' => t('-- Choose a block --'));
  foreach ($all_block_info as $module => $blocks) {
    if (!in_array($module, $allowed_modules)) {
      continue;
    }
    $group = array();
    foreach ($blocks as $delta => $info) {
      $key = $module . ':' . $delta;
      $multi = !empty($info['multiple instances']);
      if ($multi || !isset($placed[$key])) {
        $group[$key] = isset($info['info']) ? $info['info'] : $delta;
      }
    }
    if (!empty($group)) {
      $module_name = isset($module_list[$module]->info['name']) ? $module_list[$module]->info['name'] : $module;
      $block_options[$module_name] = $group;
    }
  }

  // Build region options from the layout's available positions.
  $region_labels = _dashboard_plus_region_labels($layout);
  $region_options = array('' => t('-- Choose a region --'));
  foreach ($region_labels as $region_key => $region_label) {
    $region_options[$region_key] = $region_label;
  }

  $form['layout_name'] = array(
    '#type' => 'value',
    '#value' => $layout->name,
  );

  // Flex row: selects + submit button side by side on desktop.
  $form['row'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('dashboard-plus-installer-row')),
  );

  $form['row']['block_key'] = array(
    '#type' => 'select',
    '#title' => t('Block'),
    '#options' => $block_options,
    '#required' => TRUE,
  );

  $form['row']['region'] = array(
    '#type' => 'select',
    '#title' => t('Region'),
    '#options' => $region_options,
    '#required' => TRUE,
    '#default_value' => 'top',
  );

  $form['row']['actions'] = array('#type' => 'actions');
  $form['row']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add to dashboard'),
  );

  return $form;
}

/**
 * Submit handler: add the selected block to the dashboard layout.
 */
function dashboard_plus_block_installer_form_submit($form, &$form_state) {
  $layout_name = $form_state['values']['layout_name'];
  $block_key   = $form_state['values']['block_key'];
  $region      = $form_state['values']['region'];

  list($module, $delta) = explode(':', $block_key, 2);

  $layout = layout_load($layout_name);
  if (!$layout) {
    backdrop_set_message(t('Could not load the layout. Please try again.'), 'error');
    return;
  }

  $layout->addBlock($module, $delta, $region);
  $layout->save();

  // Get a friendly label for the message.
  $block_info = layout_get_block_info($module, $delta);
  $block_label = is_array($block_info) && isset($block_info['info']) ? $block_info['info'] : $block_key;

  backdrop_set_message(t('!block has been added to the !region region.', array(
    '!block'  => $block_label,
    '!region' => $region,
  )));

  $form_state['redirect'] = 'admin/dashboard';
}

// =============================================================================
// Helper
// =============================================================================

/**
 * Returns human-readable region labels for a layout.
 *
 * Tries the layout template's region definitions first, then falls back to
 * a curated lookup table, then to auto-generated labels from key names.
 *
 * @param Layout $layout
 *   The loaded layout object.
 *
 * @return array
 *   Associative array of region_key => label, excluding system-only regions.
 */
function _dashboard_plus_region_labels($layout) {
  // Regions that should not be offered for block placement.
  $skip = array('title');

  // Try getting labels from the layout template info.
  if (!empty($layout->layout)) {
    $template_info = layout_get_layout_template_info($layout->layout);
    if (!empty($template_info['regions'])) {
      $labels = array();
      foreach ($template_info['regions'] as $key => $label) {
        if (!in_array($key, $skip)) {
          $labels[$key] = $label;
        }
      }
      return $labels;
    }
  }

  // Use the shared label map from the module file.
  $known = _dashboard_plus_region_label_map();

  $labels = array();
  foreach (array_keys($layout->positions) as $key) {
    if (in_array($key, $skip)) {
      continue;
    }
    if (isset($known[$key])) {
      $labels[$key] = $known[$key];
    }
    else {
      // Last resort: title-case the key.
      $labels[$key] = ucwords(str_replace('_', ' ', $key));
    }
  }

  return $labels;
}

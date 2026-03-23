<?php
/**
 * @file
 * Page callback and section builders for the Architecture report tab.
 */

/**
 * Form callback: Architecture report with vertical tabs.
 */
function dashboard_plus_architecture_form($form, &$form_state) {
  $form['#attached']['css'][] = backdrop_get_path('module', 'dashboard_plus') . '/css/architecture.css';

  $form['tabs'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['overview']    = _dashboard_plus_arch_section_overview();
  $form['content']     = _dashboard_plus_arch_section_content();
  $form['data_model']  = _dashboard_plus_arch_section_data_model();
  $form['structure']   = _dashboard_plus_arch_section_structure();
  $form['blocks']      = _dashboard_plus_arch_section_blocks();
  $form['people']      = _dashboard_plus_arch_section_people();
  $form['modules']     = _dashboard_plus_arch_section_modules();
  // Recipes tab hidden until the Recipes module is released.
  // $form['recipes']  = _dashboard_plus_arch_section_recipes();
  $form['media']       = _dashboard_plus_arch_section_media();

  return $form;
}

// =============================================================================
// Section: Overview
// =============================================================================

/**
 * Builds the Overview section fieldset.
 */
function _dashboard_plus_arch_section_overview() {
  $section = _dashboard_plus_arch_fieldset(t('Overview'), 'overview');

  $rows = array();

  $rows[] = array(t('Site name'), config_get('system.core', 'site_name'));
  $rows[] = array(t('Site email'), config_get('system.core', 'site_mail'));
  $rows[] = array(t('Backdrop CMS version'), BACKDROP_VERSION);
  $rows[] = array(t('PHP version'), phpversion());

  global $databases;
  $driver = isset($databases['default']['default']['driver']) ? $databases['default']['default']['driver'] : t('Unknown');
  $rows[] = array(t('Database driver'), $driver);

  $theme = config_get('system.core', 'theme_default');
  $rows[] = array(t('Default theme'), $theme);

  $admin_theme = config_get('system.core', 'admin_theme');
  $rows[] = array(t('Admin theme'), $admin_theme ? $admin_theme : t('Same as default theme'));

  $cron_last = state_get('cron_last');
  $rows[] = array(t('Cron last run'), $cron_last ? format_date($cron_last, 'short') : t('Never'));

  $front = config_get('system.core', 'site_frontpage');
  $rows[] = array(t('Front page'), $front ? $front : t('Default'));

  $slogan = config_get('system.core', 'site_slogan');
  $rows[] = array(t('Site slogan'), $slogan ? check_plain($slogan) : t('(none)'));

  $section['table'] = _dashboard_plus_arch_kv_table($rows);

  $section['links'] = _dashboard_plus_arch_links(array(
    array(t('Status report'), 'admin/reports/status'),
    array(t('Site settings'), 'admin/config/system/site-information'),
    array(t('Cron settings'), 'admin/config/system/cron'),
  ));

  return $section;
}

// =============================================================================
// Section: Content
// =============================================================================

/**
 * Builds the Content section fieldset.
 */
function _dashboard_plus_arch_section_content() {
  $section = _dashboard_plus_arch_fieldset(t('Content'), 'content');

  $types = node_type_get_types();

  if (empty($types)) {
    $section['empty'] = array('#markup' => '<p>' . t('No content types defined.') . '</p>');
    return $section;
  }

  // Query node counts grouped by type and status.
  $counts = array();
  $result = db_query('SELECT type, status, COUNT(*) AS count FROM {node} GROUP BY type, status');
  foreach ($result as $row) {
    $counts[$row->type][$row->status] = (int) $row->count;
  }

  $header = array(
    t('Content type'),
    t('Machine name'),
    t('Published'),
    t('Unpublished'),
    t('Total'),
  );

  $rows = array();
  $grand_total = 0;
  foreach ($types as $type_name => $type) {
    $published   = isset($counts[$type_name][1]) ? $counts[$type_name][1] : 0;
    $unpublished = isset($counts[$type_name][0]) ? $counts[$type_name][0] : 0;
    $total       = $published + $unpublished;
    $grand_total += $total;

    $rows[] = array(
      check_plain($type->name),
      $type_name,
      $published   > 0 ? $published   : '—',
      $unpublished > 0 ? $unpublished : '—',
      array('data' => $total, 'class' => array('arch-total')),
    );
  }

  // Totals row.
  $rows[] = array(
    array('data' => '<strong>' . t('Total') . '</strong>', 'header' => TRUE),
    '',
    '',
    '',
    array('data' => '<strong>' . $grand_total . '</strong>', 'header' => TRUE),
  );

  $section['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No content has been created yet.'),
  );

  $section['links'] = _dashboard_plus_arch_links(array(
    array(t('Manage content'), 'admin/content'),
    array(t('Manage content types'), 'admin/structure/types'),
    array(t('Add content'), 'node/add'),
  ));

  return $section;
}

// =============================================================================
// Section: Data Model
// =============================================================================

/**
 * Builds the Data Model section fieldset.
 */
function _dashboard_plus_arch_section_data_model() {
  $section = _dashboard_plus_arch_fieldset(t('Data Model'), 'data_model');

  $types = node_type_get_types();

  if (empty($types)) {
    $section['empty'] = array('#markup' => '<p>' . t('No content types defined.') . '</p>');
    return $section;
  }

  // Pre-build a map of field_name => array of type labels that use it,
  // so we can identify shared fields.
  $field_type_map = array();
  foreach ($types as $type_name => $type) {
    $instances = field_info_instances('node', $type_name);
    foreach ($instances as $field_name => $instance) {
      $field_type_map[$field_name][] = check_plain($type->name);
    }
  }

  foreach ($types as $type_name => $type) {
    $instances = field_info_instances('node', $type_name);

    $section[$type_name] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($type->name) . ' <small>(' . $type_name . ')</small>',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    if (empty($instances)) {
      $section[$type_name]['empty'] = array(
        '#markup' => '<p>' . t('No fields attached.') . '</p>',
      );
      continue;
    }

    $header = array(
      t('Field label'),
      t('Machine name'),
      t('Field type'),
      t('Required'),
      t('Also used by'),
    );

    $rows = array();
    foreach ($instances as $field_name => $instance) {
      $field_info = field_info_field($field_name);
      $field_type = isset($field_info['type']) ? $field_info['type'] : t('Unknown');

      // Other content types that share this field.
      $shared_with = array_diff($field_type_map[$field_name], array(check_plain($type->name)));
      $shared_label = empty($shared_with) ? '—' : implode(', ', $shared_with);

      $rows[] = array(
        check_plain($instance['label']),
        $field_name,
        $field_type,
        !empty($instance['required']) ? t('Yes') : '—',
        $shared_label,
      );
    }

    $section[$type_name]['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }

  // Shared fields summary.
  $shared = array();
  foreach ($field_type_map as $field_name => $type_labels) {
    if (count($type_labels) > 1) {
      $shared[$field_name] = $type_labels;
    }
  }

  if (!empty($shared)) {
    $section['shared_heading'] = array(
      '#markup' => '<h3>' . t('Shared fields') . '</h3><p>' . t('These fields are attached to more than one content type.') . '</p>',
    );

    $header = array(t('Field name'), t('Used by'));
    $rows = array();
    foreach ($shared as $field_name => $type_labels) {
      $rows[] = array($field_name, implode(', ', $type_labels));
    }
    $section['shared_table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }

  $section['links'] = _dashboard_plus_arch_links(array(
    array(t('Manage fields report'), 'admin/reports/fields'),
    array(t('Manage content types'), 'admin/structure/types'),
  ));

  return $section;
}

// =============================================================================
// Section: Structure
// =============================================================================

/**
 * Builds the Structure section fieldset.
 */
function _dashboard_plus_arch_section_structure() {
  $section = _dashboard_plus_arch_fieldset(t('Structure'), 'structure');

  // --- Menus ---
  $section['menus_heading'] = array('#markup' => '<h3>' . t('Menus') . '</h3>');

  $menus = menu_get_menus();
  if ($menus) {
    $counts = db_query('SELECT menu_name, COUNT(*) AS count FROM {menu_links} WHERE hidden = 0 GROUP BY menu_name')->fetchAllKeyed();
    $header = array(t('Menu'), t('Machine name'), t('Items'));
    $rows = array();
    foreach ($menus as $menu_name => $menu_title) {
      $rows[] = array(
        check_plain($menu_title),
        $menu_name,
        isset($counts[$menu_name]) ? (int) $counts[$menu_name] : 0,
      );
    }
    $section['menus'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows);
  }
  else {
    $section['menus'] = array('#markup' => '<p>' . t('No menus found.') . '</p>');
  }

  // --- Taxonomies ---
  if (module_exists('taxonomy')) {
    $section['tax_heading'] = array('#markup' => '<h3>' . t('Taxonomy vocabularies') . '</h3>');

    $vocabs = taxonomy_get_vocabularies();
    if ($vocabs) {
      $term_counts = db_query('SELECT vocabulary, COUNT(*) AS count FROM {taxonomy_term_data} GROUP BY vocabulary')->fetchAllKeyed();
      $header = array(t('Vocabulary'), t('Machine name'), t('Terms'));
      $rows = array();
      foreach ($vocabs as $vocab) {
        $rows[] = array(
          check_plain($vocab->name),
          $vocab->machine_name,
          isset($term_counts[$vocab->machine_name]) ? (int) $term_counts[$vocab->machine_name] : 0,
        );
      }
      $section['taxonomies'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows);
    }
    else {
      $section['taxonomies'] = array('#markup' => '<p>' . t('No vocabularies defined.') . '</p>');
    }
  }

  // --- Views ---
  if (module_exists('views')) {
    $section['views_heading'] = array('#markup' => '<h3>' . t('Views') . '</h3>');

    $views = views_get_all_views();
    if ($views) {
      $header = array(t('View name'), t('Machine name'), t('Status'), t('Displays'));
      $rows = array();
      foreach ($views as $view) {
        $display_count = isset($view->display) ? count($view->display) : 0;
        $status = $view->disabled ? t('Disabled') : t('Enabled');
        $rows[] = array(
          check_plain($view->human_name),
          $view->name,
          $status,
          $display_count,
        );
      }
      $section['views'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows);
    }
    else {
      $section['views'] = array('#markup' => '<p>' . t('No views defined.') . '</p>');
    }
  }

  // --- Layouts ---
  if (module_exists('layout')) {
    $section['layouts_heading'] = array('#markup' => '<h3>' . t('Layouts') . '</h3>');

    $layouts = layout_load_all();
    if ($layouts) {
      $header = array(t('Layout name'), t('Path'), t('Template'), t('Status'));
      $rows = array();
      foreach ($layouts as $layout) {
        $path = $layout->getPath();
        $rows[] = array(
          check_plain($layout->title),
          $path ? $path : t('(default)'),
          $layout->layout_template,
          $layout->disabled ? t('Disabled') : t('Active'),
        );
      }
      $section['layouts'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows);
    }
    else {
      $section['layouts'] = array('#markup' => '<p>' . t('No layouts found.') . '</p>');
    }
  }

  $admin_links = array(
    array(t('Manage menus'), 'admin/structure/menu'),
    array(t('Manage layouts'), 'admin/structure/layouts'),
  );
  if (module_exists('taxonomy')) {
    $admin_links[] = array(t('Manage taxonomies'), 'admin/structure/taxonomy');
  }
  if (module_exists('views')) {
    $admin_links[] = array(t('Manage views'), 'admin/structure/views');
  }
  $section['links'] = _dashboard_plus_arch_links($admin_links);

  return $section;
}

// =============================================================================
// Section: Blocks
// =============================================================================

/**
 * Builds the Blocks section fieldset.
 */
function _dashboard_plus_arch_section_blocks() {
  $section = _dashboard_plus_arch_fieldset(t('Blocks'), 'blocks');

  module_load_include('inc', 'layout', 'layout.admin');

  $block_info  = layout_get_block_info();
  $block_usage = layout_get_block_usage();

  // --- Blocks currently placed in layouts ---
  $section['placed_heading'] = array(
    '#markup' => '<h3>' . t('Blocks placed in layouts') . '</h3>',
  );

  $placed_rows = array();
  foreach ($block_usage as $module => $deltas) {
    foreach ($deltas as $delta => $layouts) {
      $label = isset($block_info[$module][$delta]['info'])
        ? check_plain($block_info[$module][$delta]['info'])
        : check_plain($delta);

      foreach ($layouts as $layout_name => $positions) {
        $layout = layout_load($layout_name);
        $layout_title = $layout ? check_plain($layout->title) : check_plain($layout_name);
        $region_names = array_map('ucfirst', array_keys($positions));

        $placed_rows[] = array(
          $label,
          $module,
          $layout_title,
          implode(', ', $region_names),
        );
      }
    }
  }

  if ($placed_rows) {
    $header = array(t('Block'), t('Module'), t('Layout'), t('Region'));
    backdrop_sort($placed_rows, array(2 => SORT_STRING, 3 => SORT_STRING));
    $section['placed'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $placed_rows,
    );
  }
  else {
    $section['placed'] = array(
      '#markup' => '<p>' . t('No blocks are currently placed in any layout.') . '</p>',
    );
  }

  // --- Custom blocks (module = 'block') ---
  $section['custom_heading'] = array(
    '#markup' => '<h3>' . t('Custom blocks') . '</h3>',
  );

  $custom_rows = array();
  if (!empty($block_info['block'])) {
    foreach ($block_info['block'] as $delta => $block) {
      $in_use = isset($block_usage['block'][$delta]);
      $custom_rows[] = array(
        check_plain($block['info']),
        $delta,
        $in_use ? t('Yes') : t('No'),
      );
    }
  }

  if ($custom_rows) {
    $header = array(t('Block title'), t('Machine name'), t('Placed in a layout'));
    $section['custom'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $custom_rows,
    );
  }
  else {
    $section['custom'] = array(
      '#markup' => '<p>' . t('No custom blocks have been created.') . '</p>',
    );
  }

  // Link to the full core report.
  $section['full_report'] = array(
    '#markup' => '<p>' . t('For a full list of all available blocks from all modules, see the <a href="@url">Blocks report</a>.', array(
      '@url' => url('admin/reports/blocks'),
    )) . '</p>',
  );

  return $section;
}

// =============================================================================
// Section: People
// =============================================================================

/**
 * Builds the People section fieldset.
 */
function _dashboard_plus_arch_section_people() {
  $section = _dashboard_plus_arch_fieldset(t('People'), 'people');

  // User counts by role.
  // In Backdrop, roles are config objects. user_roles() returns name => label.
  // {users_roles} has columns uid and role (role machine name as string).
  $roles = user_roles(TRUE); // Exclude anonymous.

  // Count users per role from {users_roles}.
  $role_counts = db_query('SELECT role, COUNT(DISTINCT uid) AS count FROM {users_roles} GROUP BY role')->fetchAllKeyed();

  // Total registered users.
  $total_users = (int) db_query('SELECT COUNT(*) FROM {users} WHERE uid > 0')->fetchField();
  $active_users = (int) db_query('SELECT COUNT(*) FROM {users} WHERE uid > 0 AND status = 1')->fetchField();

  $section['summary'] = array(
    '#markup' => '<p>' . t('Total registered people: <strong>@total</strong> (@active active)', array(
      '@total' => $total_users,
      '@active' => $active_users,
    )) . '</p>',
  );

  $header = array(t('Role'), t('Machine name'), t('People'), t('Permissions'));
  $rows = array();
  foreach ($roles as $role_name => $role_label) {
    $count = isset($role_counts[$role_name]) ? (int) $role_counts[$role_name] : 0;
    $role_obj = user_role_load($role_name);
    $perm_count = ($role_obj && !empty($role_obj->permissions)) ? count($role_obj->permissions) : 0;

    $rows[] = array(
      check_plain($role_label),
      $role_name,
      $count,
      $perm_count,
    );
  }

  $section['roles'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No roles defined.'),
    '#caption' => t('Permission counts reflect permissions explicitly granted to each role.'),
  );

  $section['links'] = _dashboard_plus_arch_links(array(
    array(t('Manage people'), 'admin/people'),
    array(t('Manage roles'), 'admin/people/roles'),
    array(t('Manage permissions'), 'admin/people/permissions'),
  ));

  return $section;
}

// =============================================================================
// Section: Modules
// =============================================================================

/**
 * Builds the Modules section fieldset.
 */
function _dashboard_plus_arch_section_modules() {
  $section = _dashboard_plus_arch_fieldset(t('Modules'), 'modules');

  $modules = system_rebuild_module_data();

  // Filter to enabled modules only, grouped by package.
  $by_package = array();
  foreach ($modules as $name => $module) {
    if ($module->status && !empty($module->info['name'])) {
      $package = isset($module->info['package']) ? $module->info['package'] : t('Other');
      $by_package[$package][$name] = $module;
    }
  }
  ksort($by_package);

  $total = 0;
  foreach ($by_package as $package => $pkg_modules) {
    $total += count($pkg_modules);
  }

  $section['summary'] = array(
    '#markup' => '<p>' . t('@count modules currently enabled.', array('@count' => $total)) . '</p>',
  );

  $header = array(
    t('Module'),
    array('data' => t('Description'), 'class' => array(RESPONSIVE_PRIORITY_LOW)),
    array('data' => t('Version'), 'class' => array(RESPONSIVE_PRIORITY_MEDIUM)),
  );

  foreach ($by_package as $package => $pkg_modules) {
    $section['pkg_' . backdrop_clean_css_identifier($package)] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($package) . ' <small>(' . count($pkg_modules) . ')</small>',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $rows = array();
    foreach ($pkg_modules as $name => $module) {
      $rows[] = array(
        check_plain($module->info['name']),
        check_plain($module->info['description']),
        isset($module->info['version']) ? $module->info['version'] : '—',
      );
    }

    $section['pkg_' . backdrop_clean_css_identifier($package)]['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }

  $section['links'] = _dashboard_plus_arch_links(array(
    array(t('Manage modules'), 'admin/modules'),
    array(t('Available updates'), 'admin/reports/updates'),
  ));

  return $section;
}

// =============================================================================
// Section: Recipes
// =============================================================================

/**
 * Builds the Recipes section fieldset.
 */
function _dashboard_plus_arch_section_recipes() {
  $section = _dashboard_plus_arch_fieldset(t('Recipes'), 'recipes');

  if (!module_exists('recipes')) {
    $section['notice'] = array(
      '#markup' => '<p>' . t('The <a href="@url">Recipes module</a> is not enabled on this site.', array('@url' => url('admin/modules'))) . '</p>',
    );
    return $section;
  }

  $modules = system_rebuild_module_data();
  $recipes = array();
  foreach ($modules as $name => $module) {
    if (isset($module->info['package']) && $module->info['package'] === 'Recipes') {
      $recipes[$name] = $module;
    }
  }

  if (empty($recipes)) {
    $section['empty'] = array('#markup' => '<p>' . t('No recipes have been applied to this site.') . '</p>');
    return $section;
  }

  $header = array(
    t('Recipe'),
    t('Machine name'),
    array('data' => t('Version'), 'class' => array(RESPONSIVE_PRIORITY_MEDIUM)),
    array('data' => t('Description'), 'class' => array(RESPONSIVE_PRIORITY_LOW)),
    t('Status'),
  );

  $rows = array();
  foreach ($recipes as $name => $module) {
    $rows[] = array(
      check_plain($module->info['name']),
      $name,
      isset($module->info['version']) ? $module->info['version'] : '—',
      check_plain($module->info['description']),
      $module->status ? t('Enabled') : t('Disabled'),
    );
  }

  $section['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  $section['links'] = _dashboard_plus_arch_links(array(
    array(t('Manage recipes'), 'admin/structure/recipes'),
    array(t('Browse recipes online'), 'admin/structure/recipes/browse'),
  ));

  return $section;
}

// =============================================================================
// Section: Media
// =============================================================================

/**
 * Builds the Media section fieldset.
 */
function _dashboard_plus_arch_section_media() {
  $section = _dashboard_plus_arch_fieldset(t('Media'), 'media');

  // Overall file totals.
  $total_files = (int) db_query('SELECT COUNT(*) FROM {file_managed}')->fetchField();
  $total_size  = (int) db_query('SELECT SUM(filesize) FROM {file_managed}')->fetchField();

  $section['summary'] = array(
    '#markup' => '<p>' . t('Total files: <strong>@count</strong> using <strong>@size</strong> of disk space.', array(
      '@count' => $total_files,
      '@size'  => format_size($total_size),
    )) . '</p>',
  );

  // Breakdown by MIME type group (image, video, audio, application, text, other).
  $result = db_query('SELECT filemime, COUNT(*) AS count, SUM(filesize) AS size FROM {file_managed} GROUP BY filemime ORDER BY size DESC');

  $header = array(t('File type'), t('Count'), t('Disk usage'));
  $rows = array();
  foreach ($result as $row) {
    $rows[] = array(
      check_plain($row->filemime),
      (int) $row->count,
      format_size($row->size),
    );
  }

  if ($rows) {
    $section['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }
  else {
    $section['empty'] = array('#markup' => '<p>' . t('No files have been uploaded to this site.') . '</p>');
  }

  // Image styles.
  if (module_exists('image')) {
    $styles = image_styles();
    $section['styles_heading'] = array(
      '#markup' => '<h3>' . t('Image styles (@count)', array('@count' => count($styles))) . '</h3>',
    );

    if ($styles) {
      $header = array(t('Style name'), t('Machine name'), t('Effects'));
      $rows = array();
      foreach ($styles as $style) {
        $effect_labels = array();
        if (!empty($style['effects'])) {
          foreach ($style['effects'] as $effect) {
            $effect_labels[] = $effect['name'];
          }
        }
        $rows[] = array(
          check_plain($style['label']),
          $style['name'],
          $effect_labels ? implode(', ', $effect_labels) : '—',
        );
      }
      $section['styles_table'] = array(
        '#theme' => 'table',
        '#header' => $header,
        '#rows' => $rows,
      );
    }
  }

  $media_links = array(
    array(t('Manage files'), 'admin/content/files'),
  );
  if (module_exists('image')) {
    $media_links[] = array(t('Manage image styles'), 'admin/config/media/image-styles');
  }
  $section['links'] = _dashboard_plus_arch_links($media_links);

  return $section;
}

// =============================================================================
// Helpers
// =============================================================================

/**
 * Returns a fieldset render array configured for a vertical tab section.
 *
 * @param string $title
 *   Human-readable tab label.
 * @param string $key
 *   Machine-readable key matching the form element key.
 *
 * @return array
 *   Fieldset render array.
 */
function _dashboard_plus_arch_fieldset($title, $key) {
  return array(
    '#type' => 'fieldset',
    '#title' => $title,
    '#group' => 'tabs',
    '#tree' => FALSE,
  );
}

/**
 * Returns a two-column key/value table render array.
 *
 * @param array $rows
 *   Array of array($label, $value) pairs.
 *
 * @return array
 *   Table render array.
 */
function _dashboard_plus_arch_kv_table(array $rows) {
  return array(
    '#theme' => 'table',
    '#header' => array(t('Setting'), t('Value')),
    '#rows' => $rows,
  );
}

/**
 * Returns a render array of contextual admin links for a section.
 *
 * @param array $links
 *   Array of array($label, $path) pairs.
 *
 * @return array
 *   Markup render array.
 */
function _dashboard_plus_arch_links(array $links) {
  $items = array();
  foreach ($links as $link) {
    $items[] = l($link[0], $link[1]);
  }
  return array(
    '#theme' => 'item_list',
    '#items' => $items,
    '#title' => t('Related pages'),
    '#attributes' => array('class' => array('arch-links')),
  );
}

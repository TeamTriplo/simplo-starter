<?php

/**
 * @file
 * Administrative page callbacks for the matomo module.
 */

/**
 * Implements hook_admin_settings() for configuring the module.
 */
function matomo_admin_settings_form($form_state) {
  global $is_https;
  $config = config('matomo.settings');

  $form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
  );

  $form['account']['site_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Matomo site ID'),
    '#default_value' => $config->get('site_id'),
    '#size' => 15,
    '#maxlength' => 20,
    '#required' => TRUE,
    '#description' => t('The user account number is unique to the websites domain. Click the <strong>Settings</strong> link in your Matomo account, then the <strong>Websites</strong> tab and enter the appropriate site <strong>ID</strong> into this field.'),
  );
  $form['account']['url_http'] = array(
    '#type' => 'textfield',
    '#title' => t('Matomo HTTP URL'),
    '#default_value' => $config->get('url_http'),
    '#size' => 80,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#description' => t('The URL to your Matomo base directory. Example: "http://www.example.com/matomo/".'),
  );
  $form['account']['url_https'] = array(
    '#type' => 'textfield',
    '#title' => t('Matomo HTTPS URL'),
    '#default_value' => $config->get('url_https'),
    '#size' => 80,
    '#maxlength' => 255,
    '#description' => t('The URL to your Matomo base directory with SSL certificate installed. Required if you track a SSL enabled website. Example: "https://www.example.com/matomo/".'),
    '#required' => $is_https,
  );
  // Required for automated form save testing only.
  $form['account']['url_skiperror'] = array(
    '#type' => 'hidden',
    '#default_value' => FALSE,
  );

  // Visibility settings.
  $form['tracking_title'] = array(
    '#type' => 'item',
    '#title' => t('Tracking scope'),
  );
  $form['tracking'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'matomo') . '/matomo.admin.js'),
    ),
  );

  $form['tracking']['domain_tracking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Domains'),
  );

  global $cookie_domain;
  $multiple_sub_domains = array();
  foreach (array('www', 'app', 'shop') as $subdomain) {
    if (count(explode('.', $cookie_domain)) > 2 && !is_numeric(str_replace('.', '', $cookie_domain))) {
      $multiple_sub_domains[] = $subdomain . $cookie_domain;
    }
    // IP addresses or localhost.
    else {
      $multiple_sub_domains[] = $subdomain . '.example.com';
    }
  }

  $form['tracking']['domain_tracking']['domain_mode'] = array(
    '#type' => 'radios',
    '#title' => t('What are you tracking?'),
    '#options' => array(
      0 => t('A single domain (default)') . '<div class="description">' . t('Domain: @domain', array('@domain' => $_SERVER['HTTP_HOST'])) . '</div>',
      1 => t('One domain with multiple subdomains') . '<div class="description">' . t('Examples: @domains', array('@domains' => implode(', ', $multiple_sub_domains))) . '</div>',
    ),
    '#default_value' => $config->get('domain_mode'),
  );

  // Page specific visibility configurations.
  $form['tracking']['page_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pages'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $options = array(
    t('Every page except the listed pages'),
    t('The listed pages only')
  );
  $description = t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array(
    '%blog' => 'blog',
    '%blog-wildcard' => 'blog/*',
    '%front' => '<front>',
  ));

  $form['tracking']['page_vis_settings']['visibility_pages'] = array(
    '#type' => 'radios',
    '#title' => t('Add tracking to specific pages'),
    '#options' => $options,
    '#default_value' => $config->get('visibility_pages'),
  );
  $form['tracking']['page_vis_settings']['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#title_display' => 'invisible',
    '#default_value' => $config->get('pages'),
    '#description' => $description,
    '#rows' => 10,
  );

  // Render the role overview.
  $form['tracking']['role_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Roles'),
  );

  $form['tracking']['role_vis_settings']['visibility_roles'] = array(
    '#type' => 'radios',
    '#title' => t('Add tracking for specific roles'),
    '#options' => array(
      t('Add to the selected roles only'),
      t('Add to every role except the selected ones'),
    ),
    '#default_value' => $config->get('visibility_roles'),
  );

  $form['tracking']['role_vis_settings']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles'),
    '#default_value' => $config->get('roles'),
    '#options' => user_roles(),
    '#description' => t('If none of the roles are selected, all users will be tracked. If a user has any of the roles checked, that user will be tracked (or excluded, depending on the setting above).'),
  );

  // Standard tracking configurations.
  $form['tracking']['user_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Users'),
  );
  $t_permission = array('%permission' => t('Opt-in or out of tracking'));
  $form['tracking']['user_vis_settings']['custom'] = array(
    '#type' => 'radios',
    '#title' => t('Allow users to customize tracking on their account page'),
    '#options' => array(
      0 => t('No customization allowed'),
      1 => t('Tracking on by default, users with %permission permission can opt out', $t_permission),
      2 => t('Tracking off by default, users with %permission permission can opt in', $t_permission)
    ),
    '#default_value' => $config->get('custom'),
  );
  $form['tracking']['user_vis_settings']['trackuserid'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track User ID'),
    '#default_value' => $config->get('trackuserid'),
    '#description' => t('User ID enables the analysis of groups of sessions, across devices, using a unique, persistent, and non-personally identifiable ID string representing a user. <a href="@url">Learn more about the benefits of using User ID</a>.', array('@url' => 'https://matomo.org/docs/user-id/')),
  );

  // Link specific configurations.
  $form['tracking']['linktracking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Links and downloads'),
  );
  $form['tracking']['linktracking']['trackmailto'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track clicks on mailto links'),
    '#default_value' => $config->get('trackmailto'),
  );
  $form['tracking']['linktracking']['track'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track clicks on outbound links and downloads (clicks on file links) for the following extensions'),
    '#default_value' => $config->get('track'),
  );
  $form['tracking']['linktracking']['trackfiles_extensions'] = array(
    '#title' => t('List of download file extensions'),
    '#title_display' => 'invisible',
    '#type' => 'textfield',
    '#default_value' => $config->get('trackfiles_extensions'),
    '#description' => t('A file extension list separated by the | character that will be tracked when clicked. Regular expressions are supported. For example: !extensions', array('!extensions' => MATOMO_TRACKFILES_EXTENSIONS)),
    '#maxlength' => 500,
    '#states' => array(
      'enabled' => array(
        ':input[name="track"]' => array('checked' => TRUE),
      ),
      # Note: Form required marker is not visible as title is invisible.
      'required' => array(
        ':input[name="track"]' => array('checked' => TRUE),
      ),
    ),
  );

  $colorbox_dependencies = '<div class="admin-requirements">';
  $colorbox_dependencies .= t('Requires: !module-list', array('!module-list' => (module_exists('colorbox') ? t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => 'Colorbox')) : t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => 'Colorbox')))));
  $colorbox_dependencies .= '</div>';

  $form['tracking']['linktracking']['trackcolorbox'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track content in colorbox modal dialogs'),
    '#default_value' => $config->get('trackcolorbox'),
    '#description' => t('Enable to track the content shown in colorbox modal windows.') . $colorbox_dependencies,
    '#disabled' => (module_exists('colorbox') ? FALSE : TRUE),
  );

  // Message specific configurations.
  $form['tracking']['messagetracking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messages'),
  );
  $form['tracking']['messagetracking']['trackmessages'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Track messages of type'),
    '#default_value' => $config->get('trackmessages'),
    '#description' => t('This will track the selected message types shown to users. Tracking of form validation errors may help you identifying usability issues in your site. Every message is tracked as one individual event. Messages from excluded pages cannot be tracked.'),
    '#options' => array(
      'status' => t('Status message'),
      'warning' => t('Warning message'),
      'error' => t('Error message'),
    ),
  );

  $form['tracking']['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search'),
  );

  $site_search_dependencies = '<div class="admin-requirements">';
  $site_search_dependencies .= t('Requires: !module-list', array('!module-list' => (module_exists('search') ? t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => 'Search')) : t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => 'Search')))));
  $site_search_dependencies .= '</div>';

  $form['tracking']['search']['site_search'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track internal search'),
    '#description' => t('If checked, internal search keywords are tracked.') . $site_search_dependencies,
    '#default_value' => $config->get('site_search'),
    '#disabled' => (module_exists('search') ? FALSE : TRUE),
  );

  // Privacy specific configurations.
  $form['tracking']['privacy'] = array(
    '#type' => 'fieldset',
    '#title' => t('Privacy'),
  );
  $form['tracking']['privacy']['privacy_donottrack'] = array(
    '#type' => 'checkbox',
    '#title' => t('Universal web tracking opt-out'),
    '#description' => t('If enabled and your Matomo server receives the <a href="@donottrack">Do-Not-Track</a> header from the client browser, the Matomo server will not track the user. Compliance with Do Not Track could be purely voluntary, enforced by industry self-regulation, or mandated by state or federal law. Please accept your visitors privacy. If they have opt-out from tracking and advertising, you should accept their personal decision.', array('@donottrack' => 'https://www.eff.org/issues/do-not-track')),
    '#default_value' => $config->get('privacy_donottrack'),
  );

  // Matomo page title tree view settings.
  $form['page_title_hierarchy'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page titles hierarchy'),
    '#description' => t('This functionality enables a dynamically expandable tree view of your site page titles in your Matomo statistics. See in Matomo statistics under <em>Actions</em> > <em>Page titles</em>.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['page_title_hierarchy']['page_title_hierarchy'] = array(
    '#type' => 'checkbox',
    '#title' => t("Show page titles as hierarchy like breadcrumbs"),
    '#description' => t('By default Matomo tracks the current page title and shows you a flat list of the most popular titles. This enables a breadcrumbs like tree view.'),
    '#default_value' => $config->get('page_title_hierarchy'),
  );
  $form['page_title_hierarchy']['page_title_hierarchy_exclude_home'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide home page from hierarchy'),
    '#description' => t('If enabled, the "Home" item will be removed from the hierarchy to flatten the structure in the Matomo statistics. Hits to the home page will still be counted, but for other pages the hierarchy will start at level Home+1.'),
    '#default_value' => $config->get('page_title_hierarchy_exclude_home'),
  );

  $form['custom_var'] = array(
    '#collapsible' => TRUE,
  	'#collapsed' => TRUE,
    '#description' => t('You can add Matomos <a href="!custom_var_documentation">Custom Variables</a> here. These will be added to every page that Matomo tracking code appears on. Custom variable names and values are limited to 200 characters in length. Keep the names and values as short as possible and expect long values to get trimmed. You may use tokens in custom variable names and values. Global and user tokens are always available; on node pages, node tokens are also available.', array('!custom_var_documentation' => 'https://matomo.org/docs/custom-variables/')),
    '#theme' => 'matomo_admin_custom_var_table',
    '#title' => t('Custom variables'),
    '#tree' => TRUE,
    '#type' => 'fieldset',
  );

  $matomo_custom_vars = $config->get('custom_var');

  // Matomo supports up to 5 custom variables.
  for ($i = 1; $i < 6; $i++) {
    $form['custom_var']['slots'][$i]['slot'] = array(
      '#default_value' => $i,
      '#description' => t('Slot number'),
      '#disabled' => TRUE,
      '#size' => 1,
      '#title' => t('Custom variable slot #@slot', array('@slot' => $i)),
      '#title_display' => 'invisible',
      '#type' => 'textfield',
    );
    $form['custom_var']['slots'][$i]['name'] = array(
      '#default_value' => !empty($matomo_custom_vars['slots'][$i]['name']) ? $matomo_custom_vars['slots'][$i]['name'] : '',
      '#description' => t('The custom variable name.'),
      '#maxlength' => 100,
    	'#size' => 20,
      '#title' => t('Custom variable name #@slot', array('@slot' => $i)),
      '#title_display' => 'invisible',
      '#type' => 'textfield',
    );
    $form['custom_var']['slots'][$i]['value'] = array(
      '#default_value' => !empty($matomo_custom_vars['slots'][$i]['value']) ? $matomo_custom_vars['slots'][$i]['value'] : '',
      '#description' => t('The custom variable value.'),
      '#maxlength' => 255,
      '#title' => t('Custom variable value #@slot', array('@slot' => $i)),
      '#title_display' => 'invisible',
      '#type' => 'textfield',
      '#element_validate' => array('token_element_validate'),
      '#token_types' => array('node'),
    );
    $form['custom_var']['slots'][$i]['value']['#element_validate'][] = 'token_element_validate';

    $form['custom_var']['slots'][$i]['scope'] = array(
      '#default_value' => !empty($matomo_custom_vars['slots'][$i]['scope']) ? $matomo_custom_vars['slots'][$i]['scope'] : 'visit',
      '#description' => t('The scope for the custom variable.'),
      '#title' => t('Custom variable slot #@slot', array('@slot' => $i)),
      '#title_display' => 'invisible',
      '#type' => 'select',
      '#options' => array(
        'visit' => t('Visit'),
        'page' => t('Page'),
      ),
    );
  }

  $form['custom_var']['custom_var_description'] = array(
    '#type' => 'item',
    '#description' => t('You can supplement Matomos\' basic IP address tracking of visitors by segmenting users based on custom variables. Make sure you will not associate (or permit any third party to associate) any data gathered from your websites (or such third parties\' websites) with any personally identifying information from any source as part of your use (or such third parties\' use) of the Matomo\' service.'),
  );
  $form['custom_var']['custom_var_token_tree'] = array(
    '#theme' => 'token_tree',
    '#token_types' => array('node'),
    '#dialog' => TRUE,
  );

  // Advanced feature configurations.
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['advanced']['cache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Locally cache tracking code file'),
    '#description' => t('If checked, the tracking code file is retrieved from your Matomo site and cached locally. It is updated daily to ensure updates to tracking code are reflected in the local copy.'),
    '#default_value' => $config->get('cache'),
  );

  // Allow for tracking of the originating node when viewing translation sets.
  if (module_exists('translation')) {
    $form['advanced']['translation_set'] = array(
      '#type' => 'checkbox',
      '#title' => t('Track translation sets as one unit'),
      '#description' => t('When a node is part of a translation set, record statistics for the originating node instead. This allows for a translation set to be treated as a single unit.'),
      '#default_value' => $config->get('translation_set'),
    );
  }

  $form['advanced']['noscript'] = array(
    '#type' => 'checkbox',
    '#title' => t('Also track when JavaScript is disabled'),
    '#description' => t('If checked, a <em>noscript</em> fallback will be added to the page, to also track when JavaScript is disabled in a browser.'),
    '#default_value' => $config->get('noscript'),
  );

  // Code snippet settings.
  $user_access_add_js_snippets = !user_access('add js snippets for matomo');
  $user_access_add_js_snippets_permission_warning = $user_access_add_js_snippets ? ' <em>' . t('This field has been disabled because you do not have sufficient permissions to edit it.') . '</em>' : '';
  $form['advanced']['codesnippet'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom JavaScript code'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('You can add custom Matomo <a href="@snippets">code snippets</a> here. These will be added to every page that Matomo appears on. <strong>Do not include the &lt;script&gt; tags</strong>, and always end your code with a semicolon (;).', array('@snippets' => 'https://matomo.org/docs/javascript-tracking/'))
  );
  $form['advanced']['codesnippet']['codesnippet_before'] = array(
    '#type' => 'textarea',
    '#title' => t('Code snippet (before)'),
    '#default_value' => $config->get('codesnippet_before'),
    '#disabled' => $user_access_add_js_snippets,
    '#rows' => 5,
    '#description' => t('Code in this textarea will be added <strong>before</strong> _paq.push(["trackPageView"]).')
  );
  $form['advanced']['codesnippet']['codesnippet_after'] = array(
    '#type' => 'textarea',
    '#title' => t('Code snippet (after)'),
    '#default_value' => $config->get('codesnippet_after'),
    '#disabled' => $user_access_add_js_snippets,
    '#rows' => 5,
    '#description' => t('Code in this textarea will be added <strong>after</strong> _paq.push(["trackPageView"]). This is useful if you\'d like to track a site in two accounts.')
  );

  $form['advanced']['js_scope'] = array(
    '#type' => 'select',
    '#title' => t('JavaScript scope'),
    '#description' => t("Matomo recommends adding the tracking code to the header for performance reasons."),
    '#options' => array(
      'footer' => t('Footer'),
      'header' => t('Header'),
    ),
    '#default_value' => $config->get('js_scope'),
  );

  $form['#config'] = 'matomo.settings';
  $form['#submit'] = array('_matomo_admin_flush_cache');

  return system_settings_form($form);
}

/**
 * Additional callback on admin form submit.
 *
 * Flush the page cache as the tracking code visibility could have changed.
 */
function _matomo_admin_flush_cache() {
  cache_flush('page');
}

function matomo_admin_settings_form_validate($form, &$form_state) {
  // Custom variables validation.
  foreach ($form_state['values']['custom_var']['slots'] as $custom_var) {
    $form_state['values']['custom_var']['slots'][$custom_var['slot']]['name'] = trim($custom_var['name']);
    $form_state['values']['custom_var']['slots'][$custom_var['slot']]['value'] = trim($custom_var['value']);

    // Validate empty names/values.
    if (empty($custom_var['name']) && !empty($custom_var['value'])) {
      form_set_error("custom_var][slots][" . $custom_var['slot'] . "][name", t('The custom variable @slot-number requires a <em>Name</em> if a <em>Value</em> has been provided.', array('@slot-number' => $custom_var['slot'])));
    }
    elseif (!empty($custom_var['name']) && empty($custom_var['value'])) {
      form_set_error("custom_var][slots][" . $custom_var['slot'] . "][value", t('The custom variable @slot-number requires a <em>Value</em> if a <em>Name</em> has been provided.', array('@slot-number' => $custom_var['slot'])));
    }
  }

  // Trim some text area values.
  $form_state['values']['site_id'] = trim($form_state['values']['site_id']);
  $form_state['values']['pages'] = trim($form_state['values']['pages']);
  $form_state['values']['codesnippet_before'] = trim($form_state['values']['codesnippet_before']);
  $form_state['values']['codesnippet_after'] = trim($form_state['values']['codesnippet_after']);

  $matomo_url = $form_state['values']['url_http'];
  if ('/' != backdrop_substr($matomo_url, -1, 1)) {
    $matomo_url = $matomo_url . '/';
    $form_state['values']['url_http'] = $matomo_url;
  }
  $url = $matomo_url . 'matomo.php';
  $result = backdrop_http_request($url);
  if ($result->code != 200 && $form_state['values']['url_skiperror'] == FALSE) {
    form_set_error('url_http', t('The validation of "@url" failed with error "@error" (HTTP code @code).', array('@url' => check_url($url), '@error' => $result->error, '@code' => $result->code)));
  }

  if (!empty($form_state['values']['url_https'])) {
    $url = $form_state['values']['url_https'];
    if ('/' != backdrop_substr($matomo_url, -1, 1)) {
      $matomo_url = $matomo_url . '/';
      $form_state['values']['url_https'] = $matomo_url;
    }
    $url = $matomo_url . 'matomo.php';
    $result = backdrop_http_request($url);
    if ($result->code != 200 && $form_state['values']['url_skiperror'] == FALSE) {
      form_set_error('url_https', t('The validation of "@url" failed with error "@error" (HTTP code @code).', array('@url' => check_url($url), '@error' => $result->error, '@code' => $result->code)));
    }
  }

  // Delete obsolete local cache file.
  if (empty($form_state['values']['cache']) && $form['advanced']['cache']['#default_value']) {
    matomo_clear_js_cache();
  }

  // This is for the Newbie's who cannot read a text area description.
  if (preg_match('/(.*)<\/?script(.*)>(.*)/i', $form_state['values']['codesnippet_before'])) {
    form_set_error('codesnippet_before', t('Do not include the &lt;script&gt; tags in the javascript code snippets.'));
  }
  if (preg_match('/(.*)<\/?script(.*)>(.*)/i', $form_state['values']['codesnippet_after'])) {
    form_set_error('codesnippet_after', t('Do not include the &lt;script&gt; tags in the javascript code snippets.'));
  }
}

/**
 * Layout for the custom variables table in the admin settings form.
 *
 * @param array $variables
 *   An array contains the form elements.
 *
 * @return string
 *   The rendered output.
 */
function theme_matomo_admin_custom_var_table($variables) {
  $form = $variables['form'];

  $header = array(
    array('data' => t('Slot')),
    array('data' => t('Name')),
    array('data' => t('Value')),
    array('data' => t('Scope')),
  );

  $rows = array();
  foreach (element_children($form['slots']) as $key => $id) {
    $rows[] = array(
      'data' => array(
        backdrop_render($form['slots'][$id]['slot']),
        backdrop_render($form['slots'][$id]['name']),
        backdrop_render($form['slots'][$id]['value']),
        backdrop_render($form['slots'][$id]['scope']),
      ),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= backdrop_render($form['custom_var_description']);
  $output .= backdrop_render($form['custom_var_token_tree']);

  return $output;
}

/**
 * Validate a form element that should have tokens in it.
 *
 * For example:
 * @code
 * $form['my_node_text_element'] = array(
 *   '#type' => 'textfield',
 *   '#title' => t('Some text to token-ize that has a node context.'),
 *   '#default_value' => 'The title of this node is [node:title].',
 *   '#element_validate' => array('matomo_token_element_validate'),
 * );
 * @endcode
 */
function matomo_token_element_validate(&$element, &$form_state) {
  $value = isset($element['#value']) ? $element['#value'] : $element['#default_value'];

  if (!backdrop_strlen($value)) {
    // Empty value needs no further validation since the element should depend
    // on using the '#required' FAPI property.
    return $element;
  }

  $tokens = token_scan($value);
  $invalid_tokens = _matomo_get_forbidden_tokens($tokens);
  if ($invalid_tokens) {
    form_error($element, t('The %element-title is using the following forbidden tokens with personal identifying information: @invalid-tokens.', array('%element-title' => $element['#title'], '@invalid-tokens' => implode(', ', $invalid_tokens))));
  }

  return $element;
}

/**
 * @param array $value
 *   An array of token values.
 *
 * @return array
 *   A unique array of invalid tokens.
 */
function _matomo_get_forbidden_tokens($value) {
  $invalid_tokens = array();
  $value_tokens = is_string($value) ? token_scan($value) : $value;

  foreach ($value_tokens as $type => $tokens) {
    if (array_filter($tokens, '_contains_forbidden_token')) {
      $invalid_tokens = array_merge($invalid_tokens, array_values($tokens));
    }
  }

  array_unique($invalid_tokens);
  return $invalid_tokens;
}

/**
 * Validate if a string contains forbidden tokens not allowed by privacy rules.
 *
 * @param string $token_string
 *   A string with one or more tokens to be validated.
 *
 * @return boolean
 *   TRUE if blacklisted token has been found, otherwise FALSE.
 */
function _matomo_contains_forbidden_token($token_string) {
  // List of strings in tokens with personal identifying information not allowed
  // for privacy reasons. See section 8.1 of the Google Analytics terms of use
  // for more detailed information.
  //
  // This list can never ever be complete. For this reason it tries to use a
  // regex and may kill a few other valid tokens, but it's the only way to
  // protect users as much as possible from admins with illegal ideas.
  //
  // User tokens are not prefixed with colon to catch 'current-user' and 'user'.
  //
  // TODO: If someone have better ideas, share them, please!
  $token_blacklist = array(
    ':author]',
    ':author:edit-url]',
    ':author:url]',
    ':author:path]',
    ':current-user]',
    ':current-user:original]',
    ':fid]',
    ':mail]',
    ':name]',
    ':uid]',
    ':one-time-login-url]',
    ':owner]',
    ':owner:cancel-url]',
    ':owner:edit-url]',
    ':owner:url]',
    ':owner:path]',
    'user:cancel-url]',
    'user:edit-url]',
    'user:url]',
    'user:path]',
    'user:picture]',
    // addressfield_tokens.module
    ':first-name]',
    ':last-name]',
    ':name-line]',
    ':mc-address]',
    ':thoroughfare]',
    ':premise]',
    // realname.module
    ':name-raw]',
    // token.module
    ':ip-address]',
  );

  return preg_match('/' . implode('|', array_map('preg_quote', $token_blacklist)) . '/i', $token_string);
}

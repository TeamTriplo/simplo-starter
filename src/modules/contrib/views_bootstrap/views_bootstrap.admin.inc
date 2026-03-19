<?php
/**
 * @file
 * Contains the administration pages for views_bootstrap.
 *
 */
function views_bootstrap_settings($form, &$form_state) {
  $form['#config'] = 'views_bootstrap.settings';
  $config = config('views_bootstrap.settings');
  $form['bootstrap_assets'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bootstrap Assets'),
  );

  $form['bootstrap_assets']['include_bootstrap_assets'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include Bootstrap CSS & JS when used by Views'),
    '#default_value' => $config->get('include_bootstrap_assets'),
    '#description' => t('Select this option if your site does not use a Bootstrap based theme; it will include the necessary Bootstrap libraries for Views using Views Bootstrap settings. If your site theme already includes Bootstrap assets, then this setting should be disabled (unchecked) to prevent including multiple (and possibly incompatible) Bootstrap libraries.'),
  );

  $form['bootstrap_assets']['bootstrap_version'] = array(
    '#type' => 'select',
    '#title' => t('Version'),
    '#description' => t('Select the version of Bootstrap to use from the Bootstrap CDN or you can use the library integrated within the module.'),
    '#options' => views_bootstrap_version_options(),
    '#default_value' => $config->get('bootstrap_version'),
    '#states' => array(
      'visible' => array(
        ':input[name="include_bootstrap_assets"]' => array('checked' => TRUE),
      ),
    ),
  );

  return system_settings_form($form);
}

/**
 * Returns an array of supported Bootstrap versions.
 */
function views_bootstrap_version_options() {
  return array(
    '3.3.6' => '3.3.6 (CDN)',
    '3.3.7' => '3.3.7 (CDN)',
    '3.4.0' => '3.4.0 (CDN)',
    '3.4.1' => '3.4.1 (CDN)',
    'module' => '3.4.1 (module)',
  );
}

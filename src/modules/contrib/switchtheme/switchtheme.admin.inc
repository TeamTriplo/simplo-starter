<?php

/**
 * @file
 * Administrative functionality for Switchtheme.
 */

/**
 * Form constructor for theme settings.
 */
function switchtheme_admin_settings($form, &$form_state) {
  $options = switchtheme_options();
  $config = config_get('switchtheme.settings', 'switchtheme_name');

  foreach ($options as $name => $label) {
    $form['switchtheme'][$name] = array(
      '#type' => 'textfield',
      '#title' => $label,
      '#default_value' => (isset($config[$name])) ? $config[$name] : '',
    );
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

function switchtheme_admin_settings_submit($form, &$form_state) {
  $options = switchtheme_options();
  $config = config_get('switchtheme.settings', 'switchtheme_name');

  foreach ($options as $name => $label) {
    if (!empty($form_state['values'][$name])) {
      $config[$name] = check_plain($form_state['values'][$name]);
    }
  }

  config_set('switchtheme.settings', 'switchtheme_name', $config);
}

/**
 * Form constructor for browser settings.
 */
function switchtheme_admin_browser_settings($form, &$form_state) {
  $config = config('switchtheme.settings');
  $form['#config'] = 'switchtheme.settings';
  $form['switchtheme_browser_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Switch theme based on the browser of the visitor'),
    '#default_value' => $config->get('switchtheme_browser_enabled'),
  );

  $themes = array('default' => t('Default'));
  $themes += switchtheme_options();
  $useragents = array();
  $result = db_query('SELECT data FROM {browscap}')->fetchCol();
  foreach ($result as $data) {
    $data = unserialize($data);
    if (isset($data['parent'])) {
      $useragents[trim($data['parent'])][] = isset($data['platform']) ? $data['platform'] : '';
    }
  }
  $form['switchtheme_browser'] = array(
    '#type' => 'fieldset',
    '#title' => t('Browsers'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  foreach ($useragents as $parent => $platforms) {
    $form['switchtheme_browser']['switchtheme_browser_' . md5($parent)] = array(
      '#type' => 'select',
      '#title' => $parent,
      '#options' => $themes,
      '#default_value' => $config->get('switchtheme_browser_' . md5($parent)),
    );
  }
  return system_settings_form($form);
}


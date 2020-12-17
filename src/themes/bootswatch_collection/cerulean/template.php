<?php
/**
 * Implements hook_css_alter().
 */
function cerulean_css_alter(&$css) {
  // Add overrides.
  $theme_path = backdrop_get_path('theme', 'cerulean');
  $override = $theme_path . '/css/bootstrap.css';
  $css[$override] = array(
    'data' => $override,
    'type' => 'file',
    'every_page' => TRUE,
    'media' => 'all',
    'preprocess' => TRUE,
    'group' => CSS_THEME,
    'browsers' => array('IE' => TRUE, '!IE' => TRUE),
    'weight' => 1,
  );
}
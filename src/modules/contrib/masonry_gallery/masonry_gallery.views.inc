<?php
/**
 * @file
 * Provides views data and handlers for masonry_gallery.module.
 */

/**
 * Implements hook_views_plugins().
 */
function masonry_gallery_views_plugins() {
  $module_path = backdrop_get_path('module', 'masonry_gallery');

  return array(
    'style' => array(
      'masonry_gallery' => array(
        'title' => t('Masonry Gallery'),
        'help' => t('Displays images in a masonry grid layout.'),
        'type' => 'normal',
        'handler' => 'views_plugin_style_masonry_gallery',
        'theme' => 'views_view_masonry_gallery',
        'theme path' => $module_path,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'uses row plugin' => FALSE,
        'uses row class' => FALSE,
        'uses grouping' => FALSE,
      ),
    ),
  );
}

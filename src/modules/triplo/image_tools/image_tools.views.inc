<?php
/**
 * @file
 * Views integration for Image Tools.
 */

/**
 * Field handler: small thumbnail preview for image files.
 *
 * Reads uri and filemime directly from the query rather than loading the
 * entity, so it works reliably in any file_managed-based view.
 */
class ImageToolsViewsHandlerFieldThumbnail extends views_handler_field {

  function construct() {
    parent::construct();
    $this->additional_fields['uri']      = 'uri';
    $this->additional_fields['filemime'] = 'filemime';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $filemime = $this->get_value($values, 'filemime');
    $uri      = $this->get_value($values, 'uri');

    if (empty($uri) || strpos($filemime, 'image/') !== 0) {
      return '';
    }

    $url = check_plain(file_create_url($uri));
    return '<img src="' . $url . '" style="max-width:80px;max-height:80px;display:block;" alt="">';
  }

}

/**
 * Field handler: Crop and Resize operation links for image files.
 */
class ImageToolsViewsHandlerFieldLinks extends views_handler_field_entity {

  /**
   * {@inheritdoc}
   */
  function render($values) {
    $file = $this->get_value($values);
    if (!$file instanceof File) {
      return '';
    }
    if (strpos($file->filemime, 'image/') !== 0) {
      return '';
    }
    if (!user_access('use image tools')) {
      return '';
    }

    $back = array('query' => array('back' => current_path()));
    return l(t('Crop'), 'admin/files/' . $file->fid . '/crop', $back)
      . ' &middot; '
      . l(t('Resize'), 'admin/files/' . $file->fid . '/resize', $back);
  }

}

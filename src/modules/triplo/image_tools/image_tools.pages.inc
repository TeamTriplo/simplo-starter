<?php
/**
 * @file
 * Page callbacks and form builders for Image Tools.
 */

// ---------------------------------------------------------------------------
// Crop form
// ---------------------------------------------------------------------------

/**
 * Form builder: visual crop tool.
 *
 * @param object $file
 *   The file entity to crop.
 */
function image_tools_crop_form($form, &$form_state, $file) {
  if (!image_tools_is_image($file)) {
    backdrop_set_message(t('This file is not an image and cannot be cropped.'), 'error');
    backdrop_goto('admin/content/files');
  }

  $form_state['storage']['file'] = $file;

  backdrop_set_title(t('Crop: @name', array('@name' => $file->filename)));

  $image_url = file_create_url($file->uri);
  $image_info = image_get_info(backdrop_realpath($file->uri));
  $dimensions = $image_info
    ? t('@w × @h px', array('@w' => $image_info['width'], '@h' => $image_info['height']))
    : '';

  $form['#attached']['js'][]  = backdrop_get_path('module', 'image_tools') . '/js/cropper.min.js';
  $form['#attached']['js'][]  = backdrop_get_path('module', 'image_tools') . '/js/image_tools.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'image_tools') . '/css/cropper.min.css';
  $form['#attached']['css'][] = backdrop_get_path('module', 'image_tools') . '/css/image_tools.css';
  $form['#attached']['js'][] = array(
    'data' => array('imageTools' => array(
      'fid'           => $file->fid,
      'mode'          => 'crop',
      'imageUrl'      => $image_url,
      'naturalWidth'  => $image_info ? (int) $image_info['width']  : 0,
      'naturalHeight' => $image_info ? (int) $image_info['height'] : 0,
    )),
    'type' => 'setting',
  );

  $back_url = isset($_GET['back']) ? $_GET['back'] : 'file/' . $file->fid;

  $form['header'] = array(
    '#markup' => '<div class="image-tools-header">'
      . '<span class="image-tools-dimensions">' . check_plain($dimensions) . '</span>'
      . ' &nbsp; ' . l(t('← Back'), $back_url, array('attributes' => array('class' => array('image-tools-back'))))
      . '</div>',
  );

  $form['image_wrapper'] = array(
    '#markup' => '<div class="image-tools-crop-wrapper">'
      . '<img id="image-tools-crop-image" src="' . check_plain($image_url) . '" alt="">'
      . '</div>'
      . '<div id="image-tools-crop-info" class="image-tools-crop-info">'
      . t('Drag to select a crop area.')
      . '</div>',
  );

  // Hidden fields populated by JS before submit.
  $form['crop_x']      = array('#type' => 'hidden', '#default_value' => 0);
  $form['crop_y']      = array('#type' => 'hidden', '#default_value' => 0);
  $form['crop_width']  = array('#type' => 'hidden', '#default_value' => 0);
  $form['crop_height'] = array('#type' => 'hidden', '#default_value' => 0);

  $form['options'] = array(
    '#type'  => 'container',
    '#attributes' => array('class' => array('image-tools-options')),
  );

  $form['options']['lock_aspect'] = array(
    '#type'  => 'checkbox',
    '#title' => t('Lock aspect ratio'),
  );

  $ext      = pathinfo($file->filename, PATHINFO_EXTENSION);
  $basename = pathinfo($file->filename, PATHINFO_FILENAME);

  $form['options']['save_to_library'] = array(
    '#type'  => 'checkbox',
    '#title' => t('Save to media library'),
  );
  $form['options']['new_filename'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Filename'),
    '#default_value' => $basename . '-cropped.' . $ext,
    '#states'        => array(
      'visible' => array(':input[name="save_to_library"]' => array('checked' => TRUE)),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Crop'),
  );

  return $form;
}

/**
 * Validate handler for image_tools_crop_form().
 */
function image_tools_crop_form_validate($form, &$form_state) {
  $w = (int) $form_state['values']['crop_width'];
  $h = (int) $form_state['values']['crop_height'];
  if ($w <= 0 || $h <= 0) {
    form_set_error('', t('Please drag to select a crop area before clicking Crop.'));
  }
  if (!empty($form_state['values']['save_to_library']) && empty(trim($form_state['values']['new_filename']))) {
    form_set_error('new_filename', t('Please enter a filename.'));
  }
}

/**
 * Submit handler for image_tools_crop_form().
 */
function image_tools_crop_form_submit($form, &$form_state) {
  $file = $form_state['storage']['file'];
  $x    = (int) $form_state['values']['crop_x'];
  $y    = (int) $form_state['values']['crop_y'];
  $w    = (int) $form_state['values']['crop_width'];
  $h    = (int) $form_state['values']['crop_height'];
  $save = !empty($form_state['values']['save_to_library']);

  $ext      = pathinfo($file->filename, PATHINFO_EXTENSION);
  $basename = pathinfo($file->filename, PATHINFO_FILENAME);
  $filename = $save
    ? trim($form_state['values']['new_filename'])
    : $basename . '-cropped.' . $ext;

  $image = image_load(backdrop_realpath($file->uri));
  if (!$image) {
    backdrop_set_message(t('Could not load image for processing.'), 'error');
    return;
  }

  if (!image_crop($image, $x, $y, $w, $h)) {
    backdrop_set_message(t('Crop failed. Please try again.'), 'error');
    return;
  }

  _image_tools_finish($file, $image, $filename, $save, $form_state);
}

// ---------------------------------------------------------------------------
// Resize form
// ---------------------------------------------------------------------------

/**
 * Form builder: resize tool (percentage, exact dimensions, scale & crop).
 *
 * @param object $file
 *   The file entity to resize.
 */
function image_tools_resize_form($form, &$form_state, $file) {
  if (!image_tools_is_image($file)) {
    backdrop_set_message(t('This file is not an image and cannot be resized.'), 'error');
    backdrop_goto('admin/content/files');
  }

  $form_state['storage']['file'] = $file;

  backdrop_set_title(t('Resize: @name', array('@name' => $file->filename)));

  $image_info = image_get_info(backdrop_realpath($file->uri));
  $orig_w = $image_info ? $image_info['width']  : 0;
  $orig_h = $image_info ? $image_info['height'] : 0;

  $form['#attached']['css'][] = backdrop_get_path('module', 'image_tools') . '/css/image_tools.css';

  $back_url = isset($_GET['back']) ? $_GET['back'] : 'file/' . $file->fid;

  $form['header'] = array(
    '#markup' => '<div class="image-tools-header">'
      . '<span class="image-tools-dimensions">' . t('Original: @w × @h px', array('@w' => $orig_w, '@h' => $orig_h)) . '</span>'
      . ' &nbsp; ' . l(t('← Back'), $back_url, array('attributes' => array('class' => array('image-tools-back'))))
      . '</div>',
  );

  $form['mode'] = array(
    '#type'          => 'radios',
    '#title'         => t('Resize method'),
    '#options'       => array(
      'percentage'    => t('Percentage'),
      'exact'         => t('Exact dimensions'),
      'scale_crop'    => t('Scale & crop to exact dimensions'),
    ),
    '#default_value' => 'percentage',
  );

  // Percentage.
  $form['percentage'] = array(
    '#type'          => 'number',
    '#title'         => t('Percentage'),
    '#min'           => 1,
    '#max'           => 200,
    '#default_value' => 100,
    '#field_suffix'  => '%',
    '#states'        => array(
      'visible' => array(':input[name="mode"]' => array('value' => 'percentage')),
    ),
  );

  // Exact / scale & crop dimensions.
  $form['width'] = array(
    '#type'          => 'number',
    '#title'         => t('Width (px)'),
    '#min'           => 1,
    '#default_value' => $orig_w,
    '#states'        => array(
      'visible' => array(
        array(':input[name="mode"]' => array('value' => 'exact')),
        array(':input[name="mode"]' => array('value' => 'scale_crop')),
      ),
    ),
  );
  $form['height'] = array(
    '#type'          => 'number',
    '#title'         => t('Height (px)'),
    '#min'           => 1,
    '#default_value' => $orig_h,
    '#states'        => array(
      'visible' => array(
        array(':input[name="mode"]' => array('value' => 'exact')),
        array(':input[name="mode"]' => array('value' => 'scale_crop')),
      ),
    ),
  );

  $ext      = pathinfo($file->filename, PATHINFO_EXTENSION);
  $basename = pathinfo($file->filename, PATHINFO_FILENAME);

  $form['save_to_library'] = array(
    '#type'  => 'checkbox',
    '#title' => t('Save to media library'),
  );
  $form['new_filename'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Filename'),
    '#default_value' => $basename . '-resized.' . $ext,
    '#states'        => array(
      'visible' => array(':input[name="save_to_library"]' => array('checked' => TRUE)),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Resize'),
  );

  return $form;
}

/**
 * Validate handler for image_tools_resize_form().
 */
function image_tools_resize_form_validate($form, &$form_state) {
  $mode = $form_state['values']['mode'];
  if ($mode === 'exact' || $mode === 'scale_crop') {
    if (empty($form_state['values']['width']) || (int) $form_state['values']['width'] <= 0) {
      form_set_error('width', t('Please enter a valid width.'));
    }
    if (empty($form_state['values']['height']) || (int) $form_state['values']['height'] <= 0) {
      form_set_error('height', t('Please enter a valid height.'));
    }
  }
  if (!empty($form_state['values']['save_to_library']) && empty(trim($form_state['values']['new_filename']))) {
    form_set_error('new_filename', t('Please enter a filename.'));
  }
}

/**
 * Submit handler for image_tools_resize_form().
 */
function image_tools_resize_form_submit($form, &$form_state) {
  $file = $form_state['storage']['file'];
  $mode = $form_state['values']['mode'];
  $save = !empty($form_state['values']['save_to_library']);

  $ext      = pathinfo($file->filename, PATHINFO_EXTENSION);
  $basename = pathinfo($file->filename, PATHINFO_FILENAME);
  $filename = $save
    ? trim($form_state['values']['new_filename'])
    : $basename . '-resized.' . $ext;

  $image = image_load(backdrop_realpath($file->uri));
  if (!$image) {
    backdrop_set_message(t('Could not load image for processing.'), 'error');
    return;
  }

  $success = FALSE;
  if ($mode === 'percentage') {
    $pct     = (int) $form_state['values']['percentage'];
    $new_w   = max(1, (int) round($image->info['width']  * $pct / 100));
    $new_h   = max(1, (int) round($image->info['height'] * $pct / 100));
    $success = image_resize($image, $new_w, $new_h);
  }
  elseif ($mode === 'exact') {
    $success = image_resize($image, (int) $form_state['values']['width'], (int) $form_state['values']['height']);
  }
  elseif ($mode === 'scale_crop') {
    $success = image_scale_and_crop($image, (int) $form_state['values']['width'], (int) $form_state['values']['height']);
  }

  if (!$success) {
    backdrop_set_message(t('Resize failed. Please try again.'), 'error');
    return;
  }

  _image_tools_finish($file, $image, $filename, $save, $form_state);
}

// ---------------------------------------------------------------------------
// Download callback
// ---------------------------------------------------------------------------

/**
 * Page callback: stream a session-prepared file download.
 *
 * @param object $file
 *   The original file entity (used for access and redirect fallback).
 */
function image_tools_download_ready($file) {
  if (empty($_SESSION['image_tools_download'])) {
    backdrop_set_message(t('No file ready for download.'), 'warning');
    backdrop_goto('admin/content/files');
  }

  $info      = $_SESSION['image_tools_download'];
  $real_path = backdrop_realpath($info['path']);

  if (!file_exists($real_path)) {
    backdrop_set_message(t('The prepared file could not be found. Please try again.'), 'error');
    unset($_SESSION['image_tools_download']);
    backdrop_goto('admin/content/files');
  }

  backdrop_add_http_header('Content-Type', $info['mime']);
  backdrop_add_http_header('Content-Disposition', 'attachment; filename="' . $info['filename'] . '"');
  backdrop_add_http_header('Content-Length', filesize($real_path));
  ob_end_clean();
  readfile($real_path);

  if (!empty($info['temp'])) {
    file_unmanaged_delete($info['path']);
  }

  unset($_SESSION['image_tools_download']);
  session_write_close();
  exit();
}

// ---------------------------------------------------------------------------
// Shared helper
// ---------------------------------------------------------------------------

/**
 * Saves a processed image and sets a download link or library message.
 *
 * @param object $file
 *   The original file entity.
 * @param object $image
 *   The processed Backdrop image object (already cropped/resized).
 * @param string $filename
 *   The output filename (basename only).
 * @param bool $save_to_library
 *   TRUE to save as a new managed file in the media library.
 * @param array $form_state
 *   The form state (redirect is set on this).
 */
function _image_tools_finish($file, $image, $filename, $save_to_library, &$form_state) {
  $temp_dir = 'public://image-tools-temp';
  $temp_uri = $temp_dir . '/' . $filename;

  $dir = backdrop_realpath($temp_dir);
  file_prepare_directory($dir, FILE_CREATE_DIRECTORY);

  if (!image_save($image, backdrop_realpath($temp_uri))) {
    backdrop_set_message(t('Could not save processed image.'), 'error');
    return;
  }

  if ($save_to_library) {
    $dest_uri = 'public://' . $filename;
    $saved_uri = file_unmanaged_copy($temp_uri, $dest_uri, FILE_EXISTS_RENAME);
    file_unmanaged_delete($temp_uri);

    if (!$saved_uri) {
      backdrop_set_message(t('Could not save file to media library.'), 'error');
      return;
    }

    $saved_file = entity_create('file', array(
      'uri'      => $saved_uri,
      'filename' => backdrop_basename($saved_uri),
      'filemime' => $file->filemime,
      'status'   => FILE_STATUS_PERMANENT,
    ));
    file_save($saved_file);

    backdrop_set_message(t('Saved as %filename in the media library. !link', array(
      '%filename' => $saved_file->filename,
      '!link'     => l(t('View files'), 'admin/content/files'),
    )));
  }
  else {
    $_SESSION['image_tools_download'] = array(
      'path'     => $temp_uri,
      'filename' => $filename,
      'mime'     => $file->filemime,
      'temp'     => TRUE,
    );

    $fid          = $file->fid;
    $current_path = current_path();
    $download_url = url('admin/files/' . $fid . '/download-ready');

    backdrop_set_message(t('Your image is ready. !link', array(
      '!link' => '<a href="' . check_plain($download_url) . '">' . t('Download') . '</a>',
    )));
  }
}

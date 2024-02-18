<?php
/**
 * @file
 * Contains the Masonry Gallery style plugin.
 */
class views_plugin_style_masonry_gallery extends views_plugin_style {

  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['gap']     = array('default' => 10);
    $options['columns'] = array(
      'default' => array(
        'auto'        => TRUE,
        'desktop'     => 5,
        'laptop'      => 4,
        'tablet'      => 3,
        'smartphone'  => 2,
        'phone'       => 1,
        'truncate'    => TRUE,
      ),
    );
    $options['caption'] = array(
      'default' => array(
        'source'       => '',
        'display'      => 'static',
        'height'       => 32,
        'no_wrap'      => TRUE,
        'no_padding_h' => FALSE,
      ),
    );
    $options['advanced'] = array(
      'default' => array(
        'threshold'    => 32,
      ),
    );

    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {

    $form['gap'] = array(
      '#title' => t('Gap'),
      '#description' => t('Amount of space between items.'),
      '#type' => 'number',
      '#min' => 0,
      '#max' => 99,
      '#field_suffix' => t('pix'),
      '#default_value' => $this->options['gap'],
    );

    $form['columns'] = array(
      '#type' => 'fieldset',
      '#title' => t('Columns settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['columns']['auto'] = array(
      '#type' => 'checkbox',
      '#title' => t('Auto columns'),
      '#description' => t("Automatically calculate number of columns for each screen size based on provided Desktop columns."),
      '#default_value' => $this->options['columns']['auto'],
    );

    $form['columns']['desktop'] = array(
      '#title' => t('Desktop') . ' ',
      '#type' => 'number',
      '#min' => 1,
      '#max' => 99,
      '#size' => 2,
      '#description' => t("Number of columns for large screens (above 1200px or 75em)."),
      '#default_value' => $this->options['columns']['desktop'],
    );

    $form['columns']['laptop'] = array(
      '#title' => t('Laptop') . ' ',
      '#type' => 'number',
      '#min' => 1,
      '#max' => 99,
      '#size' => 2,
      '#description' => t("Number of columns for medium sized screens (above 992px or 62em)."),
      '#default_value' => $this->options['columns']['laptop'],
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[columns][auto]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['columns']['tablet'] = array(
      '#title' => t('Tablet') . ' ',
      '#type' => 'number',
      '#min' => 1,
      '#max' => 99,
      '#size' => 2,
      '#description' => t("Number of columns for tablets and smartphones (above 768px or 48em)."),
      '#default_value' => $this->options['columns']['tablet'],
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[columns][auto]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['columns']['smartphone'] = array(
      '#title' => t('Smartphone') . ' ',
      '#type' => 'number',
      '#min' => 1,
      '#max' => 99,
      '#size' => 2,
      '#description' => t("Number of columns for smartphones (above 576px or 36em)."),
      '#default_value' => $this->options['columns']['smartphone'],
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[columns][auto]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['columns']['phone'] = array(
      '#title' => t('Phone'),
      '#type' => 'number',
      '#min' => 1,
      '#max' => 99,
      '#size' => 2,
      '#description' => t("Number of columns for smallest screens."),
      '#default_value' => $this->options['columns']['phone'],
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[columns][auto]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['columns']['truncate'] = array(
      '#type' => 'checkbox',
      '#title' => t('Truncate'),
      '#description' => t("Adjust number of columns so that it does not exceed number of images."),
      '#default_value' => $this->options['columns']['truncate'],
    );

    // Caption options fieldset.
    $form['caption'] = array(
      '#type' => 'fieldset',
      '#title' => t('Caption settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    // Set options list for caption selector.
    $caption_options = array(
      'alt_tag'        => t('Image alt tag'),
      'title_tag'      => t('Image title tag'),
    );
    // Detect if valid text fields are present, and add them to the options list.
    $text_fields = $this->get_non_excluded_text_fields();
    if (!empty($text_fields)) {
      foreach ($text_fields as $field_name => $text_field) {
        $caption_options[$field_name] = $text_field->ui_name();
      }
    }
    // Create caption selector form.
    $form['caption']['source'] = array(
      '#type' => 'select',
      '#title' => t('Text source'),
      '#options' => $caption_options,
      '#empty_option' => t('None'),
      '#default_value' => $this->options['caption']['source'],
      '#description' => t('A text or title field can be added to the view, to be used as caption.'),
    );

    $caption_display = array(
      'static' => t('Always visible, below the image'),
      'hover'  => t('Hidden initially, shown on mouse hover'),
    );

    $form['caption']['display'] = array(
      '#type' => 'select',
      '#title' => t('Caption display mode'),
      '#default_value' => $this->options['caption']['display'],
      '#options' => $caption_display,
      '#states' => array(
        'disabled' => array(
          ':input[name="style_options[caption][source]"]' => array('value' => ''),
        ),
      ),
    );

    $form['caption']['height'] = array(
      '#type' => 'number',
      '#title' => t('Caption height'),
      '#min' => 1,
      '#max' => 999,
      '#step' => 1,
      '#size' => 3,
      '#field_suffix' => t('px'),
      '#default_value' => $this->options['caption']['height'],
      '#states' => array(
        'disabled' => array(
          ':input[name="style_options[caption][source]"]' => array('value' => ''),
        ),
      ),
    );

    $form['caption']['no_wrap'] = array(
      '#type' => 'checkbox',
      '#title' => t('No text wrap'),
      '#default_value' => $this->options['caption']['no_wrap'],
      '#description' => t('Truncate caption text when it does not fit column width. Prevents text wrapping.'),
      '#states' => array(
        'disabled' => array(
          ':input[name="style_options[caption][source]"]' => array('value' => ''),
        ),
      ),
    );

    $form['caption']['no_padding_h'] = array(
      '#type' => 'checkbox',
      '#title' => t('No horisontal padding'),
      '#default_value' => $this->options['caption']['no_padding_h'],
      '#description' => t('Horizontally align the caption with the gallery image. This overrides the padding setting in the <a href="@link" target="_blank">configuration page</a> (currently set to <em>@global_padding_setting em</em>).', array('@link' => url('admin/config/media/masonry_gallery'), '@global_padding_setting' => config_get('masonry_gallery.settings', 'caption_padding'))),
      '#states' => array(
        'disabled' => array(
          ':input[name="style_options[caption][source]"]' => array('value' => ''),
        ),
      ),
    );

    // Advanced options fieldset.
    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['advanced']['threshold'] = array(
      '#title' => t('Sorting threshold'),
      '#description' => t('Set balance between left to right ordering of items and height sorting. Zero value sets strict sorting of items by their height. Higher values help to preserve left to right ordering but may result in more jaggy layout at the bottom.'),
      '#type' => 'number',
      '#min' => 0,
      '#max' => 99,
      '#step' => 1,
      '#size' => 2,
      '#field_suffix' => t('pix'),
      '#default_value' => $this->options['advanced']['threshold'],
    );
  }

  /**
   * Returns first non-excluded image field.
   * Returned value is an array containing handler and field's name.
   */
  function get_first_non_excluded_image_field() {
    $fields = $this->view->display_handler->get_handlers('field');

    foreach ($fields as $field_name => $field) {
      // Ignore excluded fields.
      if (!empty($field->options['exclude']))
        continue;

      if (isset($field->field_info)) {
        $type = empty($field->field_info['type']) ? NULL : $field->field_info['type'];
        // If it is of image type, return field handler and field name.
        if ($type == 'image') {
          return array('handler' => $field, 'field_name' => $field_name);
        }
      }
    }
    return NULL;
  }

  /**
   * Returns all valid non-excluded text fields.
   * Returned value is an array containing (field's name => handler) pairs.
   */
  function get_non_excluded_text_fields() {
    $fields = $this->view->display_handler->get_handlers('field');

    $result = array();
    foreach ($fields as $field_name => $field) {
      // Ignore excluded fields.
      if (!empty($field->options['exclude']))
        continue;

      $field_valid = FALSE;
      if (isset($field->field_info)) {
        $type = empty($field->field_info['type']) ? NULL : $field->field_info['type'];
        // Consider valid if it is of any text type.
        if (in_array($type, $this->get_text_field_types())) {
          $field_valid = TRUE;
        }
      }
      // Consider valid if names start with 'title' and 'nothing'.
      // These correspond to Content:title and Global: Custom text.
      elseif (substr($field_name, 0, 5) == 'title' || substr($field_name, 0, 7) == 'nothing') {
        $field_valid = TRUE;
      }

      if ($field_valid) {
        $result[$field_name] = $field;
      }
    }
    return count($result) ? $result : NULL;
  }

  /**
   * Returns array of text field types.
   */
  function get_text_field_types() {
    return array('title', 'text', 'text_long', 'text_with_summary');
  }

  /**
   * Validates the view configuration.
   * Fails if there is no image field, or there is no text field in
   * case it is set as caption source in the plugin settings.
   */
  function validate() {
    $errors = parent::validate();

    // Do not validate when view is being created as there is no image field
    // by default, and validation will fail.
    if (!$this->view->editing) {
      return $errors;
    }

    // Check if there is an image field in the fields list.
    $image_field = $this->get_first_non_excluded_image_field();
    if (empty($image_field)) {
      $errors[] = t('Image field is not found or excluded from display. @title format requires one image field.', array('@title' => $this->plugin_title()));
    }

    // Check if there is a text field in the fields list in case text field is set as caption source.
    $caption_source = $this->options['caption']['source'];
    if (!empty($caption_source) && $caption_source != 'alt' && $caption_source != 'title') {
      $text_fields = $this->get_non_excluded_text_fields();
      if (empty($text_fields)) {
        $errors[] = t('A text field is set as caption source in @title format settings but is either not present or hidden in the fields list.', array('@title' => $this->plugin_title()));
      }
    }

    return $errors;
  }
}

<?php

/**
 *  BlockHero extends BlockText
 *
 *  This class allows us to create hero blocks with image backgrounds.
 */
class BlockVideoHero extends BlockText {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    $this->settings += array(
      'video' => '',
      'options' => array(),
    );
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Hero block');
  }

  /**
   * Returns the rendered content of this block.
   *
   * If there is no content, a non-breaking space is returned to ensure the
   * block is still displayed, with only the background image applied.
   *
   * @return string
   */
  function getContent() {
    $path = backdrop_get_path('module', 'video_hero');
    backdrop_add_css($path . '/css/video_hero.css');
    $output = '';
    if (!empty($this->settings['content'])) {
      return check_markup(token_replace($this->settings['content']), $this->settings['format'], '', TRUE);
    }
    elseif (!empty($this->settings['video'])) {

      // Set query options.
      $options = backdrop_map_assoc(array('controls', 'autoplay', 'loop'));
      foreach ($options as $value) {
        if (in_array($value, $this->settings['options'])) {
          $options[$value] = 1;
        }
        else {
          $options[$value] = 0;
        }
      }

      // Set playlist query value
      // needed if looping, unused if not looping.
      $vid = preg_replace('~(?#!js YouTubeId Rev:20160125_1800)
        # Match non-linked youtube URL in the wild. (Rev:20130823)
        https?://          # Required scheme. Either http or https.
        (?:[0-9A-Z-]+\.)?  # Optional subdomain.
        (?:                # Group host alternatives.
          youtu\.be/       # Either youtu.be,
        | youtube          # or youtube.com or
          (?:-nocookie)?   # youtube-nocookie.com
          \.com            # followed by
          \S*?             # Allow anything up to VIDEO_ID,
          [^\w\s-]         # but char before ID is non-ID char.
        )                  # End host alternatives.
        ([\w-]{11})        # $1: VIDEO_ID is exactly 11 chars.
        (?=[^\w-]|$)       # Assert next char is non-ID or EOS.
        (?!                # Assert URL is not pre-linked.
          [?=&+%\w.-]*     # Allow URL (query) remainder.
          (?:              # Group pre-linked alternatives.
            [\'"][^<>]*>   # Either inside a start tag,
          | </a>           # or inside <a> element text contents.
          )                # End recognized pre-linked alts.
        )                  # End negative lookahead assertion.
        [?=&+%\w.-]*       # Consume any URL (query) remainder.
        ~ix', '$1',
        $this->settings['video']);
      $options['playlist'] = $vid;

      // Needed to autoplay on reloading page.
      $options['mute'] = 1;

      $url = url(trim($this->settings['video']), array('query' => $options));

      $allow = array(
        'accelerometer',
        'autoplay',
        'clipboard-write',
        'encrypted-media',
        'gyroscope',
      );
      $attributes = array(
        'width' => '560',
        'height' => '315',
        'src' => $url,
        'title' => t('YouTube video player'),
        'frameborder' => '0',
        'allow' => implode('; ', $allow),
        'allowfullscreen' => TRUE,
      );
      $output .= '<iframe ' . backdrop_attributes($attributes) . '></iframe>';
    }
    elseif (!empty($this->settings['title'])) {
      $output .= '&nbsp;';
    }

    return $output;
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    unset($form['title_display']);
    unset($form['content']);
    // Add a video field at the top of the settings form.
    $form['video'] = array(
      '#type' => 'textfield',
      '#title' => t('Background video'),
      '#description' => 'YouTube video URL in the format https://www.youtube.com/embed/j16T_QlAPRc',
      '#default_value' => $this->settings['video'],
      '#weight' => -10,
    );
    $options = backdrop_map_assoc(array('controls', 'autoplay', 'loop'));
    // Add an image field at the top of the settings form.
    $form['options'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Player options'),
      '#options' => $options,
      // '#default_value' => $this->settings['options']?: array_keys($options),
      '#default_value' => $this->settings['options'],
      '#weight' => -10,
    );

    // Disable reusable until a later date.
    $form['convert']['#access'] = FALSE;
  }

  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    // Check that one of title, text, or image, is provided.
    if (empty($form_state['values']['video']) && empty($form_state['values']['title']) && empty($form_state['values']['content']['value'])) {
      form_error($form['image'], t('Background video, a Display title, or Block content must be provided.'));
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    // Save the things not handled by BlockText.
    if (!empty($form_state['values']['video'])) {
      if ($form_state['values']['video'] != $form['video']['#default_value']) {
        $this->settings['video'] = $form_state['values']['video'];
      }
    }
    $checked_options = array_filter($form_state['values']['options']);

    if (!empty($checked_options)) {
      $this->settings['options'] = $checked_options;
    }
    else {
      $this->settings['options'] = array();
    }
  }
}

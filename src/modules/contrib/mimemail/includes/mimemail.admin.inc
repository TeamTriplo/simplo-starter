<?php

/**
 * @file
 * Configuration settings page for sending MIME-encoded emails.
 */

/**
 * Configuration form.
 */
function mimemail_admin_settings() {
  // Check for the existence of a mail.css file in the default theme folder.
  $theme = config_get('system.core', 'theme_default');
  $mailstyle = backdrop_get_path('theme', $theme) . '/mail.css';
  // Disable site style sheets including option if found.
  if (is_file($mailstyle)) {
    config_get('mimemail.settings', 'mimemail_sitestyle');
    $disable_sitestyle = TRUE;
  }
  else {
    $disable_sitestyle = FALSE;
  }

  $form = array();
  $form['#config'] = 'mimemail.settings';
  $form['mimemail']['mimemail_name'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Sender name'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_name'),
    '#size'          => 60,
    '#maxlength'     => 128,
    '#description'   => t('The name that all site emails will be from when using default engine.'),
  );
  $form['mimemail']['mimemail_mail'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Sender email address'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_mail'),
    '#size'          => 60,
    '#maxlength'     => 128,
    '#description'   => t('The email address that all site emails will be from when using default engine.'),
  );
  $form['mimemail']['mimemail_simple_address'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Use simple address format'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_simple_address'),
    '#description' => t('Use the simple format of user@example.com for all recipient email addresses.'),
  );
  $form['mimemail']['mimemail_sitestyle'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Include site style sheets'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_sitestyle'),
    '#description'   => t('Gather all style sheets when no mail.css found in the theme directory. If the module Mail System is enabled, Mime Mail will look for css files in the folder of the theme selected in the Mail System settings. Otherwise the default theme folder will be used.'),
    '#disabled'      => $disable_sitestyle,
  );
  $form['mimemail']['mimemail_textonly'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send plain text email only'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_textonly'),
    '#description' => t('This option disables the use of email messages with graphics and styles. All messages will be converted to plain text.'),
  );
  $form['mimemail']['mimemail_linkonly'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Link images only'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_linkonly'),
    '#description'   => t('This option disables the embedding of images. All image will be available as external content. This can make email messages much smaller.'),
  );
  if (module_exists('mimemail_compress')) {
    $form['mimemail']['mimemail_preserve_class'] = array(
      '#type' => 'checkbox',
      '#title' => t('Preserve class attributes'),
      '#default_value' => config_get('mimemail.settings', 'mimemail_preserve_class'),
      '#description' => t('This option disables the removing of class attributes from the message source. Useful for debugging the style of the message.'),
    );
  }

  // Get a list of all formats.
  $formats = filter_formats();
  foreach ($formats as $format) {
    $format_options[$format->format] = $format->name;
  }
  $form['mimemail']['mimemail_format'] = array(
    '#type' => 'select',
    '#title' => t('Email format'),
    '#options' => $format_options,
    '#default_value' => config_get('mimemail.settings', 'mimemail_format'),
    '#access' => count($formats) > 1,
    '#attributes' => array('class' => array('filter-list')),
    '#description' => t('The filter set that will be applied to the message body.
      If you are using Mime Mail as default mail sytem, make sure to enable
      "Convert line breaks into HTML" and "Convert URLs into links" with a long
      enough maximum length for e.g. password reset URLs!'),
  );

  $form['mimemail']['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['mimemail']['advanced']['mimemail_incoming'] = array(
    '#type' => 'checkbox',
    '#title' => t('Process incoming messages posted to this site'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_incoming'),
    '#description' => t('This is an advanced setting that should not be enabled unless you know what you are doing.'),
  );
  $form['mimemail']['advanced']['mimemail_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Message validation string'),
    '#default_value' => config_get('mimemail.settings', 'mimemail_key'),
    '#required' => FALSE,
    '#description' => t('This string will be used to validate incoming messages. It can be anything, but must be used on both sides of the transfer.'),
  );

  // Get the available mail engines.
  $engines = mimemail_get_engines();
  foreach ($engines as $module => $engine) {
    $engine_options[$module] = $engine['name'] . ': ' . $engine['description'];
  }
  // Hide the settings if only 1 engine is available.
  $mimemail_engine = config_get('mimemail.settings', 'mimemail_engine');
  if (count($engines) == 1) {
    reset($engines);
    $form['mimemail']['mimemail_engine'] = array(
      '#type' => 'hidden',
      '#value' => $mimemail_engine,
    );
  }
  else {
    $form['mimemail']['mimemail_engine'] = array(
      '#type' => 'select',
      '#title' => t('Email engine'),
      '#default_value' => $mimemail_engine,
      '#options' => $engine_options,
      '#description' => t('Choose an engine for sending mails from your site.'),
    );
  }

  if ($mimemail_engine) {
    $settings = module_invoke($mimemail_engine, 'mailengine', 'settings');
    if ($settings) {
      $form['mimemail']['engine_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Engine specific settings'),
      );
      foreach ($settings as $name => $value) {
        $form['mimemail']['engine_settings'][$name] = $value;
      }
    }
  }
  else {
    backdrop_set_message(t('Please choose a mail engine.'), 'error');
  }

  return system_settings_form($form);
}

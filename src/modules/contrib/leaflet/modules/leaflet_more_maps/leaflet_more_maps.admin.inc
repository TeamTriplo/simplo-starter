<?php
/**
 * @file
 * Menu callback for Leaflet More Maps admin configuration.
 */

/**
 * Configure leaflet more maps.
 */
function leaflet_more_maps_admin_configure() {
  $config = config_get('leaflet_more_maps.settings');

  // First a global setting to enter the Google sat map version number.
  $form['global_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Global settings'),
  );
  $form['global_settings']['google_satellite_version'] = array(
    '#type' => 'textfield',
    '#title' => t('Google satellite map version'),
    '#default_value' => isset($config['google_satellite_version']) ? $config['google_satellite_version'] : '899',
    '#description' => t('If you find that tiles have gone missing, it is time to increase this number.'),
  );
  $form['global_settings']['thunderforest_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Thunderforest API Key'),
    '#default_value' => isset($config['thunderforest_api_key']) ? $config['thunderforest_api_key'] : '',
    '#description' => t('API keys are required for all Thunderforest maps.'),
  );
  $form['global_settings']['mapbox_access_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Mapbox access token'),
    '#default_value' => isset($config['mapbox_access_token']) ? $config['mapbox_access_token'] : '',
    '#description' => t('All Mapbox maps require an access token.'),
  );

  $map_info = array();
  foreach (module_implements('leaflet_map_info') as $module) {
    if ($module !== 'leaflet_more_maps') {
      $function = $module . '_leaflet_map_info';
      if (function_exists($function)) {
        $result = call_user_func_array($function, array());
        $map_info = array_merge_recursive($map_info, $result);
      }
    }
  }
  _leaflet_more_maps_assemble_default_map_info($map_info);

  $all_layer_keys = array();
  foreach ($map_info as $map_key => $map) {
    foreach ($map['layers'] as $layer_key => $layer) {
      // Unique.
      $all_layer_keys["$map_key $layer_key"] = "$map_key $layer_key";
    }
  }
  $custom_map_layers = !empty($config['leaflet_more_maps_custom_maps']) ? $config['leaflet_more_maps_custom_maps'] : array();

  if (empty($custom_map_layers)) {
    for ($i = 1; $i <= LEAFLET_MORE_MAPS_NO_CUSTOM_MAPS; $i++) {
      $custom_map_layers[$i] = array(
        'map-key' => '',
        'layer-keys' => array(),
        'reverse-order' => FALSE,
      );
    }
  }
  for ($i = 1; $i <= LEAFLET_MORE_MAPS_NO_CUSTOM_MAPS; $i++) {
    $form['map'][$i] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => $i > 1,
      '#title' => t('Custom map #@number layer selection', array('@number' => $i)),
    );
    $form['map'][$i]['map-key'] = array(
      '#type' => 'textfield',
      '#title' => t('Name of custom map #@number in the administrative UI', array('@number' => $i)),
      '#default_value' => $custom_map_layers[$i]['map-key'],
      '#description' => t('Use a blank field to remove this layer configuration from the set of selectable maps.'),
    );
    $form['map'][$i]['layer-keys'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Select one or more layers to be included in this map.'),
      '#options' => $all_layer_keys,
      '#default_value' => $custom_map_layers[$i]['layer-keys'],
      '#description' => t('If you select two or more layers, these will be selectable via radio buttons in the layer switcher on your map.'),
    );
    $form['map'][$i]['reverse-order'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reverse order in layer switcher'),
      '#default_value' => $custom_map_layers[$i]['reverse-order'],
      '#description' => t('The first layer in the switcher will be the default.'),
    );
    // Organise the $form_state['values'] structure available after submission.
    $form['map'][$i]['map-key']['#parents'] = array(
      'map', $i, 'map-key');
    $form['map'][$i]['layer-keys']['#parents'] = array(
      'map', $i, 'layer-keys');
    $form['map'][$i]['reverse-order']['#parents'] = array(
      'map', $i, 'reverse-order');
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save map configurations'),
  );
  $form['#submit'][] = 'leaflet_more_maps_admin_configure_form_submit';

  $form['#theme'] = 'system_settings_form';
  return $form;
}

/**
 * Execute leaflet_admin_configure_form.
 */
function leaflet_more_maps_admin_configure_form_submit($form, &$form_state) {
  $config = config('leaflet_more_maps.settings');

  // form_state_values_clean($form_state);
  $config->set('google_satellite_version', $form_state['values']['google_satellite_version']);
  $config->set('thunderforest_api_key', $form_state['values']['thunderforest_api_key']);
  $config->set('mapbox_access_token', $form_state['values']['mapbox_access_token']);
  $custom_maps = $form_state['values']['map'];

  // Clear out the unticked boxes.
  foreach ($custom_maps as &$custom_map) {
    $custom_map['layer-keys'] = array_filter($custom_map['layer-keys']);
  }
  $config->set('leaflet_more_maps_custom_maps', $custom_maps);
  $config->save();

  backdrop_set_message(t('The map configurations have been saved.'));

  // Refresh the cache table or hook_leaflet_more_maps() won't be called.
  // @todo No such hook!
  // @todo This is deprecated.
  cache_clear_all('*', 'cache', TRUE);
}

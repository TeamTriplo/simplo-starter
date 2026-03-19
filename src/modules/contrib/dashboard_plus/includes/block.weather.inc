<?php
/**
 * @file
 * Dashboard block showing current weather conditions via Open-Meteo API.
 */
class DashboardWeatherBlock extends Block {

  /**
   * WMO weather interpretation codes mapped to description and Phosphor icon.
   *
   * @see https://open-meteo.com/en/docs
   */
  protected static $wmo_codes = array(
    0  => array('description' => 'Clear sky', 'icon' => 'sun'),
    1  => array('description' => 'Mainly clear', 'icon' => 'cloud-sun'),
    2  => array('description' => 'Partly cloudy', 'icon' => 'cloud-sun'),
    3  => array('description' => 'Overcast', 'icon' => 'cloud'),
    45 => array('description' => 'Fog', 'icon' => 'cloud-fog'),
    48 => array('description' => 'Icy fog', 'icon' => 'cloud-fog'),
    51 => array('description' => 'Light drizzle', 'icon' => 'cloud-rain'),
    53 => array('description' => 'Moderate drizzle', 'icon' => 'cloud-rain'),
    55 => array('description' => 'Dense drizzle', 'icon' => 'cloud-rain'),
    56 => array('description' => 'Light freezing drizzle', 'icon' => 'cloud-rain'),
    57 => array('description' => 'Heavy freezing drizzle', 'icon' => 'cloud-rain'),
    61 => array('description' => 'Slight rain', 'icon' => 'cloud-rain'),
    63 => array('description' => 'Moderate rain', 'icon' => 'cloud-rain'),
    65 => array('description' => 'Heavy rain', 'icon' => 'cloud-rain'),
    66 => array('description' => 'Light freezing rain', 'icon' => 'cloud-rain'),
    67 => array('description' => 'Heavy freezing rain', 'icon' => 'cloud-rain'),
    71 => array('description' => 'Slight snow', 'icon' => 'cloud-snow'),
    73 => array('description' => 'Moderate snow', 'icon' => 'cloud-snow'),
    75 => array('description' => 'Heavy snow', 'icon' => 'cloud-snow'),
    77 => array('description' => 'Snow grains', 'icon' => 'snowflake'),
    80 => array('description' => 'Slight showers', 'icon' => 'cloud-rain'),
    81 => array('description' => 'Moderate showers', 'icon' => 'cloud-rain'),
    82 => array('description' => 'Heavy showers', 'icon' => 'cloud-rain'),
    85 => array('description' => 'Slight snow showers', 'icon' => 'cloud-snow'),
    86 => array('description' => 'Heavy snow showers', 'icon' => 'cloud-snow'),
    95 => array('description' => 'Thunderstorm', 'icon' => 'cloud-lightning'),
    96 => array('description' => 'Thunderstorm with hail', 'icon' => 'cloud-lightning'),
    99 => array('description' => 'Thunderstorm with heavy hail', 'icon' => 'cloud-lightning'),
  );

  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'location' => '',
      'units' => 'celsius',
      'show_feels_like' => TRUE,
      'show_humidity' => TRUE,
      'show_wind' => TRUE,
      'more_info_url' => '',
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Current Weather');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Current weather');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Shows current weather conditions for a configured location.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('access administration pages');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!$this->checkAccess()) {
      return array();
    }

    $location = trim($this->settings['location']);
    if (empty($location)) {
      return array(
        '#markup' => '<p>' . t('Please configure a location in the block settings.') . '</p>',
      );
    }

    $units = $this->settings['units'];

    $geo = $this->fetchGeocode($location);
    if (!$geo) {
      return array(
        '#markup' => '<p>' . t('Unable to find the location %location. Please check the block settings.', array('%location' => $location)) . '</p>',
      );
    }

    $weather = $this->fetchWeather($geo['latitude'], $geo['longitude'], $units);
    if (!$weather) {
      return array(
        '#markup' => '<p>' . t('Unable to fetch weather data at this time. Please try again later.') . '</p>',
      );
    }

    $current = $weather['current'];
    $current_units = $weather['current_units'];
    $wmo_info = $this->getWeatherInfo((int) $current['weather_code']);

    // Build the location display string.
    $location_display = check_plain($geo['name']);
    if (!empty($geo['admin1'])) {
      $location_display .= ', ' . check_plain($geo['admin1']);
    }
    $location_display .= ', ' . check_plain($geo['country']);

    $temp_unit = ($units === 'fahrenheit') ? '&deg;F' : '&deg;C';
    $wind_unit = check_plain($current_units['wind_speed_10m']);

    $weather_icon = icon($wmo_info['icon'], array(
      'alt' => $wmo_info['description'],
      'attributes' => array('style' => 'width:56px;height:56px;'),
    ));

    $output = '<div class="dashboard-weather-block" style="text-align:center;">';

    $output .= '<div class="weather-location" style="font-weight:bold;margin-bottom:8px;">'
      . $location_display . '</div>';

    $output .= '<div class="weather-main" style="display:flex;align-items:center;justify-content:center;gap:12px;margin-bottom:4px;">';
    $output .= '<div class="weather-icon">' . $weather_icon . '</div>';
    $output .= '<div class="weather-temp-block" style="text-align:left;">';
    $output .= '<div class="weather-temp" style="font-size:2.4em;font-weight:bold;line-height:1.1;">';
    $output .= round($current['temperature_2m']) . $temp_unit;
    $output .= '</div>';
    if ($this->settings['show_feels_like'] && isset($current['apparent_temperature'])) {
      $output .= '<div class="weather-feels-like" style="font-size:0.85em;color:#666;">';
      $output .= t('Feels like !temp', array('!temp' => round($current['apparent_temperature']) . $temp_unit));
      $output .= '</div>';
    }
    $output .= '</div>';
    $output .= '</div>';

    $output .= '<div class="weather-condition" style="font-size:1em;margin-bottom:8px;">'
      . check_plain(t($wmo_info['description'])) . '</div>';

    $details = array();
    if ($this->settings['show_humidity'] && isset($current['relative_humidity_2m'])) {
      $humidity_icon = icon('drop', array('attributes' => array('style' => 'width:14px;height:14px;vertical-align:middle;')));
      $details[] = $humidity_icon . ' ' . (int) $current['relative_humidity_2m'] . '%';
    }
    if ($this->settings['show_wind'] && isset($current['wind_speed_10m'])) {
      $wind_icon = icon('wind', array('attributes' => array('style' => 'width:14px;height:14px;vertical-align:middle;')));
      $details[] = $wind_icon . ' ' . round($current['wind_speed_10m']) . ' ' . $wind_unit;
    }

    if (!empty($details)) {
      $output .= '<div class="weather-details" style="display:flex;justify-content:center;gap:16px;font-size:0.9em;color:#555;margin-bottom:8px;">';
      $output .= implode('', $details);
      $output .= '</div>';
    }

    if (!empty($this->settings['more_info_url'])) {
      $output .= '<div class="weather-more-info" style="margin-top:8px;">';
      $output .= l(t('More weather information'), $this->settings['more_info_url'], array(
        'attributes' => array('target' => '_blank'),
      ));
      $output .= '</div>';
    }

    $updated_ago = REQUEST_TIME - $weather['fetched_at'];
    $output .= '<div class="weather-updated" style="font-size:0.78em;color:#999;margin-top:4px;">';
    $output .= t('Updated !time ago', array('!time' => format_interval($updated_ago)));
    $output .= '</div>';

    $output .= '</div>';

    return array('#markup' => $output);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['location'] = array(
      '#type' => 'textfield',
      '#title' => t('Location'),
      '#default_value' => $this->settings['location'],
      '#description' => t('Enter a city name (e.g. "London", "New York", "Sydney"). Weather data is provided by <a href="https://open-meteo.com" target="_blank">Open-Meteo</a> (free, no API key required).'),
      '#required' => TRUE,
    );

    $form['units'] = array(
      '#type' => 'radios',
      '#title' => t('Temperature units'),
      '#options' => array(
        'celsius'    => t('Celsius (&deg;C)'),
        'fahrenheit' => t('Fahrenheit (&deg;F)'),
      ),
      '#default_value' => $this->settings['units'],
    );

    $form['show_feels_like'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show "feels like" temperature'),
      '#default_value' => $this->settings['show_feels_like'],
    );

    $form['show_humidity'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show humidity'),
      '#default_value' => $this->settings['show_humidity'],
    );

    $form['show_wind'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show wind speed'),
      '#default_value' => $this->settings['show_wind'],
    );

    $form['more_info_url'] = array(
      '#type' => 'textfield',
      '#title' => t('More information URL'),
      '#default_value' => $this->settings['more_info_url'],
      '#description' => t('Optional. Enter a URL to link to a full weather forecast (e.g. your national meteorological service). Leave blank to show no link.'),
      '#maxlength' => 2048,
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $new_location = trim($form_state['values']['location']);

    // Clear geocode cache if the location changed.
    if ($new_location !== trim($this->settings['location'])) {
      cache()->delete('dashboard_plus:weather_geo:' . md5(strtolower($new_location)));
    }

    $this->settings['location'] = $new_location;
    $this->settings['units'] = $form_state['values']['units'];
    $this->settings['show_feels_like'] = !empty($form_state['values']['show_feels_like']);
    $this->settings['show_humidity'] = !empty($form_state['values']['show_humidity']);
    $this->settings['show_wind'] = !empty($form_state['values']['show_wind']);
    $this->settings['more_info_url'] = check_url($form_state['values']['more_info_url']);
  }

  /**
   * Fetch geocoding data for a location name via Open-Meteo geocoding API.
   *
   * Results are cached for 24 hours.
   *
   * @param string $location
   *   City name to geocode.
   *
   * @return array|NULL
   *   Associative array with keys: latitude, longitude, name, admin1, country.
   *   Returns NULL on failure or if the location cannot be found.
   */
  protected function fetchGeocode($location) {
    $cid = 'dashboard_plus:weather_geo:' . md5(strtolower(trim($location)));
    $cache = cache()->get($cid);
    if ($cache && REQUEST_TIME - $cache->created < 86400) {
      return $cache->data;
    }

    $url = 'https://geocoding-api.open-meteo.com/v1/search?name=' . rawurlencode($location) . '&count=1&language=en&format=json';
    $context = stream_context_create(array(
      'http' => array(
        'user_agent' => 'BackdropDashboardPlus/1.0',
        'timeout' => 5,
      ),
    ));
    $json = @file_get_contents($url, FALSE, $context);
    if (!$json) {
      return NULL;
    }

    $data = json_decode($json, TRUE);
    if (empty($data['results'][0])) {
      return NULL;
    }

    $result = $data['results'][0];
    $geo = array(
      'latitude'  => $result['latitude'],
      'longitude' => $result['longitude'],
      'name'      => $result['name'],
      'admin1'    => isset($result['admin1']) ? $result['admin1'] : '',
      'country'   => isset($result['country']) ? $result['country'] : '',
    );
    cache()->set($cid, $geo);
    return $geo;
  }

  /**
   * Fetch current weather data from Open-Meteo API.
   *
   * Results are cached for 30 minutes.
   *
   * @param float $lat
   *   Latitude.
   * @param float $lon
   *   Longitude.
   * @param string $units
   *   Temperature unit: 'celsius' or 'fahrenheit'.
   *
   * @return array|NULL
   *   Weather data array with 'current', 'current_units', and 'fetched_at'
   *   keys. Returns NULL on failure.
   */
  protected function fetchWeather($lat, $lon, $units) {
    $cid = 'dashboard_plus:weather:' . md5($lat . ',' . $lon . ',' . $units);
    $cache = cache()->get($cid);
    if ($cache && REQUEST_TIME - $cache->created < 1800) {
      return $cache->data;
    }

    $temp_unit = ($units === 'fahrenheit') ? 'fahrenheit' : 'celsius';
    $wind_unit = ($units === 'fahrenheit') ? 'mph' : 'kmh';

    $url = 'https://api.open-meteo.com/v1/forecast'
      . '?latitude=' . rawurlencode($lat)
      . '&longitude=' . rawurlencode($lon)
      . '&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m'
      . '&temperature_unit=' . $temp_unit
      . '&wind_speed_unit=' . $wind_unit;

    $context = stream_context_create(array(
      'http' => array(
        'user_agent' => 'BackdropDashboardPlus/1.0',
        'timeout' => 5,
      ),
    ));
    $json = @file_get_contents($url, FALSE, $context);
    if (!$json) {
      return NULL;
    }

    $data = json_decode($json, TRUE);
    if (empty($data['current'])) {
      return NULL;
    }

    $data['fetched_at'] = REQUEST_TIME;
    cache()->set($cid, $data);
    return $data;
  }

  /**
   * Get weather description and Phosphor icon name for a WMO weather code.
   *
   * @param int $code
   *   WMO weather interpretation code.
   *
   * @return array
   *   Array with 'description' and 'icon' keys.
   */
  protected function getWeatherInfo($code) {
    if (isset(self::$wmo_codes[$code])) {
      return self::$wmo_codes[$code];
    }
    return array('description' => 'Unknown conditions', 'icon' => 'cloud');
  }

}

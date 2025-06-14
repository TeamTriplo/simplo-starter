<?php
/**
 * @file
 * Contains rules integration for the system module needed during evaluation.
 *
 * @addtogroup rules
 *
 * @{
 */

/**
 * Action: Show a backdrop message.
 */
function rules_action_backdrop_message($message, $status, $repeat) {
  backdrop_set_message(filter_xss_admin($message), $status, $repeat);
}

/**
 * Action: Write a watchdog db log message.
 */
function rules_action_backdrop_watchdog($type, $message, $severity, $link_text, $link_path) {
  if (!empty($link_path)) {
    // Use $link_path for the text if no specific text was supplied.
    $link = l(!empty($link_text) ? $link_text : $link_path, url($link_path));
  }
  else {
    $link = NULL;
  }
  watchdog($type, $message, array(), $severity, $link);
}

/**
 * Action: Page redirect.
 *
 * @see rules_page_build()
 * @see rules_backdrop_goto_alter()
 */
function rules_action_backdrop_goto($url, $force = FALSE, $destination = FALSE) {
  // Don't let administrators lock them out from Rules administration pages.
  if (isset($_GET['q']) && strpos($_GET['q'], 'admin/config/workflow/rules') === 0) {
    rules_log('Skipped page redirect on Rules administration page.', array(), RulesLog::WARN);
    return;
  }
  // Do not redirect during batch processing.
  if (($batch = batch_get()) && isset($batch['current_set'])) {
    rules_log('Skipped page redirect during batch processing.');
    return;
  }

  // Keep the current destination parameter if there is one set.
  if ($destination) {
    $url .= strpos($url, '?') === FALSE ? '?' : '&';
    $url .= backdrop_http_build_query(backdrop_get_destination());
  }
  // If force is enabled, remove any destination parameter.
  if ($force && isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  // We don't invoke backdrop_goto() right now, as this would end the the current
  // page execution unpredictly for modules. So we'll take over backdrop_goto()
  // calls from somewhere else via hook_backdrop_goto_alter() and make sure
  // a backdrop_goto() is invoked before the page is output with
  // rules_page_build().
  $GLOBALS['_rules_action_backdrop_goto_do'] = array($url, $force);
}

/**
 * Action: Set breadcrumb.
 */
function rules_action_breadcrumb_set(array $titles, array $paths) {
  $trail = array(l(t('Home'), ''));
  foreach ($titles as $i => $title) {
    // Skip empty titles.
    if ($title = trim($title)) {
      // Output plaintext instead of a link if there is a title without a path.
      $path = trim($paths[$i]);
      if (!empty($paths[$i]) && $paths[$i] != '<none>') {
        $trail[] = l($title, trim($paths[$i]));
      }
      else {
        $trail[] = check_plain($title);
      }
    }
  }
  backdrop_set_breadcrumb($trail);
}

/**
 * Action Implementation: Send mail.
 */
function rules_action_mail($to, $cc, $bcc, $reply_to, $subject, $message, $from, $langcode, $settings, RulesState $state, RulesPlugin $element) {
  _rules_strip_line_breaks($to, FALSE);

  _rules_strip_line_breaks($cc);
  _rules_strip_line_breaks($bcc);
  _rules_strip_line_breaks($reply_to);
  _rules_strip_line_breaks($from);

  $params = array(
    'subject' => $subject,
    'message' => $message,
    'langcode' => $langcode,
  );
  if (!empty($cc)) {
    $params['cc'] = $cc;
  }
  if (!empty($bcc)) {
    $params['bcc'] = $bcc;
  }
  if (!empty($reply_to)) {
    $params['reply-to'] = $reply_to;
  }
  if (!empty($from)) {
    $params['from'] = $from;
  }

  // Set a unique key for this mail.
  $name = isset($element->root()->name) ? $element->root()->name : 'unnamed';
  $key = 'rules_action_mail_' . $name . '_' . $element->elementId();
  $languages = language_list();
  $language = isset($languages[$langcode]) ? $languages[$langcode] : language_default();

  $message = backdrop_mail('rules', $key, $to, $language, $params);
  if ($message['result']) {
    watchdog('rules', 'Successfully sent email to %recipient', array('%recipient' => $to));
  }
}

/**
 * Action: Send mail to all users of a specific role group(s).
 */
function rules_action_mail_to_users_of_role($roles, $cc, $bcc, $reply_to, $subject, $message, $from, $settings, RulesState $state, RulesPlugin $element) {

  _rules_strip_line_breaks($cc);
  _rules_strip_line_breaks($bcc);
  _rules_strip_line_breaks($reply_to);
  _rules_strip_line_breaks($from);

  // All authenticated users, which is everybody.
  if (in_array(BACKDROP_AUTHENTICATED_ROLE, $roles)) {
    $result = db_query('SELECT mail FROM {users} WHERE uid > 0');
  }
  else {
    // Avoid sending emails to members of two or more target role groups.
    $query = db_select('users', 'u')
    ->distinct(TRUE)
    ->fields('u', array('mail'));
    $query->innerJoin('users_roles', 'r', 'u.uid = r.uid');
    $query->condition('r.role', $roles, 'IN');
    $result = $query->execute();
  }

  // Now, actually send the mails.
  $params = array(
    'subject' => $subject,
    'message' => $message,
  );
  if (!empty($cc)) {
    $params['cc'] = $cc;
  }
  if (!empty($bcc)) {
    $params['bcc'] = $bcc;
  }
  if (!empty($reply_to)) {
    $params['reply-to'] = $reply_to;
  }
  if (!empty($from)) {
    $params['from'] = $from;
  }

  // Set a unique key for this mail.
  $name = isset($element->root()->name) ? $element->root()->name : 'unnamed';
  $key = 'rules_action_mail_to_users_of_role_' . $name . '_' . $element->elementId();
  $languages = language_list();

  $message = array('result' => TRUE);
  foreach ($result as $row) {
    $message = backdrop_mail('rules', $key, $row->mail, language_default(), $params);
    // If $message['result'] is FALSE, then it's likely that email sending is
    // failing at the moment, and we should just abort sending any more. If
    // however, $message['result'] is NULL, then it's likely that a module has
    // aborted sending this particular email to this particular user, and we
    // should just keep on sending emails to the other users.
    // For more information on the result value, see backdrop_mail().
    if ($message['result'] === FALSE) {
      break;
    }
  }
  if ($message['result'] !== FALSE) {
    $role_names = array_intersect_key(user_roles(TRUE), array_flip($roles));
    watchdog('rules', 'Successfully sent email to the role(s) %roles.', array('%roles' => implode(', ', $role_names)));
  }
}

/**
 * Strip line breaks from a string.
 */
function _rules_strip_line_breaks(&$string, $null_if_empty = TRUE) {
  $string = !empty($string) ? str_replace(array("\r", "\n"), '', $string) : ($null_if_empty ? NULL : '');
}

/**
 * Implements hook_mail().
 *
 * Sets the message subject and body as configured.
 */
function rules_mail($key, &$message, $params) {

  $message['subject'] .= str_replace(array("\r", "\n"), '', $params['subject']);
  $message['body'][] = $params['message'];
  if (!empty($params['from'])) {
    $message['from'] = $params['from'];
    $message['headers']['From'] = $params['from'];
  }
  if (!empty($params['cc'])) {
    $message['headers']['Cc'] = $params['cc'];
  }
  if (!empty($params['bcc'])) {
    $message['headers']['Bcc'] = $params['bcc'];
  }
  if (!empty($params['reply-to'])) {
    $message['reply-to'] = $params['reply-to'];
    $message['headers']['Reply-to'] = $params['reply-to'];
  }
}

/**
 * Action: Block an IP address.
 */
function rules_action_block_ip($ip_address = NULL) {
  if (empty($ip_address)) {
    $ip_address = ip_address();
  }
  db_insert('blocked_ips')->fields(array('ip' => $ip_address))->execute();
  watchdog('rules', 'Banned IP address %ip', array('%ip' => $ip_address));
}

/**
 * A class implementing a rules input evaluator processing tokens.
 */
class RulesTokenEvaluator extends RulesDataInputEvaluator {

  /**
   * Overrides RulesDataInputEvaluator::prepare().
   */
  public function prepare($text, $var_info) {
    $text = is_array($text) ? implode('', $text) : $text;
    // Skip this evaluator if there are no tokens.
    $this->setting = token_scan($text) ? TRUE : NULL;
  }

  /**
   * Evaluate tokens.
   *
   * We replace the tokens on our own as we cannot use token_replace(), because
   * token usually assumes that $data['node'] is a of type node, which doesn't
   * hold in general in our case.
   * So we properly map variable names to variable data types and then run the
   * replacement ourself.
   */
  public function evaluate($text, $options, RulesState $state) {
    $var_info = $state->varInfo();
    $options += array('sanitize' => FALSE);

    $replacements = array();
    $data = array();
    // We also support replacing tokens in a list of textual values.
    $whole_text = is_array($text) ? implode('', $text) : $text;
    foreach (token_scan($whole_text) as $var_name => $tokens) {
      $var_name = str_replace('-', '_', $var_name);
      if (isset($var_info[$var_name]) && ($token_type = _rules_system_token_map_type($var_info[$var_name]['type']))) {
        // We have to key $data with the type token uses for the variable.
        $data = rules_unwrap_data(array($token_type => $state->get($var_name)), array($token_type => $var_info[$var_name]));
        $replacements += token_generate($token_type, $tokens, $data, $options);
      }
      else {
        $replacements += token_generate($var_name, $tokens, array(), $options);
      }
      // Remove tokens if no replacement value is found. As token_replace() does
      // if 'clear' is set.
      $replacements += array_fill_keys($tokens, '');
    }

    // Optionally clean the list of replacement values.
    if (!empty($options['callback']) && function_exists($options['callback'])) {
      $function = $options['callback'];
      $function($replacements, $data, $options);
    }

    // Actually apply the replacements.
    $tokens = array_keys($replacements);
    $values = array_values($replacements);
    if (is_array($text)) {
      foreach ($text as $i => $text_item) {
        $text[$i] = str_replace($tokens, $values, $text_item);
      }
      return $text;
    }
    return str_replace($tokens, $values, $text);
  }

  /**
   * Create documentation about the available replacement patterns.
   *
   * @param array $var_info
   *   Array with the available variables.
   *
   * @return array
   *   Renderable array with the replacement pattern documentation.
   */
  public static function help($var_info) {
    $render = array(
      '#type' => 'fieldset',
      '#title' => t('Replacement patterns'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Note that token replacements containing chained objects – such as [node:author:uid] – are not listed here, but are still available. The <em>data selection</em> input mode may help you find more complex replacement patterns. See <a href="@url">the online documentation</a> for more information about complex replacement patterns.',
        array('@url' => rules_external_help('chained-tokens'))),
    );

    $token_info = token_info();
    foreach ($var_info as $name => $info) {
      $token_types[$name] = _rules_system_token_map_type($info['type']);
    }

    foreach ($token_types as $name => $token_type) {
      if (isset($token_info['types'][$token_type])) {
        $render[$name] = array(
          '#theme' => 'table',
          '#header' => array(t('Token'), t('Label'), t('Description')),
          '#prefix' => '<h3>' . t('Replacement patterns for %label', array('%label' => $var_info[$name]['label'])) . '</h3>',
        );
        foreach ($token_info['tokens'][$token_type] as $token => $info) {
          $token = '[' . str_replace('_', '-', $name) . ':' . $token . ']';
          $render[$name]['#rows'][$token] = array(
            check_plain($token),
            check_plain($info['name']),
            check_plain($info['description']),
          );
        }
      }
    }
    return $render;
  }

}

/**
 * Looks for a token type mapping. Defaults to passing through the type.
 */
function _rules_system_token_map_type($type) {
  $entity_info = entity_get_info();
  if (isset($entity_info[$type]['token type'])) {
    return $entity_info[$type]['token type'];
  }
  $cache = rules_get_cache();
  if (isset($cache['data_info'][$type]['token type'])) {
    return $cache['data_info'][$type]['token type'];
  }
  return $type;
}

/**
 * @} End of "addtogroup rules"
 */

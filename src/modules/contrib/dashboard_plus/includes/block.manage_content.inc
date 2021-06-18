<?php
/**
 * @file
 * Dashboard block displaying informtion about content types.
 */
class DashboardManageContentBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    // Set defaults.
    $this->settings += array(
      'types' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Manage content');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Manage content');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Links to administer content types.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    $node_types = node_type_get_types();
    $access = user_access('access content overview');
    //    $options = array('destination' => current_path());
    $header = array(
      array('data' => t('Content type')),
      array('data' => t('Create')),
      array('data' => t('Manage')),
    );
    $rows = array();
    // Check access, then add a link to create content.
    if ($access) {
      foreach ($node_types as $machine => $node_type) {
        // If no types are specified, show all types.
        if (empty($this->settings['types']) || in_array($machine, $this->settings['types'])) {
          $create = l(t('Create', array('@type' => t($node_type->name))), 'node/add/' . $node_type->type, array('html' => TRUE));
          $manage = l(t('Manage', array('@type' => t($node_type->name))), 'admin/content/node', array('html' => TRUE, 'query' => array('status' => 'All', 'type' => $node_type->type)));
          $rows[] = array(
            'data' => array(
              check_plain(t($node_type->name)),
              $create,
              $manage,
            ),
          );
        }
      }
      // exit;
    }
    else {
      return array();
    }

    if (empty($rows)) {
      $rows[] = array(
        array(
          'data' => t('There are no content types to display.'),
          'colspan' => 4,
        ),
      );
    }

    $panel = array(
      '#theme' => 'dashboard_panel__node_types',
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );

    return $panel;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $node_types = node_type_get_types();
    $type_options = array();
    $type_defaults = array();

    foreach ($node_types as $machine => $node_type) {
      $type_options[$machine] = check_plain(t($node_type->name));
      if (empty($this->settings['types']) || in_array($machine, $this->settings['types'])) {
        $type_defaults[] = $machine;
      }
    }

    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links for the following content types'),
      '#options' => $type_options,
      '#default_value' => $type_defaults,
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    // If all types are checked, save nothing, indicating all types are allowed.
    $checked_types = array_filter($form_state['values']['types']);
    if (count($form['types']['#options']) === count($checked_types)) {
      $this->settings['types'] = array();
    }
    else {
      $this->settings['types'] = array_values($checked_types);
    }
  }
}

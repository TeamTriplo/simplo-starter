<?php
/**
 * @file
 * Dashboard block displaying information about taxonomy vocabularies.
 */
class DashboardManageTaxonomyBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'vocabularies' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Manage taxonomy');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Manage taxonomy');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Links to administer taxonomy vocabularies.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!user_access('administer taxonomy')) {
      return array();
    }

    $vocabularies = taxonomy_get_vocabularies();

    $header = array(
      array('data' => t('Vocabulary')),
      array('data' => t('Terms')),
      array('data' => t('Create')),
      array('data' => t('Manage')),
    );

    $rows = array();
    foreach ($vocabularies as $machine => $vocab) {
      if (!empty($this->settings['vocabularies']) && !in_array($machine, $this->settings['vocabularies'])) {
        continue;
      }

      $count = db_query('SELECT COUNT(*) FROM {taxonomy_term_data} WHERE vocabulary = :vocab', array(':vocab' => $machine))->fetchField();

      $create = l(t('Add term'), 'admin/structure/taxonomy/' . $machine . '/add-term');
      $manage = l(t('Manage'), 'admin/structure/taxonomy/' . $machine);

      $rows[] = array(
        'data' => array(
          check_plain($vocab->name),
          $count,
          $create,
          $manage,
        ),
      );
    }

    if (empty($rows)) {
      $rows[] = array(
        array(
          'data' => t('There are no vocabularies to display.'),
          'colspan' => 4,
        ),
      );
    }

    $panel = array(
      '#theme' => 'dashboard_panel__taxonomy',
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

    $vocabularies = taxonomy_get_vocabularies();
    $vocab_options = array();
    $vocab_defaults = array();

    foreach ($vocabularies as $machine => $vocab) {
      $vocab_options[$machine] = check_plain($vocab->name);
      if (empty($this->settings['vocabularies']) || in_array($machine, $this->settings['vocabularies'])) {
        $vocab_defaults[] = $machine;
      }
    }

    $form['vocabularies'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links for the following vocabularies'),
      '#options' => $vocab_options,
      '#default_value' => $vocab_defaults,
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $checked = array_filter($form_state['values']['vocabularies']);
    if (count($form['vocabularies']['#options']) === count($checked)) {
      $this->settings['vocabularies'] = array();
    }
    else {
      $this->settings['vocabularies'] = array_values($checked);
    }
  }
}

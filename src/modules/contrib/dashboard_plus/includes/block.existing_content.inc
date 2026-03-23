<?php
/**
 * @file
 * Content block for Dashboard Plus.
 *
 * Displays a specific node chosen by the site architect. When the block is
 * placed on the dashboard without a node selected it renders a picker form
 * so the architect never has to leave the dashboard to configure it.
 */

// =============================================================================
// Block class
// =============================================================================

class DashboardPlusExistingContentBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    if (!empty($this->settings['nid'])) {
      $node = node_load($this->settings['nid']);
      if ($node) {
        return check_plain($node->title);
      }
    }
    return !empty($this->settings['title'])
      ? check_plain($this->settings['title'])
      : t('Content block');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Content block');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Find an existing piece of content or create something new to display here, all without leaving the dashboard.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('access content');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    // No node selected yet — show the picker to layout admins.
    if (empty($this->settings['nid'])) {
      if (!user_access('administer layouts')) {
        return array();
      }
      return array(
        'picker' => backdrop_get_form(
          'dashboard_plus_content_block_pick_form',
          $this->uuid
        ),
      );
    }

    $node = node_load($this->settings['nid']);

    // Node missing or inaccessible — let admins re-pick.
    if (!$node || !node_access('view', $node)) {
      if (user_access('administer layouts')) {
        return array(
          'notice' => array(
            '#markup' => '<p class="dashboard-plus-picker-notice">'
              . t('The selected content is no longer available. Please choose a different piece of content.')
              . '</p>',
          ),
          'picker' => backdrop_get_form(
            'dashboard_plus_content_block_pick_form',
            $this->uuid
          ),
        );
      }
      return array();
    }

    $view_mode = isset($this->settings['view_mode']) ? $this->settings['view_mode'] : 'full';

    // Clone before rendering so we can suppress the title without mutating the
    // cached node object. The block label already shows the title, so we don't
    // need it repeated inside the block body.
    $display = clone $node;
    $display->title = '';
    return node_view($display, $view_mode);
  }

  /**
   * {@inheritdoc}
   *
   * Settings form shown in the layout manager's "Configure block" dialog.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $nid = isset($this->settings['nid']) ? (int) $this->settings['nid'] : '';

    $node_title = '';
    if ($nid) {
      $node = node_load($nid);
      if ($node) {
        $node_title = $node->title . ' [nid: ' . $nid . ']';
      }
    }

    $form['nid'] = array(
      '#type' => 'textfield',
      '#title' => t('Node ID'),
      '#default_value' => $nid ?: '',
      '#size' => 10,
      '#description' => t('The numeric ID of the node to display. Currently showing: %title', array(
        '%title' => $node_title ?: t('(none selected)'),
      )),
    );

    $form['view_mode'] = array(
      '#type' => 'select',
      '#title' => t('Display mode'),
      '#options' => array(
        'full'   => t('Full content'),
        'teaser' => t('Teaser'),
      ),
      '#default_value' => isset($this->settings['view_mode']) ? $this->settings['view_mode'] : 'full',
    );
  }

  /**
   * {@inheritdoc}
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    $nid = $form_state['values']['nid'];
    if ($nid === '' || $nid === NULL) {
      return;
    }
    if (!is_numeric($nid) || (int) $nid <= 0) {
      form_set_error('nid', t('Node ID must be a positive number.'));
    }
    elseif (!node_load((int) $nid)) {
      form_set_error('nid', t('No content was found with ID %nid.', array('%nid' => $nid)));
    }
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $nid = $form_state['values']['nid'];
    $this->settings['nid'] = ($nid !== '' && $nid !== NULL) ? (int) $nid : NULL;
    $this->settings['view_mode'] = $form_state['values']['view_mode'];
  }
}

// =============================================================================
// Node picker form
// =============================================================================

/**
 * Form: configure a content block directly on the dashboard.
 *
 * Offers two paths:
 *  - Find existing content (filterable by content type, AJAX-driven).
 *  - Create new content: stubs a draft node, wires it to the block, then
 *    sends the architect to the node edit form. Saving the node returns them
 *    to the dashboard where the block already shows their new content.
 *
 * @param string $block_uuid
 *   UUID of the DashboardPlusExistingContentBlock being configured.
 */
function dashboard_plus_content_block_pick_form($form, &$form_state, $block_uuid) {
  $module_path = backdrop_get_path('module', 'dashboard_plus');
  $form['#attached']['css'][] = $module_path . '/css/existing-content-picker.css';
  $form['#attached']['js'][]  = $module_path . '/js/content-picker.js';

  $form['#attributes']['class'][] = 'dashboard-plus-content-picker';

  // Identify the layout that owns this block.
  $layouts = layout_load_multiple_by_path('admin/dashboard');
  $layout  = reset($layouts);
  if (!$layout) {
    return array(
      '#markup' => '<p>' . t('Could not load the dashboard layout.') . '</p>',
    );
  }

  $form['block_uuid']  = array('#type' => 'value', '#value' => $block_uuid);
  $form['layout_name'] = array('#type' => 'value', '#value' => $layout->name);

  $form['intro'] = array(
    '#markup' => '<p class="dashboard-plus-picker-intro">'
      . t('Place an existing piece of content in this block or create a new card.')
      . '</p>',
  );

  $types = node_type_get_types();

  // Pass all published nodes to JS for client-side filtering — avoids AJAX
  // round-trips and works reliably regardless of form caching state.
  $all_rows = db_select('node', 'n')
    ->fields('n', array('nid', 'title', 'type'))
    ->condition('n.status', NODE_PUBLISHED)
    ->orderBy('n.type')
    ->orderBy('n.title')
    ->range(0, 500)
    ->execute();

  $js_nodes = array();
  foreach ($all_rows as $row) {
    $js_nodes[] = array(
      'nid'        => (int) $row->nid,
      'title'      => $row->title,
      'type'       => $row->type,
      'type_label' => isset($types[$row->type]) ? $types[$row->type]->name : $row->type,
    );
  }

  $form['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('dashboardPlusContentPicker' => array('nodes' => $js_nodes)),
  );

  // ===========================================================================
  // Row 1: Find existing content — compact 3-column grid.
  // Type filtering is done client-side by content-picker.js.
  // ===========================================================================

  $type_filter_options = array('' => t('All content types'));
  foreach ($types as $machine_name => $type_obj) {
    $type_filter_options[$machine_name] = $type_obj->name;
  }

  // The three grid cells are direct children of this container:
  // [content_type select] [node_wrapper] [actions]
  $form['find_row'] = array(
    '#type'       => 'container',
    '#attributes' => array('class' => array('dashboard-plus-picker-row')),
  );

  $form['find_row']['content_type'] = array(
    '#type'       => 'select',
    '#title'      => t('Filter by type'),
    '#options'    => $type_filter_options,
    '#attributes' => array('class' => array('dashboard-plus-type-filter')),
  );

  $form['find_row']['node_wrapper'] = array(
    '#type'       => 'container',
    '#attributes' => array('class' => array('dashboard-plus-node-wrapper')),
  );
  $form['find_row']['node_wrapper']['nid'] = array(
    '#type'       => 'select',
    '#title'      => t('Content'),
    '#options'    => _dashboard_plus_node_picker_options(''),
    '#attributes' => array('class' => array('dashboard-plus-node-select')),
  );

  $form['find_row']['find_actions'] = array('#type' => 'actions');
  $form['find_row']['find_actions']['submit'] = array(
    '#type'                    => 'submit',
    '#value'                   => t('Display this content'),
    '#limit_validation_errors' => array(
      array('layout_name'),
      array('block_uuid'),
      array('nid'),
    ),
    '#submit'                  => array('dashboard_plus_content_block_pick_form_submit'),
  );

  // ===========================================================================
  // Row 2: Create a new Card — compact single row.
  //
  // Cards are pathless nodes designed for embedded content. If the Card content
  // type does not exist, link to the Cards recipe instead.
  // ===========================================================================

  $card_type = node_type_load('card');

  if ($card_type) {
    $form['create_row'] = array(
      '#type'       => 'container',
      '#attributes' => array('class' => array('dashboard-plus-picker-create-row')),
    );
    $form['create_row']['create_actions'] = array('#type' => 'actions');
    $form['create_row']['create_actions']['create'] = array(
      '#type'                    => 'submit',
      '#value'                   => t('Create a new Card'),
      '#limit_validation_errors' => array(
        array('layout_name'),
        array('block_uuid'),
      ),
      '#submit'                  => array('dashboard_plus_content_block_create_submit'),
    );
  }
  else {
    $form['no_card'] = array(
      '#markup' => '<p class="dashboard-plus-picker-no-card">'
        . t('To create new content for this block, install the !link.', array(
          '!link' => l(t('Cards recipe'), 'admin/config/installation/recipes'),
        ))
        . '</p>',
    );
  }

  return $form;
}

/**
 * Submit handler: wire an existing node to the block and return to dashboard.
 */
function dashboard_plus_content_block_pick_form_submit($form, &$form_state) {
  $layout_name = $form_state['values']['layout_name'];
  $block_uuid  = $form_state['values']['block_uuid'];
  $nid         = (int) $form_state['values']['nid'];

  if (!$nid) {
    backdrop_set_message(t('Please select a piece of content to display.'), 'warning');
    return;
  }

  $layout = layout_load($layout_name);
  if (!$layout || !isset($layout->content[$block_uuid])) {
    backdrop_set_message(t('Could not locate the block to configure.'), 'error');
    return;
  }

  $layout->content[$block_uuid]->settings['nid'] = $nid;
  $layout->save();

  $node = node_load($nid);
  if ($node) {
    backdrop_set_message(t('This block will now display %title.', array(
      '%title' => $node->title,
    )));
  }

  $form_state['redirect'] = 'admin/dashboard';
}

/**
 * Submit handler: create a draft node stub, wire it to the block, then send
 * the architect to the node edit form with a return path to the dashboard.
 */
function dashboard_plus_content_block_create_submit($form, &$form_state) {
  $layout_name = $form_state['values']['layout_name'];
  $block_uuid  = $form_state['values']['block_uuid'];

  $card_type = node_type_load('card');
  if (!$card_type) {
    backdrop_set_message(t('The Card content type is not available.'), 'error');
    return;
  }

  // Cards are pathless nodes designed for embedded content — publishing them
  // does not create a public-facing page in listings or feeds.
  $node = new Node(array(
    'type'     => 'card',
    'title'    => t('New Card'),
    'uid'      => $GLOBALS['user']->uid,
    'status'   => 1,
    'langcode' => LANGUAGE_NONE,
  ));
  node_save($node);

  // Immediately wire this node to the block so the dashboard is ready when
  // the architect returns after editing.
  $layout = layout_load($layout_name);
  if (!$layout || !isset($layout->content[$block_uuid])) {
    backdrop_set_message(t('Could not locate the block to configure.'), 'error');
    // Still continue to the node edit form — they can configure the block later.
  }
  else {
    $layout->content[$block_uuid]->settings['nid'] = $node->nid;
    $layout->save();
  }

  backdrop_set_message(t('Fill in the content details below. When you save, your dashboard block will display this content automatically.'));

  // Redirect to the node edit form; saving there returns to the dashboard.
  $form_state['redirect'] = array(
    'node/' . $node->nid . '/edit',
    array('query' => array('destination' => 'admin/dashboard')),
  );
}

// =============================================================================
// Helper
// =============================================================================

/**
 * Builds select options for published nodes, optionally filtered by type.
 *
 * Results are grouped into optgroups by content type so the selector stays
 * readable when "All content types" is chosen.
 *
 * @param string $type
 *   Machine name of a content type, or empty string for all types.
 *
 * @return array
 *   Options array suitable for a #type => 'select' element.
 */
function _dashboard_plus_node_picker_options($type = '') {
  $query = db_select('node', 'n')
    ->fields('n', array('nid', 'title', 'type'))
    ->condition('n.status', NODE_PUBLISHED)
    ->orderBy('n.type')
    ->orderBy('n.title')
    ->range(0, 500);

  if ($type !== '') {
    $query->condition('n.type', $type);
  }

  $results = $query->execute();

  $options    = array('' => t('-- Select content --'));
  $types      = node_type_get_types();
  $by_type    = array();

  foreach ($results as $row) {
    $by_type[$row->type][$row->nid] = $row->title;
  }

  if ($type !== '') {
    // Single type — flat list, no optgroups needed.
    if (!empty($by_type[$type])) {
      $options += $by_type[$type];
    }
  }
  else {
    // Multiple types — one optgroup per type.
    foreach ($by_type as $type_key => $nodes) {
      $label = isset($types[$type_key]) ? $types[$type_key]->name : $type_key;
      $options[$label] = $nodes;
    }
  }

  return $options;
}

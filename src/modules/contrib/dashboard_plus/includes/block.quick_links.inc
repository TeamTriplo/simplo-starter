<?php
/**
 * @file
 * Quick Links block for Dashboard Plus.
 *
 * Lets site architects collect frequently-used links (internal paths or
 * external URLs) and display them in a responsive multi-column grid directly
 * on the dashboard. Links are added and removed without leaving the page.
 */

// =============================================================================
// Block class
// =============================================================================

class DashboardPlusQuickLinksBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title'])
      ? check_plain($this->settings['title'])
      : t('Quick links');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Quick links');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('A customizable collection of links to pages, forms, or configuration the site architect uses often. Links spread across multiple columns in wide regions.');
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
    $links    = !empty($this->settings['links']) ? $this->settings['links'] : array();
    $is_admin = user_access('administer layouts');
    $module_path = backdrop_get_path('module', 'dashboard_plus');

    if (empty($links) && !$is_admin) {
      return array();
    }

    $output = array(
      '#attached' => array(
        'css' => array($module_path . '/css/quick-links.css'),
        'js'  => array($module_path . '/js/quick-links.js'),
      ),
    );

    // ---- Link grid ----------------------------------------------------------

    // Sort alphabetically by display label (fall back to URL when no label).
    usort($links, function ($a, $b) {
      $label_a = strtolower(!empty($a['label']) ? $a['label'] : $a['url']);
      $label_b = strtolower(!empty($b['label']) ? $b['label'] : $b['url']);
      return strcmp($label_a, $label_b);
    });

    if (!empty($links)) {
      $items_html = '';
      foreach ($links as $link) {
        $label      = !empty($link['label']) ? $link['label'] : $link['url'];
        $is_ext     = url_is_external($link['url']);
        $link_attrs = array(
          'class' => array('dashboard-plus-quick-link'),
        );
        if ($is_ext) {
          $link_attrs['target'] = '_blank';
          $link_attrs['rel']    = 'noopener noreferrer';
        }

        $link_html = l(check_plain($label), $link['url'], array(
          'attributes' => $link_attrs,
          'external'   => $is_ext,
        ));

        $remove_html = '';
        if ($is_admin) {
          $token = backdrop_get_token(
            'dashboard-plus-ql-remove-' . $this->uuid . '-' . $link['id']
          );
          $remove_html = l('&times;', 'dashboard-plus/quick-link/' . $this->uuid . '/remove/' . $link['id'], array(
            'html'       => TRUE,
            'query'      => array('token' => $token),
            'attributes' => array(
              'class'      => array('dashboard-plus-ql-remove'),
              'title'      => t('Remove this link'),
              'aria-label' => t('Remove link: !label', array('!label' => $label)),
            ),
          ));
        }

        $items_html .= '<div class="dashboard-plus-ql-item">'
          . $link_html . $remove_html
          . '</div>';
      }

      $output['links'] = array(
        '#markup' => '<div class="dashboard-plus-ql-list">' . $items_html . '</div>',
      );
    }

    // ---- Add trigger + inline form (admins only) ----------------------------

    if ($is_admin) {
      $trigger_icon = icon('plus-circle', array(
        'attributes' => array('style' => 'width:15px;height:15px;vertical-align:-3px;margin-right:4px;'),
      ));

      $output['add_trigger'] = array(
        '#markup' => '<p class="dashboard-plus-ql-bar">'
          . '<a href="#" class="dashboard-plus-ql-trigger">'
          . $trigger_icon . t('Add a quick link')
          . '</a></p>',
      );

      $output['add_form_wrapper'] = array(
        '#prefix' => '<div class="dashboard-plus-ql-form-wrapper">',
        '#suffix' => '</div>',
        'form'    => backdrop_get_form('dashboard_plus_quick_link_add_form', $this->uuid),
      );
    }

    return $output;
  }
}

// =============================================================================
// Add-link form
// =============================================================================

/**
 * Form: add a quick link to this block.
 *
 * @param string $block_uuid
 *   UUID of the DashboardPlusQuickLinksBlock being edited.
 */
function dashboard_plus_quick_link_add_form($form, &$form_state, $block_uuid) {
  $layouts = layout_load_multiple_by_path('admin/dashboard');
  $layout  = reset($layouts);
  if (!$layout) {
    return array('#markup' => '<p>' . t('Could not load the dashboard layout.') . '</p>');
  }

  $form['block_uuid']  = array('#type' => 'value', '#value' => $block_uuid);
  $form['layout_name'] = array('#type' => 'value', '#value' => $layout->name);

  // Compact 3-column grid: [URL] [Label] [Add link].
  $form['row'] = array(
    '#type'       => 'container',
    '#attributes' => array('class' => array('dashboard-plus-ql-add-row')),
  );

  $form['row']['url'] = array(
    '#type'        => 'textfield',
    '#title'       => t('URL or path'),
    '#placeholder' => t('https://example.com or admin/content'),
    '#required'    => TRUE,
    '#size'        => 30,
  );

  $form['row']['label'] = array(
    '#type'        => 'textfield',
    '#title'       => t('Label'),
    '#placeholder' => t('Optional'),
    '#size'        => 20,
  );

  $form['row']['actions'] = array('#type' => 'actions');
  $form['row']['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Add link'),
  );

  return $form;
}

/**
 * Validation handler for the add-link form.
 */
function dashboard_plus_quick_link_add_form_validate($form, &$form_state) {
  $url = trim($form_state['values']['url']);
  if (empty($url)) {
    form_set_error('url', t('Please enter a URL or path.'));
  }
}

/**
 * Submit handler: append the new link to the block settings.
 */
function dashboard_plus_quick_link_add_form_submit($form, &$form_state) {
  $layout_name = $form_state['values']['layout_name'];
  $block_uuid  = $form_state['values']['block_uuid'];
  $url         = trim($form_state['values']['url']);
  $label       = trim($form_state['values']['label']);

  $layout = layout_load($layout_name);
  if (!$layout || !isset($layout->content[$block_uuid])) {
    backdrop_set_message(t('Could not find the block to update.'), 'error');
    return;
  }

  $block  = $layout->content[$block_uuid];
  $links  = isset($block->settings['links']) ? $block->settings['links'] : array();
  $links[] = array(
    'id'    => uniqid('ql', TRUE),
    'url'   => $url,
    'label' => $label,
  );
  $block->settings['links'] = $links;
  $layout->save();

  $form_state['redirect'] = 'admin/dashboard';
}

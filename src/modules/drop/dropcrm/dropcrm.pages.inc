<?php
/**
 * @file
 * Page callbacks for viewing CRM entities on the front end.
 */

/**
 * Page callback: display a single CRM contact.
 *
 * @param CrmContact $contact
 *   The contact entity to display.
 *
 * @return array
 *   A render array.
 */
function dropcrm_contact_view(CrmContact $contact) {
  backdrop_set_title(entity_label('crm_contact', $contact), PASS_THROUGH);
  backdrop_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Drop Suite'), 'drop'),
    l(t('CRM'), 'drop/crm'),
    l(t('Contacts'), 'drop/crm/contacts'),
  ));

  // Always start with a plain array so all named children render reliably.
  $output = array();
  $output['#attached']['css'][] = backdrop_get_path('module', 'dropcrm') . '/dropcrm.admin.css';

  // --- Action links bar (edit + bundle-specific quick actions) ---
  $actions = array();
  if (dropcrm_contact_access('edit', $contact)) {
    $bundle_labels = array(
      'crm_person'       => t('Edit person'),
      'crm_organization' => t('Edit organization'),
      'crm_household'    => t('Edit household'),
    );
    $edit_label = isset($bundle_labels[$contact->bundle()]) ? $bundle_labels[$contact->bundle()] : t('Edit');
    $actions[] = l($edit_label, 'drop/crm/contacts/' . $contact->cid . '/edit');
  }
  if (dropcrm_contact_access('delete', $contact)) {
    $actions[] = l(t('Delete'), 'drop/crm/contacts/' . $contact->cid . '/delete');
  }
  $output['actions'] = array(
    '#markup' => drop_base_page_actions($actions),
    '#weight' => -100,
  );

  // --- Core details ---
  if ($contact->bundle() === 'crm_person') {
    $output['details'] = _dropcrm_person_details($contact);
    $output['details']['#weight'] = 0;
  }
  else {
    $output['details'] = entity_view($contact, 'full');
    $output['details']['#weight'] = 0;
  }

  // --- Affiliated contacts (organizations only) ---
  if ($contact->bundle() === 'crm_organization') {
    $affiliated = _dropcrm_load_org_contacts($contact->cid);
    $aff_rows = array();
    foreach ($affiliated as $person) {
      $aff_rows[] = array(
        l(entity_label('crm_contact', $person), 'drop/crm/contact/' . $person->cid),
      );
    }
    $output['affiliated_contacts'] = array(
      '#type'   => 'container',
      '#weight' => 5,
    );
    $output['affiliated_contacts']['heading'] = array(
      '#markup' => drop_base_section_header(
        t('Affiliated People'),
        l(t('+ Add person'), 'drop/crm/contacts/add/crm_person',
          array('attributes' => array('class' => array('drop-section-add')),
                'query'      => array('org' => $contact->cid)))
      ),
      '#weight' => -5,
    );
    $output['affiliated_contacts']['table'] = array(
      '#theme'  => 'table',
      '#header' => array(t('Name')),
      '#rows'   => $aff_rows,
      '#empty'  => t('No people affiliated with this organization yet.'),
    );
  }

  // --- Contact details (phone, email, address) ---
  $output['contact_methods'] = array(
    '#type'   => 'container',
    '#weight' => 10,
  );
  $add_links = array();
  foreach (array(
    'email'     => t('Email'),
    'telephone' => t('Phone'),
    'address'   => t('Address'),
    'url'       => t('Website/Social'),
  ) as $bundle => $label) {
    $add_links[] = l('+ ' . $label, 'drop/crm/contact-methods/add/' . $bundle,
      array('attributes' => array('class' => array('drop-section-add')),
            'query'      => array('contact' => $contact->cid)));
  }
  $output['contact_methods']['heading'] = array(
    '#markup' => drop_base_section_header(
      t('Contact Details'),
      '<span class="drop-section-add-group">' . implode(' ', $add_links) . '</span>'
    ),
    '#weight' => -5,
  );
  $methods = _dropcrm_load_contact_methods($contact->cid);
  foreach ($methods as $type => $type_methods) {
    $output['contact_methods'][$type] = array(
      '#theme'  => 'item_list',
      '#title'  => _dropcrm_contact_method_bundle_label($type),
      '#items'  => array(),
      '#weight' => 0,
    );
    foreach ($type_methods as $method) {
      $label = !empty($method->label) ? $method->label . ': ' : '';
      if ($type === 'url') {
        $value = l(check_plain($method->value), $method->value,
          array('attributes' => array('target' => '_blank', 'rel' => 'noopener')));
      }
      else {
        $value = check_plain($method->value);
      }
      $output['contact_methods'][$type]['#items'][] = $label . $value;
    }
  }

  // --- Relationships ---
  $output['relationships'] = array('#type' => 'container', '#weight' => 15);
  $output['relationships']['heading'] = array(
    '#markup' => drop_base_section_header(
      t('Relationships'),
      l(t('+ Add relationship'), 'drop/crm/relationships/add',
        array('attributes' => array('class' => array('drop-section-add')),
              'query'      => array('contact' => $contact->cid)))
    ),
    '#weight' => -5,
  );
  $relationships  = _dropcrm_load_contact_relationships($contact->cid);
  $type_options   = _dropcrm_relationship_type_options();
  $rel_rows = array();
  foreach ($relationships as $rel) {
    $other = crm_contact_load($rel->other_cid);
    if (!$other) {
      continue;
    }
    $type_label = isset($type_options[$rel->type]) ? $type_options[$rel->type] : $rel->type;
    $ops = l(t('Edit'), 'drop/crm/relationships/' . $rel->rid . '/edit')
      . ' &middot; ' . l(t('Delete'), 'drop/crm/relationships/' . $rel->rid . '/delete');
    $rel_rows[] = array(
      l(entity_label('crm_contact', $other), 'drop/crm/contact/' . $other->cid),
      $type_label,
      check_plain($rel->notes),
      $ops,
    );
  }
  $output['relationships']['table'] = array(
    '#theme'  => 'table',
    '#header' => array(t('Contact'), t('Relationship'), t('Notes'), t('Operations')),
    '#rows'   => $rel_rows,
    '#empty'  => t('No relationships recorded.'),
  );

  // --- Activities ---
  $output['activities'] = array(
    '#type'   => 'container',
    '#weight' => 20,
  );
  $output['activities']['heading'] = array(
    '#markup' => drop_base_section_header(
      t('Activities'),
      l(t('+ Add activity'), 'drop/crm/activities/add',
        array('attributes' => array('class' => array('drop-section-add')),
              'query'      => array('contact' => $contact->cid)))
    ),
    '#weight' => -5,
  );
  $activities = _dropcrm_load_contact_activities($contact->cid);
  $type_options   = _dropcrm_activity_type_options();
  $status_options = _dropcrm_activity_status_options();

  // Find which activities have meeting notes — single query for all IDs.
  $aids = array_keys($activities);
  $aids_with_notes = array();
  if ($aids) {
    $notes_result = db_select('field_data_field_crm_meeting_notes', 'n')
      ->fields('n', array('entity_id'))
      ->condition('entity_id', $aids, 'IN')
      ->condition('field_crm_meeting_notes_value', '', '<>')
      ->execute();
    foreach ($notes_result as $row) {
      $aids_with_notes[$row->entity_id] = TRUE;
    }
  }

  $rows = array();
  foreach ($activities as $activity) {
    $subject_cell = l(check_plain($activity->subject), 'drop/crm/activity/' . $activity->aid);
    if (!empty($activity->is_outreach)) {
      $subject_cell .= ' <span class="crm-outreach-badge">' . t('outreach') . '</span>';
    }
    if (!empty($aids_with_notes[$activity->aid])) {
      $subject_cell .= ' <span class="crm-notes-badge" title="' . t('Has meeting notes') . '">'
        . icon('note', array('attributes' => array('width' => 14, 'height' => 14))) . '</span>';
    }
    $rows[] = array(
      'data'  => array(
        array('data' => $subject_cell),
        isset($type_options[$activity->type]) ? $type_options[$activity->type] : $activity->type,
        isset($status_options[$activity->status]) ? $status_options[$activity->status] : $activity->status,
        format_date($activity->activity_date, 'custom', 'M j, Y'),
      ),
      'class' => $activity->status === 'planned' ? array('crm-activity-planned') : array(),
    );
  }
  $output['activities']['table'] = array(
    '#theme'  => 'table',
    '#header' => array(t('Subject'), t('Type'), t('Status'), t('Date')),
    '#rows'   => $rows,
    '#empty'  => t('No activities recorded.'),
  );

  // Allow other modules (e.g. dropbridge) to append sections.
  backdrop_alter('dropcrm_contact_view', $output, $contact);

  return $output;
}

/**
 * Page callback: display a single CRM activity.
 *
 * @param CrmActivity $activity
 *   The activity entity to display.
 *
 * @return array
 *   A render array.
 */
function dropcrm_activity_view(CrmActivity $activity) {
  backdrop_set_title(entity_label('crm_activity', $activity), PASS_THROUGH);
  backdrop_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Drop Suite'), 'drop'),
    l(t('CRM'), 'drop/crm'),
    l(t('Activities'), 'drop/crm/activities'),
  ));

  $aid            = $activity->aid;
  $type_options   = _dropcrm_activity_type_options();
  $status_options = _dropcrm_activity_status_options();
  $priority_options = _dropcrm_activity_priority_options();
  $direction_options = _dropcrm_activity_direction_options();

  $rows = array();

  if (!empty($activity->type)) {
    $rows[] = array(t('Type'), isset($type_options[$activity->type]) ? $type_options[$activity->type] : $activity->type);
  }
  if (!empty($activity->status)) {
    $rows[] = array(t('Status'), isset($status_options[$activity->status]) ? $status_options[$activity->status] : $activity->status);
  }
  if (isset($activity->priority)) {
    $p_class = 'drop-priority-' . drop_base_priority_class($activity->priority);
    $rows[] = array(
      t('Priority'),
      array(
        'data'  => isset($priority_options[$activity->priority]) ? $priority_options[$activity->priority] : '',
        'class' => array($p_class),
      ),
    );
  }
  if (!empty($activity->direction)) {
    $rows[] = array(t('Direction'), isset($direction_options[$activity->direction]) ? $direction_options[$activity->direction] : $activity->direction);
  }
  if (!empty($activity->activity_date)) {
    $rows[] = array(t('Activity Date'), format_date($activity->activity_date, 'custom', 'M j, Y'));
  }
  if (!empty($activity->is_outreach)) {
    $rows[] = array(t('Outreach'), t('Yes — counted toward outreach goal'));
  }
  if (!empty($activity->created)) {
    $rows[] = array(t('Created'), format_date($activity->created, 'custom', 'M j, Y'));
  }
  if (!empty($activity->due_date)) {
    $today     = strtotime('today midnight');
    $two_days  = strtotime('+2 days midnight');
    if ($activity->due_date < $today) {
      $due_class = 'crm-due-overdue';
    }
    elseif ($activity->due_date < $two_days) {
      $due_class = 'crm-due-soon';
    }
    else {
      $due_class = 'crm-due-future';
    }
    $rows[] = array(
      t('Date Due'),
      array(
        'data'  => format_date($activity->due_date, 'custom', 'M j, Y'),
        'class' => array($due_class),
      ),
    );
  }

  // Contacts linked to this activity.
  $lang = LANGUAGE_NONE;
  try {
    $wrapper = entity_metadata_wrapper('crm_activity', $activity);
    $contacts = $wrapper->field_crm_contacts->value();
    if ($contacts) {
      $links = array();
      foreach ((array) $contacts as $contact) {
        if ($contact) {
          $links[] = l(entity_label('crm_contact', $contact), 'drop/crm/contact/' . $contact->cid);
        }
      }
      if ($links) {
        $rows[] = array(t('Contacts'), implode(', ', $links));
      }
    }
  }
  catch (EntityMetadataWrapperException $e) {}

  $output = array();
  $output['#attached']['css'][] = backdrop_get_path('module', 'dropcrm') . '/dropcrm.admin.css';

  // --- Action links bar (top of page) ---
  $activity_actions = array();
  if (dropcrm_activity_access('edit', $activity)) {
    $activity_actions[] = l(t('Edit activity'), 'drop/crm/activities/manage/' . $aid . '/edit');
  }
  if (dropcrm_activity_access('delete', $activity)) {
    $activity_actions[] = l(t('Delete'), 'drop/crm/activities/manage/' . $aid . '/delete');
  }

  // "Add follow-up task" button — pre-fills title, links contact and activity.
  if (module_exists('droptasks') && user_access('access tasks')) {
    $task_query = array(
      'title'    => 'Follow up: ' . $activity->subject,
      'activity' => $aid,
    );
    // Link to the first contact on the activity if available.
    try {
      $wrapper  = entity_metadata_wrapper('crm_activity', $activity);
      $contacts = $wrapper->field_crm_contacts->value();
      if ($contacts) {
        $first = reset($contacts);
        if ($first) {
          $task_query['contact'] = $first->cid;
        }
      }
    }
    catch (EntityMetadataWrapperException $e) {}

    $activity_actions[] = l(t('+ Add follow-up task'), 'drop/tasks/add', array(
      'query' => $task_query,
    ));
  }

  // "Log time" button — links to DropTime daily page with note pre-filled.
  if (module_exists('droptime') && user_access('access droptime')) {
    $activity_actions[] = l(t('+ Log time'), 'drop/time', array(
      'query' => array(
        'notes'  => $activity->subject,
        'return' => 'drop/crm/activity/' . $aid,
      ),
    ));
  }
  $output['actions'] = array(
    '#markup' => drop_base_page_actions($activity_actions),
    '#weight' => -100,
  );

  // --- Details section ---
  $output['details_heading'] = array(
    '#markup' => drop_base_section_header(t('Details')),
    '#weight' => 0,
  );
  $output['details'] = array(
    '#theme'      => 'table',
    '#rows'       => $rows,
    '#attributes' => array('class' => array('crm-activity-view')),
    '#weight'     => 1,
  );

  // --- Extend due date shortcuts ---
  $extend_label = !empty($activity->due_date) ? t('Extend Due Date:') : t('Set Due Date:');
  $output['extend'] = array(
    '#prefix' => '<div class="crm-extend-links"><strong>' . $extend_label . '</strong> ',
    '#markup' => l(t('+ 1 Day'),   'drop/crm/activity/' . $aid . '/extend/day')
      . ' &nbsp;·&nbsp; '
      . l(t('+ 1 Week'),  'drop/crm/activity/' . $aid . '/extend/week')
      . ' &nbsp;·&nbsp; '
      . l(t('+ 1 Month'), 'drop/crm/activity/' . $aid . '/extend/month'),
    '#suffix' => '</div>',
    '#weight' => 2,
  );

  // --- Notes section ---
  try {
    $notes = entity_metadata_wrapper('crm_activity', $activity)->field_crm_activity_body->value();
    if (!empty($notes)) {
      $output['notes_heading'] = array(
        '#markup' => drop_base_section_header(t('Notes')),
        '#weight' => 10,
      );
      $output['notes'] = array(
        '#markup' => check_markup($notes, 'plain_text'),
        '#weight' => 11,
      );
    }
  }
  catch (EntityMetadataWrapperException $e) {}

  // --- Meeting notes — displayed as a collapsed accordion ---
  $meeting_items = field_get_items('crm_activity', $activity, 'field_crm_meeting_notes');
  if ($meeting_items && !empty($meeting_items[0]['value'])) {
    $format = !empty($meeting_items[0]['format']) ? $meeting_items[0]['format'] : 'filtered_html';
    $output['meeting_notes_heading'] = array(
      '#markup' => drop_base_section_header(t('Meeting Notes')),
      '#weight' => 20,
    );
    $output['meeting_notes'] = array(
      '#markup' => '<details class="crm-accordion">'
        . '<summary class="crm-accordion-summary">' . t('Show / Hide') . '</summary>'
        . '<div class="crm-accordion-body">'
        . check_markup($meeting_items[0]['value'], $format)
        . '</div>'
        . '</details>',
      '#weight' => 21,
    );
  }

  return $output;
}

/**
 * Page callback: display a single CRM contact method.
 *
 * @param CrmContactMethod $method
 *   The contact method entity to display.
 *
 * @return array
 *   A render array.
 */
function dropcrm_contact_method_view(CrmContactMethod $method) {
  backdrop_set_title(entity_label('crm_contact_method', $method), PASS_THROUGH);

  $crumb = array(
    l(t('Home'), '<front>'),
    l(t('Drop Suite'), 'drop'),
    l(t('CRM'), 'drop/crm'),
  );
  if (!empty($method->cid)) {
    $contact = crm_contact_load($method->cid);
    if ($contact) {
      $crumb[] = l(t('Contacts'), 'drop/crm/contacts');
      $crumb[] = l(entity_label('crm_contact', $contact), 'drop/crm/contact/' . $method->cid);
    }
  }
  backdrop_set_breadcrumb($crumb);

  return entity_view($method, 'full');
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/**
 * Returns a key => label array of relationship types from config.
 *
 * @return array
 */
function _dropcrm_relationship_type_options() {
  $types = config_get('dropcrm.settings', 'relationship_types');
  if (empty($types)) {
    // Fall back to a minimal set if config has not been populated yet.
    return array(
      'referral'     => t('Referral / Connection'),
      'spouse'       => t('Spouse / Partner'),
      'parent_child' => t('Parent / Child'),
      'sibling'      => t('Sibling'),
      'friend'       => t('Friend'),
      'colleague'    => t('Colleague'),
      'other'        => t('Other'),
    );
  }
  $options = array();
  foreach ($types as $key => $info) {
    $options[$key] = t($info['label']);
  }
  return $options;
}

/**
 * Loads all relationships for a contact, annotating each with other_cid.
 *
 * Queries both cid_a and cid_b sides so symmetric relationships always
 * appear on both contacts' pages.
 *
 * @param int $cid
 *   The contact ID to load relationships for.
 *
 * @return array
 *   Array of stdClass objects from crm_relationship, each with an added
 *   'other_cid' property set to the CID of the other contact in the pair.
 */
function _dropcrm_load_contact_relationships($cid) {
  $cid = (int) $cid;
  $result = db_select('crm_relationship', 'r')
    ->fields('r')
    ->condition(
      db_or()
        ->condition('cid_a', $cid)
        ->condition('cid_b', $cid)
    )
    ->orderBy('created', 'DESC')
    ->execute()
    ->fetchAll();

  foreach ($result as $row) {
    $row->other_cid = ($row->cid_a == $cid) ? (int) $row->cid_b : (int) $row->cid_a;
  }
  return $result;
}

/**
 * Loads all contact methods for a contact, keyed by bundle type.
 *
 * @param int $cid
 *   Contact ID.
 *
 * @return array
 *   Array of arrays, keyed by bundle type (email, telephone, address).
 */
function _dropcrm_load_contact_methods($cid) {
  $result = db_select('crm_contact_method', 'm')
    ->fields('m', array('cmid', 'type', 'label', 'value', 'is_primary'))
    ->condition('cid', $cid)
    ->orderBy('type')
    ->orderBy('is_primary', 'DESC')
    ->execute();

  $grouped = array();
  foreach ($result as $row) {
    $grouped[$row->type][] = $row;
  }
  return $grouped;
}

/**
 * Returns a human label for a contact method bundle.
 *
 * @param string $type
 *   Bundle machine name.
 *
 * @return string
 */
function _dropcrm_contact_method_bundle_label($type) {
  $labels = array(
    'email'     => t('Email'),
    'telephone' => t('Telephone'),
    'address'   => t('Address'),
    'url'       => t('Website & Social'),
  );
  return isset($labels[$type]) ? $labels[$type] : ucfirst($type);
}

/**
 * Builds a custom render array for a crm_person contact's core fields.
 *
 * Combines first/middle/last name onto one line and labels the employer
 * field clearly. Avoids relying on the field display system, which renders
 * each name part as a separate unlabelled row.
 *
 * @param CrmContact $contact
 *   A contact entity with bundle crm_person.
 *
 * @return array
 *   A render array for the person's details block.
 */
function _dropcrm_person_details(CrmContact $contact) {
  try {
    $w = entity_metadata_wrapper('crm_contact', $contact);
  }
  catch (EntityMetadataWrapperException $e) {
    return array();
  }

  $items = array();

  // Rating.
  $rating_items = field_get_items('crm_contact', $contact, 'field_crm_rating');
  if ($rating_items && !empty($rating_items[0]['value'])) {
    $stars = _dropcrm_render_stars((int) $rating_items[0]['value']);
    if ($stars) {
      $items[] = array('label' => t('Rating'), 'value' => $stars);
    }
  }

  // Job title.
  $job = $w->field_crm_job_title->value();
  if ($job) {
    $items[] = array('label' => t('Job Title'), 'value' => check_plain($job));
  }

  // Affiliated organization(s) — field is unlimited cardinality.
  try {
    $org_links = array();
    foreach ($w->field_crm_employer->getIterator() as $item) {
      $org = $item->value();
      if ($org) {
        $org_links[] = l(entity_label('crm_contact', $org), 'drop/crm/contact/' . $org->cid);
      }
    }
    if ($org_links) {
      $items[] = array(
        'label'  => format_plural(count($org_links), 'Organization', 'Organizations'),
        'value'  => implode(', ', $org_links),
      );
    }
  }
  catch (EntityMetadataWrapperException $e) {}

  // Website.
  try {
    $website = $w->field_crm_website->value();
    if ($website) {
      $items[] = array('label' => t('Website'), 'value' => l(check_plain($website), $website));
    }
  }
  catch (EntityMetadataWrapperException $e) {}

  // Tags.
  $tag_items = field_get_items('crm_contact', $contact, 'field_crm_tags');
  if ($tag_items) {
    $tag_links = array();
    foreach ($tag_items as $item) {
      if (!empty($item['tid'])) {
        $term = taxonomy_term_load($item['tid']);
        if ($term) {
          $tag_links[] = l($term->name, 'drop/crm/people',
            array('query' => array('tag' => $item['tid'])));
        }
      }
    }
    if ($tag_links) {
      $items[] = array('label' => t('Tags'), 'value' => implode(', ', $tag_links));
    }
  }

  // Status — only show when inactive; active is the silent default.
  if (!$contact->status) {
    $items[] = array('label' => t('Status'), 'value' => t('Inactive'));
  }

  if (!$items) {
    return array();
  }

  $html = '<dl class="crm-contact-details">';
  foreach ($items as $item) {
    $html .= '<dt>' . check_plain($item['label']) . '</dt>';
    $html .= '<dd>' . $item['value'] . '</dd>';
  }
  $html .= '</dl>';

  return array(
    '#markup' => $html,
    '#weight' => 0,
  );
}

/**
 * Loads people whose Organization field points to a given organization.
 *
 * @param int $cid
 *   The organization's contact ID.
 *
 * @return array
 *   Array of CrmContact entities (crm_person bundle), sorted by name.
 */
function _dropcrm_load_org_contacts($cid) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'crm_contact')
    ->entityCondition('bundle', 'crm_person')
    ->fieldCondition('field_crm_employer', 'target_id', $cid)
    ->propertyOrderBy('name', 'ASC');

  $result = $query->execute();
  if (empty($result['crm_contact'])) {
    return array();
  }

  return crm_contact_load_multiple(array_keys($result['crm_contact']));
}

/**
 * Loads activities linked to a contact via the field_crm_contacts entity reference.
 *
 * @param int $cid
 *   Contact ID.
 *
 * @return array
 *   Array of CrmActivity entities, sorted by activity_date descending.
 */
function _dropcrm_load_contact_activities($cid) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'crm_activity')
    ->fieldCondition('field_crm_contacts', 'target_id', $cid)
    ->propertyOrderBy('activity_date', 'DESC');

  $result = $query->execute();
  if (empty($result['crm_activity'])) {
    return array();
  }

  return crm_activity_load_multiple(array_keys($result['crm_activity']));
}

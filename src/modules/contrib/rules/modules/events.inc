<?php
/**
 * @file
 * Invokes events on behalf core modules.
 *
 * For non-core modules, the code to invoke events should be found in the
 * module providing rules integration.
 *
 * @addtogroup rules
 *
 * @{
 */

/**
 * Gets an unchanged entity that doesn't contain any recent changes.
 *
 * This handler assumes the name of the variable for the changed entity is the
 * same as for the unchanged entity but without the trailing "_unchanged"; e.g.,
 * for the "node_unchanged" variable the handler assumes there is a "node"
 * variable.
 */
function rules_events_entity_unchanged($arguments, $name, $info) {
  // Cut of the trailing _unchanged.
  $var_name = substr($name, 0, -10);
  $entity = $arguments[$var_name];
  if (isset($entity->original)) {
    return $entity->original;
  }
}

/*
 * Generic entity events, used for core-entities for which we provide Rules
 * integration only. We are implementing the generic-entity hooks instead of
 * the entity-type specific hooks to ensure we come last.
 * See https://www.drupal.org/node/1211946 for details.
 */

/**
 * Implements hook_entity_view().
 */
function rules_entity_view($entity, $type, $view_mode, $langcode) {
  switch ($type) {
    case 'comment':
      rules_invoke_event('comment_view--' . $entity->node_type, $entity, $view_mode);
      rules_invoke_event('comment_view', $entity, $view_mode);
      break;

    case 'node':
      rules_invoke_event('node_view--' . $entity->type, $entity, $view_mode);
      rules_invoke_event('node_view', $entity, $view_mode);
      break;

    case 'user':
      rules_invoke_event('user_view', $entity, $view_mode);
      break;
  }
}

/**
 * Implements hook_entity_presave().
 */
function rules_entity_presave($entity, $type) {
  switch ($type) {
    case 'comment':
      rules_invoke_event('comment_presave--' . $entity->node_type, $entity);
      rules_invoke_event('comment_presave', $entity);
      break;

    case 'node':
      rules_invoke_event('node_presave--' . $entity->type, $entity);
      rules_invoke_event('node_presave', $entity);
      break;

    case 'taxonomy_term':
      rules_invoke_event('taxonomy_term_presave--' . $entity->vocabulary, $entity);
      rules_invoke_event('taxonomy_term_presave', $entity);
      break;

    case 'user':
      rules_invoke_event('user_presave', $entity);
      break;
  }
}

/**
 * Implements hook_entity_update().
 */
function rules_entity_update($entity, $type) {
  switch ($type) {
    case 'comment':
      rules_invoke_event('comment_update--' . $entity->node_type, $entity);
      rules_invoke_event('comment_update', $entity);
      break;

    case 'node':
      rules_invoke_event('node_update--' . $entity->type, $entity);
      rules_invoke_event('node_update', $entity);
      break;

    case 'taxonomy_term':
      rules_invoke_event('taxonomy_term_update--' . $entity->vocabulary, $entity);
      rules_invoke_event('taxonomy_term_update', $entity);
      break;

    case 'user':
      rules_invoke_event('user_update', $entity);
      break;
  }
}

/**
 * Implements hook_entity_insert().
 */
function rules_entity_insert($entity, $type) {
  switch ($type) {
    case 'comment':
      rules_invoke_event('comment_insert--' . $entity->node_type, $entity);
      rules_invoke_event('comment_insert', $entity);
      break;

    case 'node':
      rules_invoke_event('node_insert--' . $entity->type, $entity);
      rules_invoke_event('node_insert', $entity);
      break;

    case 'taxonomy_term':
      rules_invoke_event('taxonomy_term_insert--' . $entity->vocabulary, $entity);
      rules_invoke_event('taxonomy_term_insert', $entity);
      break;

    case 'user':
      rules_invoke_event('user_insert', $entity);
      break;
  }
}

/**
 * Implements hook_entity_delete().
 */
function rules_entity_delete($entity, $type) {
  switch ($type) {
    case 'comment':
      rules_invoke_event('comment_delete--' . $entity->node_type, $entity);
      rules_invoke_event('comment_delete', $entity);
      break;

    case 'node':
      rules_invoke_event('node_delete--' . $entity->type, $entity);
      rules_invoke_event('node_delete', $entity);
      break;

    case 'taxonomy_term':
      rules_invoke_event('taxonomy_term_delete--' . $entity->vocabulary, $entity);
      rules_invoke_event('taxonomy_term_delete', $entity);
      break;

    case 'user':
      rules_invoke_event('user_delete', $entity);
      break;
  }
}

/**
 * Implements hook_user_login().
 */
function rules_user_login(&$edit, $account) {
  rules_invoke_event('user_login', $account);
}

/**
 * Implements hook_user_logout().
 */
function rules_user_logout($account) {
  rules_invoke_event('user_logout', $account);
}

/*
 * System events.
 *
 * Note that rules_init() is the main module file is used to
 * invoke the init event.
 */

/**
 * Implements hook_cron().
 */
function rules_cron() {
  rules_invoke_event('cron');
}

/**
 * Implements hook_watchdog().
 */
function rules_watchdog($log_entry) {
  rules_invoke_event('watchdog', $log_entry);
}

/**
 * Getter callback for the log entry message property.
 */
function rules_system_log_get_message($log_entry) {
  return t($log_entry['message'], (array) $log_entry['variables']);
}

/**
 * Gets all view modes of an entity for an entity_view event.
 */
function rules_get_entity_view_modes($name, $var_info) {
  // Read the entity type from a special key out of the variable info.
  $entity_type = $var_info['options list entity type'];
  $info = entity_get_info($entity_type);
  foreach ($info['view modes'] as $mode => $mode_info) {
    $modes[$mode] = $mode_info['label'];
  }
  return $modes;
}

/**
 * Implements hook_taxonomy_vocabulary_presave().
 */
function rules_taxonomy_vocabulary_presave(TaxonomyVocabulary $vocabulary) {
  // Since core doesn't add (yet) the original vocabulary
  // add it here for presave and update events. 
  if (empty($vocabulary->original)) {
    $config = config('taxonomy.vocabulary.' . $vocabulary->machine_name);
    $is_new = $config->isNew();
    if (!$is_new) {
      $data = $config->get();
      $original = new TaxonomyVocabulary($data);
      $vocabulary->original = $original;
    }
  }
  rules_invoke_event('taxonomy_vocabulary_presave', $vocabulary);
}

/**
 * Implements hook_taxonomy_vocabulary_update().
 */
function rules_taxonomy_vocabulary_update(TaxonomyVocabulary $vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_update', $vocabulary);
}

/**
 * Implements hook_taxonomy_vocabulary_insert().
 */
function rules_taxonomy_vocabulary_insert(TaxonomyVocabulary $vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_insert', $vocabulary);
}

/**
 * Implements hook_taxonomy_vocabulary_delete().
 */
function rules_taxonomy_vocabulary_delete(TaxonomyVocabulary $vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_delete', $vocabulary);
}

/**
 * Implements hook_taxonomy_vocabulary_predelete().
 */
function rules_taxonomy_vocabulary_predelete(TaxonomyVocabulary $vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_predelete', $vocabulary);
}

/**
 * Gets an unchanged vocabulary that doesn't contain any recent changes.
 */
function rules_events_vocabulary_unchanged($arguments, $name, $info) {
  $vocabulary = $arguments['vocabulary'];
  if (isset($vocabulary->original)) {
    return $vocabulary->original;
  }
}

/**
 * @} End of "addtogroup rules"
 */

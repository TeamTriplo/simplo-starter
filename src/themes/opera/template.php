<?php

/**
 * Prepares variables for layout templates.
 *
 * @see layout.tpl.php
 */
function opera_preprocess_layout(&$variables) {
  if ($variables['is_front']) {
    // Add a special front-page class.
    $variables['classes'][] = 'layout-front';
    // Add a special front-page template suggestion.
    $original = $variables['theme_hook_original'];
    $variables['theme_hook_suggestions'][] = $original . '__front';
    $variables['theme_hook_suggestion'] = $original . '__front';
  }
}
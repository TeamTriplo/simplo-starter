<?php
/**
 * @file
 * Theme functions for the biblio module.
 */

/**
 * Theme function: Styles biblio author list(s).
 * @param  array $variables
 *   Variables passed into the theme function.
 *
 * @return string $output
 */
function theme_biblio_authors($variables) {
  $role = $variables['role'];
  $authors = $variables['authors'];

  // Hack! Make each look like it's a field.
  $output  = '<div class="field field-title field-label-inline clearfix">';
  $output .=   '<div class="field-label">';
  $output .=      $role . ':';
  $output .=   '</div>';
  $output .=   '<div class="field-items">';
  $output .=     '<div class="field-item even">';
  $output .=        $authors;
  $output .=     '</div>';
  $output .=   '</div>';
  $output .= '</div>';

  return $output;
}

<?php

/**
 * @file
 * Display Suite 1 column template.
 */

if (is_array($classes)) {
  $classes = implode(' ', $classes);
}

?>

<<?php print $ds_content_wrapper; print $layout_attributes; ?> class="ds-1col <?php print $classes; ?> clearfix">

  <?php if (isset($title_suffix['contextual_links'])): ?>
  <?php print render($title_suffix['contextual_links']); ?>
  <?php endif; ?>

  <?php print $ds_content; ?>
</<?php print $ds_content_wrapper ?>>

<?php if (!empty($backdrop_render_children)): ?>
  <?php print $backdrop_render_children ?>
<?php endif; ?>

<?php
/**
 * @file
 * Template for outputting the default block styling within a Layout.
 *
 * Variables available:
 * - $classes: Array of classes that should be displayed on the block's wrapper.
 * - $title: The title of the block.
 * - $title_prefix/$title_suffix: A prefix and suffix for the title tag. This
 *   is important to print out as administrative links to edit this block are
 *   printed in these variables.
 * - $content: The actual content of the block.
 */
?>
<div class="<?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>

<?php if ($variables['is_front']) {
	    
	} ?>

<?php print render($title_prefix); ?>
<?php if ($title): ?>
  <?php if ($variables['is_front']): ?>
  	<h2 class="block-title container"><?php print $title; ?></h2>
  <?php else: ?>
    <h2 class="block-title"><?php print $title; ?></h2>
  <?php endif; ?>
<?php endif; ?>
<?php print render($title_suffix); ?>
  <?php if ($variables['is_front']): ?>
  	  <div class="block-content container">
	    <?php print render($content); ?>
	  </div>
  <?php else: ?>
  	  <div class="block-content">
	    <?php print render($content); ?>
	  </div>
  <?php endif; ?>
</div>

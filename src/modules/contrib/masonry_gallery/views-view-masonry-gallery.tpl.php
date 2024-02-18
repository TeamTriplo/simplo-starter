<?php
/**
 * @file
 * Template for the Masonry Gallery style.
 *
 * - $images_rendered    array of rendered images.
 * - $captions           array of caption texts.
 * - $masonry_id         unique id for this gallery instance.
 * - $custom_css         generated styles for customized captions. Only required for preview mode.
 * - $loader             defines if loader is needed. Can be either TRUE or FALSE/undefined.
 *
 * @ingroup views_templates
 */
?>

<?php if(isset($view->live_preview)): ?>
<style>
<?php print $custom_css;?>
</style>
<?php endif; ?>

<?php if (!empty($title)) : ?>
<h3><?php print $title; ?></h3>
<?php endif; ?>

<div id="<?php print $masonry_id ?>" class="masonry-gallery">
  <?php foreach ($images_rendered as $i => $image): ?>
    <div data-id="<?php print $i ?>" class="masonry-gallery-item">
      <?php if(!empty($image)):?>
        <?php print $image; ?>
        <?php if(!empty($loader)):?>
          <div class="masonry-gallery-loader"></div>
        <?php endif ?>
        <?php if(isset($captions[$i])): ?>
          <div<?php if(!empty($caption_class)): print(' class="' . $caption_class . '"'); endif; print('>' . $captions[$i]); ?></div>
        <?php endif ?>
      <?php endif ?>
    </div>
  <?php endforeach; ?>
</div>

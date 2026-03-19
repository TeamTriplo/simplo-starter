<?php
/**
 * @file
 * Template to display Bootstrap Carousel images.
 *
 * Variables available (beyond standard Views variables):
 * - $title: The title of each carousel element.
 * - $description: The description of each carousel element.
 *
 * @ingroup views_templates
 */
?>
<?php print $image ?>
<?php if (!empty($title) || !empty($description)): ?>
  <div class="carousel-caption">
    <?php if (!empty($title)): ?>
      <h3><?php print $title ?></h3>
    <?php endif ?>

    <?php if (!empty($description)): ?>
      <p><?php print $description ?></p>
    <?php endif ?>
  </div>
<?php endif ?>

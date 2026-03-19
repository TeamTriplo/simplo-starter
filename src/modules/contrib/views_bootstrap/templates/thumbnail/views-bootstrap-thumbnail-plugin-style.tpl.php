<?php
/**
 * @file
 * Template to display Bootstrap Thumbnails.
 *
 * Variables available (beyond standard Views variables):
 * - $options: Array of settings to specify the layout of the thumbnails.
 * - $items: Items distributed for vertical alignment.
 * - $col_classes: Classes on individual columns as space-delimited names.
 *
 * @ingroup views_templates
 */
?>
<?php if (!empty($title)): ?>
  <h3><?php print $title ?></h3>
<?php endif ?>

<div id="views-bootstrap-thumbnail-<?php print $id ?>" class="<?php print implode(' ', $classes) ?>">
  <?php if ($options['alignment'] == 'horizontal'): ?>

    <?php foreach ($items as $row): ?>
      <div class="row">
        <?php foreach ($row['content'] as $key => $column): ?>
          <div class="<?php print $col_classes ?>">
            <div class="thumbnail">
              <?php print $column['content'] ?>
            </div>
          </div>

          <?php /* Add clearfix divs if required */ ?>
          <?php if ($options['columns_horizontal'] == -1 && !empty($options['clear_columns']) && $key != 0): ?>
            <?php foreach ($clearfix as $screen => $count): ?>
              <?php if (($key + 1) % $count == 0): ?>
                <div class="clearfix visible-<?php print $screen; ?>-block"></div>
              <?php endif; ?>
            <?php endforeach; ?>
          <?php endif; ?>
        <?php endforeach ?>
      </div>
    <?php endforeach ?>

  <?php else: ?>

    <div class="row">
      <?php foreach ($items as $column): ?>
        <div class="<?php print $col_classes ?>">
          <?php foreach ($column['content'] as $row): ?>
            <div class="thumbnail">
              <?php print $row['content'] ?>
            </div>
          <?php endforeach ?>
        </div>
      <?php endforeach ?>
    </div>

  <?php endif ?>
</div>

<?php
/**
 * @file
 * Template to display Bootstrap Panels.
 *
 * Variables available (beyond standard Views variables):
 * - $headers: The header to use for each row.
 * - $labels: The label to use for each row.
 * - $title_flag: Whether to wrap the header in a special tag.
 * - $title_tag: The tag to wrap the header in.
 * - $footers: The footer to use for each row.
 *
 * @ingroup views_templates
 */
?>
<?php if (!empty($title)): ?>
  <h3><?php print $title ?></h3>
<?php endif ?>

<div id="views-bootstrap-panel-<?php print $id ?>" class="<?php print implode(' ', $classes) ?>">
  <?php foreach ($rows as $key => $row): ?>
    <div class="panel panel-<?php print $context ?>">
      <?php if (isset($headers[$key])): ?>
        <div class="panel-heading">
          <?php if (isset($labels[$key])): ?>
            <span class="badge pull-right"><?php print $labels[$key] ?></span>
          <?php endif ?>
          <?php if ($title_flag): ?>
            <<?php print $title_tag; ?> class="panel-title">
          <?php endif ?>
            <?php print $headers[$key] ?>
          <?php if ($title_flag): ?>
            </<?php print $title_tag; ?>>
          <?php endif ?>
        </div>
      <?php endif ?>

        <div class="panel-body">
          <?php print $row ?>
        </div>

      <?php if (isset($footers[$key])): ?>
        <div class="panel-footer">
          <?php print $footers[$key] ?>
        </div>
      <?php endif ?>

    </div>
  <?php endforeach ?>
</div>

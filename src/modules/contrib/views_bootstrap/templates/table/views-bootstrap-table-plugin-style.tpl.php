<?php
/**
 * @file
 * Template to display a Bootstrap table.
 *
 * Variables available (beyond standard Views variables):
 *
 * - $title : The title of this group of rows.  May be empty.
 * - $header: An array of header labels keyed by field id.
 * - $caption: The caption for this table. May be empty.
 * - $header_classes: An array of header classes keyed by field id.
 * - $fields: An array of CSS IDs to use for each field id.
 * - $classes: A class or classes to apply to the table, based on settings.
 * - $row_classes: An array of classes to apply to each row, indexed by row
 *   number. This matches the index in $rows.
 * - $rows: An array of row items. Each row is an array of content.
 *   $rows are keyed by row number, fields within rows are keyed by field ID.
 * - $field_classes: An array of classes to apply to each field, indexed by
 *   field id, then row number. This matches the index in $rows.
 * - $responsive: A flag indicating whether table is responsive.
 *
 * @ingroup views_templates
 */
?>
<?php if ($responsive): ?>
  <div class="table-responsive">
<?php endif ?>

<table <?php if ($classes) {
  print 'class="' . implode(' ', $classes) . '" ';
} ?>
  <?php print backdrop_attributes($attributes); ?>>
  <?php if (!empty($title) || !empty($caption)) : ?>
    <caption><?php print $caption . $title; ?></caption>
  <?php endif; ?>
  <?php if (!empty($header)) : ?>
    <thead>
    <tr>
      <?php foreach ($header as $field => $label): ?>
        <th <?php if ($header_classes[$field]) {
          print 'class="' . implode(' ', $header_classes[$field]) . '" ';
       } ?>>
          <?php print $label; ?>
        </th>
      <?php endforeach; ?>
    </tr>
    </thead>
  <?php endif; ?>
  <tbody>
  <?php foreach ($rows as $row_count => $row): ?>
    <tr <?php if ($row_classes[$row_count]) {
      print 'class="' . implode(' ', $row_classes[$row_count]) . '"';
   } ?>>
      <?php foreach ($row as $field => $content): ?>
        <td <?php if ($field_classes[$field][$row_count]) {
          print 'class="' . implode(' ', $field_classes[$field][$row_count]) . '" ';
       } ?><?php print backdrop_attributes($field_attributes[$field][$row_count]); ?>>
          <?php print $content; ?>
        </td>
      <?php endforeach; ?>
    </tr>
  <?php endforeach; ?>
  </tbody>
</table>

<?php if ($responsive): ?>
  </div>
<?php endif ?>

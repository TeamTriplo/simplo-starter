<?php
/**
 * @file
 * Template to display a list of rows as a Bootstrap drop-down.
 *
 * Variables available:
 * - $wrapper_prefix: Prefix tag(s) to wrap around the dropdown.
 * - $button_class: Class to apply to the button.
 * - $button_text: Text to use for the button.
 * - $list_type_prefix: Prefix tag(s) to wrap around the dropdown list.
 * - $list_type_suffix: Closing tag(s) for the dropdown list.
 * - $wrapper_suffix: Closing tag(s) for the dropdown.
 *
 * @ingroup views_templates
*/
?>
<?php print $wrapper_prefix; ?>
  <button class="dropdown-toggle btn <?php print $button_class; ?>" type="button" id="<?php print $id; ?>-dropdown" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
    <?php print $button_text; ?> <span class="caret"></span>
  </button>
  <?php print $list_type_prefix; ?>
<?php foreach ($rows as $key => $row): ?>
    <li <?php print $row['attributes']; ?>><?php print strip_tags($row['content'], '<a><img>'); ?></li>
<?php endforeach ?>
  <?php print $list_type_suffix; ?>
<?php print $wrapper_suffix; ?>

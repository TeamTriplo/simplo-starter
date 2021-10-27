<?php
/**
 * @file
 * Theme settings file for Basis.
 *
 * Although Basis itself does not provide any settings, we use this file to
 * inform the user that the module supports color schemes if the Color module
 * is enabled.
 */

if (module_exists('color')) {

  // Color Set One
  $form['set_one'] = array(
    '#type' => 'fieldset',
    '#title' => t('Header and Footer'),
    '#collapsible' => TRUE,
  );
  $fields = array(
    'set1bg',
    'set1text',
    'set1links',
  );
  foreach ($fields as $field) {
    $form['set_one'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }

  // Color Set Two
  $form['set_two'] = array(
    '#type' => 'fieldset',
    '#title' => t('Hero Blocks without Images'),
    '#collapsible' => TRUE,
  );
  $fields = array(
    'set2bg',
    'set2text',
    'set2links',
  );
  foreach ($fields as $field) {
    $form['set_two'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }

  // Color Set Three
  $form['set_three'] = array(
    '#type' => 'fieldset',
    '#title' => t('Blocks - Set 1'),
    '#collapsible' => TRUE,
    '#description' => 'Blocks 2,5,8,... of content region.',
  );
  $fields = array(
    'set3bg',
    'set3text',
    'set3links',
  );
  foreach ($fields as $field) {
    $form['set_three'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }

  // Color Set Four
  $form['set_four'] = array(
    '#type' => 'fieldset',
    '#title' => t('Blocks - Set 2'),
    '#collapsible' => TRUE,
    '#description' => 'Blocks 3,6,9,... of content region.',
  );
  $fields = array(
    'set4bg',
    'set4text',
    'set4links',
  );
  foreach ($fields as $field) {
    $form['set_four'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }
   // Color Set Five
  $form['set_five'] = array(
    '#type' => 'fieldset',
    '#title' => t('Blocks - Set 3'),
    '#collapsible' => TRUE,
    '#description' => 'Blocks 4,7,10,... of content region.',
  );
  $fields = array(
    'set5bg',
    'set5text',
    'set5links',
  );
  foreach ($fields as $field) {
    $form['set_five'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }
}
else {
  $form['color'] = array(
    '#markup' => '<p>' . t('This theme supports custom color palettes if the Color module is enabled on the <a href="!url">modules page</a>. Enable the Color module to customize this theme.', array('!url' => url('admin/modules'))) . '</p>',
  );
}
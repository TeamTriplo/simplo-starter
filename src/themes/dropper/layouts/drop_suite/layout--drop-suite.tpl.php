<?php
/**
 * @file
 * Template for the Drop Suite layout.
 *
 * A two-column layout (main content + right sidebar) designed for Drop Suite
 * application pages. Based on Moscone Flipped with an added Navigation region.
 *
 * Regions:
 * - $content['header']:     Site header (logo, site name).
 * - $content['navigation']: Drop Suite primary navigation bar.
 * - $content['top']:        Full-width region above the main columns.
 * - $content['content']:    Main content column (3/4 width).
 * - $content['sidebar']:    Right sidebar column (1/4 width).
 * - $content['bottom']:     Full-width region below the main columns.
 * - $content['footer']:     Site footer.
 */
?>
<div class="layout--drop-suite <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>

  <?php if ($content['header']): ?>
    <header class="l-header" role="banner" aria-label="<?php print t('Site header'); ?>">
      <div class="l-header-inner">
        <?php print $content['header']; ?>
      </div>
    </header>
  <?php endif; ?>

  <?php if ($content['navigation']): ?>
    <nav class="l-navigation" role="navigation" aria-label="<?php print t('Drop Suite navigation'); ?>">
      <div class="l-navigation-inner">
        <?php print $content['navigation']; ?>
      </div>
    </nav>
  <?php endif; ?>

  <div class="l-wrapper">
    <div class="l-wrapper-inner">

      <?php if (!empty($content['top'])): ?>
        <div class="l-top">
          <?php print $content['top']; ?>
        </div>
      <?php endif; ?>

      <div class="l-middle row">
        <main class="l-content col-md-9" role="main" aria-label="<?php print t('Main content'); ?>">
          <?php print $content['content']; ?>
        </main>
        <aside class="l-sidebar col-md-3" aria-label="<?php print t('Sidebar'); ?>">
          <?php print $content['sidebar']; ?>
        </aside>
      </div><!-- /.l-middle -->

      <?php if (!empty($content['bottom'])): ?>
        <div class="l-bottom">
          <?php print $content['bottom']; ?>
        </div>
      <?php endif; ?>

    </div><!-- /.l-wrapper-inner -->
  </div><!-- /.l-wrapper -->

  <?php if ($content['footer']): ?>
    <footer class="l-footer" role="contentinfo">
      <div class="l-footer-inner">
        <?php print $content['footer']; ?>
      </div>
    </footer>
  <?php endif; ?>

</div><!-- /.layout--drop-suite -->

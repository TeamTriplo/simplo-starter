(function ($) {
  /**
   * Appends a "Remove from dashboard" link to each block's contextual links
   * menu on the admin dashboard, and hides the fallback × button when the
   * contextual menu is available.
   */
  Backdrop.behaviors.dashboardPlusRemoveBlock = {
    attach: function (context) {
      $('.dashboard-plus-remove-trigger', context).once('dashboard-plus-remove').each(function () {
        var $trigger = $(this);
        var url = $trigger.data('remove-url');
        var $block = $trigger.closest('.block');
        var $contextualList = $block.find('.contextual-links');

        if ($contextualList.length) {
          // Add to the existing contextual links menu and hide the × button.
          $contextualList.append(
            '<li class="dashboard-plus-remove-from-dashboard">' +
            '<a href="' + url + '">' + Backdrop.t('Remove from dashboard') + '</a>' +
            '</li>'
          );
          $block.find('.dashboard-plus-remove-block').hide();
        }
        // If no contextual links menu exists, the × button remains visible.
      });
    }
  };
}(jQuery));

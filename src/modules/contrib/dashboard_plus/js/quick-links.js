(function ($) {

  Backdrop.behaviors.dashboardPlusQuickLinks = {
    attach: function (context, settings) {
      $('.dashboard-plus-ql-trigger', context)
        .once('dashboard-plus-quick-links')
        .on('click', function (e) {
          e.preventDefault();
          // The form wrapper is a sibling inside the same block-content div.
          $(this).closest('.block-content')
            .find('.dashboard-plus-ql-form-wrapper')
            .slideToggle(180);
        });
    }
  };

}(jQuery));

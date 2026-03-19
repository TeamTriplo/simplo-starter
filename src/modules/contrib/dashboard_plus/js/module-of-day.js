(function ($) {
  Backdrop.behaviors.dashboardPlusModuleOfDay = {
    attach: function (context, settings) {

      // "Read more" toggle for truncated descriptions.
      $('.module-desc-toggle', context).once('motd-read-more').on('click', function (e) {
        e.preventDefault();
        var $desc = $(this).closest('.module-description');
        $desc.find('.module-desc-short').hide();
        $desc.find('.module-desc-full').show();
        $(this).hide();
      });

      // "See another" button — fetch a new module via AJAX.
      $('.dashboard-motd-next', context).once('motd-next').on('click', function (e) {
        e.preventDefault();
        var $btn = $(this);
        var $wrapper = $btn.closest('.dashboard-module-of-day-wrapper');
        var url = $wrapper.data('motd-url');

        $btn.prop('disabled', true).text(Backdrop.t('Loading\u2026'));

        $.ajax({
          url: url,
          type: 'POST',
          dataType: 'json',
          success: function (response) {
            if (response.html) {
              $wrapper.find('.dashboard-module-of-day-content').html(response.html);
              Backdrop.attachBehaviors($wrapper[0]);
            }
          },
          complete: function () {
            $btn.prop('disabled', false).text(Backdrop.t('See another'));
          }
        });
      });

    }
  };
}(jQuery));

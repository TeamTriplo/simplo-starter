/**
 * @file
 * DropTime — live elapsed time display and AJAX timer controls.
 */
(function ($) {

  /**
   * Live elapsed time ticker.
   *
   * Reads the start timestamp from the data-timer-start attribute on the
   * .droptime-timer-running element so this works both on page load and after
   * an AJAX start (no Backdrop.settings dependency).
   */
  Backdrop.behaviors.droptimeTimer = {
    attach: function (context, settings) {
      $('.droptime-timer-running', context).once('droptime-ticker').each(function () {
        var $banner  = $(this);
        var startTs  = parseInt($banner.data('timerStart'), 10) * 1000;
        var $elapsed = $banner.find('.droptime-elapsed');

        function update() {
          var secs  = Math.floor((Date.now() - startTs) / 1000);
          var h     = Math.floor(secs / 3600);
          var m     = Math.floor((secs % 3600) / 60);
          var s     = secs % 60;
          var parts = [];
          if (h) { parts.push(h + 'h'); }
          if (m || h) { parts.push(m + 'm'); }
          parts.push(s + 's');
          $elapsed.text('(' + parts.join(' ') + ')');
        }

        update();
        setInterval(update, 1000);
      });
    }
  };

  /**
   * AJAX timer start — fires the start endpoint without a page reload.
   *
   * On success, inserts the server-rendered banner into #droptime-banner-area
   * and hides the start link. Falls back to a full page reload on error.
   */
  Backdrop.behaviors.droptimeTimerControls = {
    attach: function (context, settings) {
      $('.js-droptime-start', context).once('droptime-start').on('click', function (e) {
        e.preventDefault();
        var url = $(this).data('url');

        $.post(url, {_dt_ajax: '1'}, function (data) {
          if (!data.ts) {
            window.location.reload();
            return;
          }
          var $bannerArea = $('#droptime-banner-area');
          $bannerArea.html(data.bannerHtml);
          $('#droptime-nav-timer-action').hide();
          Backdrop.attachBehaviors($bannerArea[0]);
        }, 'json').fail(function () {
          window.location.reload();
        });
      });
    }
  };

  /**
   * Datepicker for the edit entry form.
   */
  Backdrop.behaviors.droptimeDatepicker = {
    attach: function (context, settings) {
      $('.droptime-datepicker', context).once('droptime-datepicker').datepicker({
        dateFormat: 'yy-mm-dd',
        changeMonth: true,
        changeYear: true
      });
    }
  };

}(jQuery));

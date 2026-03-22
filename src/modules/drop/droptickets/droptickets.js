/**
 * @file
 * DropTickets — datepicker, body toggle, emoji reactions.
 */
(function ($) {
  Backdrop.behaviors.dropticketsDatpicker = {
    attach: function (context, settings) {
      if (!$.fn.datepicker) { return; }
      $('.droptickets-datepicker', context).once('droptickets-datepicker').datepicker({
        dateFormat: 'yy-mm-dd',
        changeMonth: true,
        changeYear: true
      });
    }
  };
}(jQuery));

(function ($) {
  Backdrop.behaviors.dropticketBodyToggle = {
    attach: function (context, settings) {
      $('.droptickets-body--collapsible', context).once('body-toggle').each(function () {
        var $card  = $(this);
        var $inner = $card.find('.droptickets-body-inner');
        var $btn   = $card.find('.droptickets-body-toggle');

        // If content is short enough, hide the toggle and fade entirely.
        if ($inner[0].scrollHeight <= 400) {
          $btn.hide();
          $card.find('.droptickets-body-fade').hide();
          $inner.css('max-height', 'none');
          return;
        }

        $btn.on('click', function () {
          var expanded = $card.hasClass('is-expanded');
          $card.toggleClass('is-expanded', !expanded);
          $btn.attr('aria-expanded', String(!expanded));
          $btn.text(!expanded ? Backdrop.t('Collapse') : Backdrop.t('Expand'));
        });
      });
    }
  };
}(jQuery));

(function ($) {
  Backdrop.behaviors.dropticketReactions = {
    attach: function (context, settings) {
      $('.droptickets-reactions', context).once('reactions').each(function () {
        var $bar = $(this);
        var tid  = $bar.data('tid');
        var tcid = $bar.data('tcid');
        var entityType = (tcid > 0) ? 'comment' : 'ticket';
        var entityId   = (tcid > 0) ? tcid : tid;

        $bar.find('.react-btn').on('click', function () {
          var $btn  = $(this);
          var emoji = $btn.data('emoji');

          $btn.addClass('is-loading');

          $.ajax({
            url: Backdrop.settings.basePath + 'admin/drop/tickets/react/' + entityType + '/' + entityId,
            type: 'POST',
            data: { emoji: emoji },
            dataType: 'json',
            success: function (data) {
              if (!data.reactions) { return; }
              var emojis = (Backdrop.settings.droptickets || {}).reactionEmojis || {};
              $bar.find('.react-btn').each(function () {
                var $b   = $(this);
                var code = $b.data('emoji');
                if (!data.reactions[code]) { return; }
                var info  = data.reactions[code];
                var char  = emojis[code] || code;
                var label = info.count > 0 ? char + ' ' + info.count : char;
                $b.text(label);
                $b.toggleClass('is-active', info.user_reacted);
                // Update tooltip with reactor names.
                var title = info.names && info.names.length
                  ? char + ' ' + info.names.join(' · ')
                  : char;
                $b.attr('title', title);
              });
            },
            complete: function () {
              $btn.removeClass('is-loading');
            }
          });
        });
      });
    }
  };
}(jQuery));

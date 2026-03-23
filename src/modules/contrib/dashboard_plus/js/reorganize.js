(function ($) {

  Backdrop.behaviors.dashboardPlusReorganize = {
    attach: function (context, settings) {
      if (!settings.dashboardPlus || !settings.dashboardPlus.reorganize) {
        return;
      }
      var data = settings.dashboardPlus.reorganize;

      // Bind to the trigger link rendered by the Welcome block.
      $('.dashboard-plus-reorganize-trigger', context)
        .once('dashboard-plus-reorganize')
        .on('click', function (e) {
          e.preventDefault();
          _openModal(data);
        });
    }
  };

  /**
   * Builds and opens the reorganize modal.
   */
  function _openModal(data) {
    var $modal = $('<div id="dashboard-plus-reorganize-modal"></div>');
    var $regions = $('<div class="reorganize-regions"></div>');

    $.each(data.regions, function (regionKey, regionData) {
      var cls = 'reorganize-region' + (regionData.full_width ? ' reorganize-region--full' : '');
      var $col = $('<div></div>').addClass(cls).attr('data-region', regionKey);
      $col.append('<h3 class="reorganize-region-label">' + _esc(regionData.label) + '</h3>');

      var $list = $('<ul class="reorganize-sortable"></ul>');
      $.each(regionData.blocks, function (i, block) {
        $('<li class="reorganize-block"></li>')
          .attr('data-uuid', block.uuid)
          .append('<span class="reorganize-handle" aria-hidden="true">\u2630</span>')
          .append($('<span class="reorganize-block-label"></span>').text(block.label))
          .appendTo($list);
      });
      $col.append($list);
      $regions.append($col);
    });

    var $status = $('<p class="reorganize-status" aria-live="polite"></p>');
    $modal.append($regions).append($status);
    $('body').append($modal);

    $modal.dialog({
      title: Backdrop.t('Reorganize dashboard layout'),
      modal: true,
      width: Math.min($(window).width() - 40, 920),
      maxHeight: $(window).height() - 80,
      buttons: [
        {
          text: Backdrop.t('Save layout'),
          'class': 'button button-primary',
          click: function () { _save(data, $modal, $status); }
        },
        {
          text: Backdrop.t('Cancel'),
          'class': 'button',
          click: function () { $modal.dialog('close'); }
        }
      ],
      close: function () { $modal.remove(); }
    });

    // Init connected sortable lists — must run after dialog opens.
    $modal.find('.reorganize-sortable').sortable({
      connectWith: '.reorganize-sortable',
      handle: '.reorganize-handle',
      placeholder: 'reorganize-placeholder',
      forcePlaceholderSize: true,
      tolerance: 'pointer'
    }).disableSelection();
  }

  /**
   * POSTs the new positions to the save endpoint.
   */
  function _save(data, $modal, $status) {
    var positions = {};

    $modal.find('.reorganize-region').each(function () {
      var region = $(this).data('region');
      var uuids = [];
      $(this).find('.reorganize-block').each(function () {
        uuids.push($(this).data('uuid'));
      });
      positions[region] = uuids;
    });

    $status.text(Backdrop.t('Saving\u2026'));

    $.post(data.saveUrl, {
      token: data.token,
      positions: JSON.stringify(positions)
    })
    .done(function (response) {
      if (response && response.success) {
        $modal.dialog('close');
        window.location.reload();
      }
      else {
        var msg = (response && response.message) ? response.message : 'unknown error';
        $status.text(Backdrop.t('Could not save: !msg', {'!msg': msg}));
      }
    })
    .fail(function () {
      $status.text(Backdrop.t('Save failed — please try again.'));
    });
  }

  function _esc(str) {
    return $('<div>').text(str).html();
  }

}(jQuery));

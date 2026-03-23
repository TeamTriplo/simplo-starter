(function ($) {

  Backdrop.behaviors.dashboardPlusContentPicker = {
    attach: function (context, settings) {
      var nodes = (settings.dashboardPlusContentPicker || {}).nodes;
      if (!nodes || !nodes.length) {
        return;
      }

      $('.dashboard-plus-type-filter', context)
        .once('dashboard-plus-content-picker')
        .on('change', function () {
          var selectedType = $(this).val();
          var $nodeSelect = $(this)
            .closest('.dashboard-plus-picker-row')
            .find('.dashboard-plus-node-select');

          _rebuildOptions($nodeSelect, nodes, selectedType);
        });
    }
  };

  /**
   * Rebuilds the node <select> options based on the selected content type.
   *
   * Shows all types as optgroups when no type is selected; shows a flat list
   * when a specific type is chosen.
   */
  function _rebuildOptions($select, nodes, type) {
    var currentVal = $select.val();
    $select.empty().append($('<option>', { value: '', text: Backdrop.t('-- Select content --') }));

    if (type === '') {
      // Group nodes into optgroups by content type.
      var groups = {};
      $.each(nodes, function (i, node) {
        if (!groups[node.type]) {
          groups[node.type] = { label: node.type_label, items: [] };
        }
        groups[node.type].items.push(node);
      });

      $.each(groups, function (typeKey, group) {
        var $group = $('<optgroup>', { label: group.label });
        $.each(group.items, function (i, node) {
          $group.append($('<option>', { value: node.nid, text: node.title }));
        });
        $select.append($group);
      });
    }
    else {
      // Flat list for the selected type only.
      $.each(nodes, function (i, node) {
        if (node.type === type) {
          $select.append($('<option>', { value: node.nid, text: node.title }));
        }
      });
    }

    // Restore previous selection if it's still in the new list.
    if (currentVal) {
      $select.val(currentVal);
    }
  }

}(jQuery));

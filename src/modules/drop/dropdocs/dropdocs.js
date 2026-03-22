/**
 * @file
 * DropDocs — collapsible tree navigation.
 *
 * Parent items in the doc tree can be toggled open or closed. The collapsed
 * state is persisted in localStorage. The path to the currently active item
 * is always kept expanded regardless of saved state.
 */
(function ($) {
  'use strict';

  var STORAGE_KEY = 'dropdocs_expanded';

  /**
   * Returns the set of nids the person has manually expanded.
   * Default state is collapsed, so only explicitly opened items are stored.
   *
   * @return {Array.<number>}
   */
  function loadExpanded() {
    try {
      return JSON.parse(localStorage.getItem(STORAGE_KEY) || '[]');
    }
    catch (e) {
      return [];
    }
  }

  /**
   * Persists the current expanded set to localStorage.
   *
   * @param {Array.<number>} nids
   */
  function saveExpanded(nids) {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(nids));
    }
    catch (e) {
      // localStorage unavailable — state simply won't persist.
    }
  }

  /**
   * Returns the nids of all ancestor <li> items of the active tree item.
   * These must remain expanded regardless of saved state.
   *
   * @return {Array.<number>}
   */
  function activeAncestorNids() {
    var nids = [];
    $('.dropdocs-tree-active').parents('li.dropdocs-has-children').each(function () {
      nids.push($(this).data('nid'));
    });
    return nids;
  }

  Backdrop.behaviors.dropdocsTree = {
    attach: function (context) {
      var $items = $('li.dropdocs-has-children', context);
      if (!$items.length) {
        return;
      }

      var expanded  = loadExpanded();
      var ancestors = activeAncestorNids();

      // Collapse all parent items by default; expand only those that are
      // active ancestors or have been manually expanded.
      $items.each(function () {
        var $li  = $(this);
        var nid  = $li.data('nid');
        var $btn = $li.find('> .dropdocs-tree-item > .dropdocs-toggle');
        if (ancestors.indexOf(nid) !== -1 || expanded.indexOf(nid) !== -1) {
          // Keep expanded.
          $btn.attr('aria-expanded', 'true');
        }
        else {
          $li.addClass('dropdocs-collapsed');
          $btn.attr('aria-expanded', 'false');
        }
      });

      // Attach click handlers (once per element).
      $items.find('> .dropdocs-tree-item > .dropdocs-toggle')
        .once('dropdocs-toggle')
        .on('click', function () {
          var $btn  = $(this);
          var $li   = $btn.closest('li.dropdocs-has-children');
          var nid   = $li.data('nid');
          var saved = loadExpanded();
          var idx   = saved.indexOf(nid);

          if ($li.hasClass('dropdocs-collapsed')) {
            $li.removeClass('dropdocs-collapsed');
            $btn.attr('aria-expanded', 'true');
            if (idx === -1) {
              saved.push(nid);
            }
          }
          else {
            $li.addClass('dropdocs-collapsed');
            $btn.attr('aria-expanded', 'false');
            if (idx !== -1) {
              saved.splice(idx, 1);
            }
          }

          saveExpanded(saved);
        });
    }
  };

})(jQuery);

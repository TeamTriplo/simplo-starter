
(function($) {

/**
 * Attach behaviors.
 */
Backdrop.behaviors.fieldUIFieldsFormsOverview = {
  attach: function (context, settings) {
    $('table#field-overview', context).once('field-field-overview', function() {
      Backdrop.fieldUIOverview.attach(this, settings.fieldUIRowsData, Backdrop.fieldUIFieldOverview);
    });
  }
};

/**
 * Row handlers for the 'Manage fields' screen.
 */
Backdrop.fieldUIFieldOverview = Backdrop.fieldUIFieldOverview || {};

Backdrop.fieldUIFieldOverview.ds = function (row, data) {

  this.row = row;
  this.name = data.name;
  this.region = data.region;
  this.tableDrag = data.tableDrag;

  this.$regionSelect = $('select.ds-field-region', row);
  this.$regionSelect.change(Backdrop.fieldUIOverview.onChange);

  return this;
};

Backdrop.fieldUIFieldOverview.ds.prototype = {

  /**
   * Returns the region corresponding to the current form values of the row.
   */
  getRegion: function () {
    return this.$regionSelect.val();
  },

  /**
   * Reacts to a row being changed regions.
   *
   * This function is called when the row is moved to a different region, as a
   * result of either :
   * - a drag-and-drop action
   * - user input in one of the form elements watched by the
   *   Backdrop.fieldUIOverview.onChange change listener.
   *
   * @param region
   *   The name of the new region for the row.
   * @return
   *   A hash object indicating which rows should be AJAX-updated as a result
   *   of the change, in the format expected by
   *   Backdrop.fieldOverview.AJAXRefreshRows().
   */
  regionChange: function (region, recurse) {

     // Replace dashes with underscores.
     region = region.replace(/-/g, '_');

     // Set the region of the select list.
     this.$regionSelect.val(region);

     // Prepare rows to be refreshed in the form.
     var refreshRows = {};
     refreshRows[this.name] = this.$regionSelect.get(0);

     // If a row is handled by field_group module, loop through the children.
     if ($(this.row).hasClass('field-group') && $.isFunction(Backdrop.fieldUIFieldOverview.group.prototype.regionChangeFields)) {
       Backdrop.fieldUIFieldOverview.group.prototype.regionChangeFields(region, this, refreshRows);
     }

     return refreshRows;
  }
};

})(jQuery);

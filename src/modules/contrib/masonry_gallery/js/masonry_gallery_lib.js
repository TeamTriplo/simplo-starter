/**
 * @file
 * Object that controls gallery layout.
 */

function MasonryGallery(newSettings) {

  var masonry = {};

  var settings = {
    containerId: null,                      // Id of the container element. Must be defined to build the gallery.
    ratio: null,                            // Array of images' width/height ratios. Must be defined to build the gallery.
    itemClass: 'masonry-gallery-item',      // CSS class of the gallery item.
    fillerClass: 'masonry-gallery-filler',  // CSS class of filler elements.
    gap: 0,                                 // Size of gap between gallery items in pixels.
    captionHeight: 48,                      // Height of the image caption in pixels.
    threshold: 32,                          // Larger value gives more weight to initial image order vs. height sorting.
  };
  var container;                            // Container element.
  var columnCountStored = 0;                // Number of columns stored from previous gallery build.

  /**
   *  Updates default settings.
   *  @param {object} newSettings - list of settings that replace default ones.
   */
  masonry.updateSettings = function(newSettings) {
    for (let i in settings) {
      if (newSettings.hasOwnProperty(i)) {
        settings[i] = newSettings[i];
      }
    }
  };

  /**
   *  Retrieves number of columns from the container's css.
   */
  masonry.getColumns = function() {
    let containerStyle = window.getComputedStyle(container, null);
    return Number(containerStyle.getPropertyValue('column-count'));
  };

  /**
   *  Builds gallery first time.
   */
  masonry.init = function() {
    container = document.getElementById(settings.containerId);
    if (!container) {
      console.error('Failed to init Masonry Gallery. Bad container.');
      return false;
    }
    if (!Array.isArray(settings.ratio)) {
      console.error('Array with images ratio is not provided.');
      return false;
    }
    masonry.update();
    return true;
  };

  /**
   *  Distributes gallery items to columns based on items height.
   *  Calculates fillers heights (filler is an empty element at the end
   *  of each column that prevents items from being placed into a
   *  neighbouring column).
   *  @param {array} items        - array of gallery items as DOM elements.
   *  @param {array} ratio        - array of images width/height ratios.
   *  @param {number} columnCount - number of columns in the gallery.
   *  @param {number} columnWidth - width of a single column excluding gap.
   */
  masonry.calculateColumns = function(items, ratio, columnCount, columnWidth) {
    // Keep items for each column here.
    var columnItems = [];
    // Keep each column height here.
    var columnHeight = [];
    // Fill arrays with initial values.
    for (let i = 0; i < columnCount; i++) {
      columnItems[i]  = [];
      columnHeight[i] = 0;
    }
    // Add items to each column. 
    // Every item goes to the column with minimal height.
    items.forEach((item, i) => {
      var minHeight = columnHeight[0]; // Take first column height to start with.
      var minColumn = 0;               // Index of the smallest column.
      columnHeight.forEach((height, c) => {
        if (height < (minHeight - settings.threshold)) {
          minHeight = height;
          minColumn = c;
        }
      });

      // Add item height to the column with minimal height.
      let itemHeight = columnWidth/ratio[i] + settings.captionHeight + settings.gap;
      columnHeight[minColumn] += itemHeight;
      columnItems[minColumn].push(item);  
    });
    // Get max column height.
    var maxHeight = Math.max.apply(null, columnHeight);
    // Calculate fillers for smaller columns.
    var filler = [];
    for (let i in columnHeight) {
      filler[i] = maxHeight - columnHeight[i]; 
    }
    //console.log(`columnItems: ${columnItems} columnHeight: ${columnHeight}`);
    return {items: columnItems, height: columnHeight, filler: filler};
  };

  /**
   *  Places items in the gallery in calculated order.
   *  @param {object} columns - object containing data returned by calculateColumns() method.
   */
  masonry.build = function(columns) {
    // Remove fillers from previous gallery build.
    var oldFillers = container.getElementsByClassName(settings.fillerClass);
    var i = oldFillers.length;
    while (i--) {
      oldFillers[i].remove();
    }

    // Place columns items and put fillers at the end of each column.
    var newItems = [];
    columns.items.forEach((column, n) => {
      newItems.push(...column);
      let filler = document.createElement('span');
      filler.setAttribute('class', settings.fillerClass);
      filler.setAttribute('style', 'height: ' + columns.filler[n] + 'px');
      newItems.push(filler);
    });
    for(let item of newItems) {
      //container.removeChild(item);
      container.appendChild(item);
    }
  };

  /**
   *  Adjusts filler height. Does not move any of the gallery elements.
   *  @param {object} columns - object containing data returned by calculateColumns() method.
   */
  masonry.refresh = function(columns) {
    var fillers = container.getElementsByClassName(settings.fillerClass);
    for (let i in columns.filler) {
      fillers[i].setAttribute('style', 'height: ' + columns.filler[i] + 'px');
    }
  }

  /**
   *  Builds and refreshes gallery. This method is supposed to be launched to first build
   *  the gallery and on window resize event.
   */
  masonry.update = function() {
    // Get number of columns and column width.
    let columnCount = masonry.getColumns();
    let columnWidth = Math.ceil((container.clientWidth - (settings.gap * (columnCount - 1))) / columnCount);

    // Sort items by data-id because their order changes after the first gallery build.
    let items = [].slice.call(container.getElementsByClassName(settings.itemClass));
    items.sort((a, b) => {
      return parseInt(a.dataset.id) - parseInt(b.dataset.id);
    });

    var columns = masonry.calculateColumns(items, settings.ratio, columnCount, columnWidth);
    // Do not rebuild gallery if column number has not changed.
    if (columnCountStored === columnCount) {
      masonry.refresh(columns);
      return;
    };
    // If number of columns changed, rebuild the gallery.
    columnCountStored = columnCount;
    masonry.build(columns);
  };

  // Gallery initialization.
  masonry.updateSettings(newSettings);
  if (masonry.init()) {
    window.addEventListener("resize", masonry.update);
    return masonry;
  }
}
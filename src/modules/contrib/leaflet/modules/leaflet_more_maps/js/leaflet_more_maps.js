(function ($) {

  Backdrop.leaflet._create_layer_orig = Backdrop.leaflet.create_layer;

  Backdrop.leaflet.create_layer = function(layer, key) {
    if (layer.type === 'quad') {
      var mapLayer = L.tileLayerQuad(layer.urlTemplate, layer.options);
      mapLayer._leaflet_id = key;
      mapLayer._type = 'quad';
      return mapLayer;
    }
    // Default to the original code;
    return Backdrop.leaflet._create_layer_orig(layer, key);
  };

})(jQuery);

L.TileLayerQuad = L.TileLayer.extend({

  getTileUrl: function(tilePoint) {

    return L.Util.template(this._url, L.extend({
      s: this._getSubdomain(tilePoint),
      q: this._xyzToQuad(tilePoint.x, tilePoint.y, this._getZoomForUrl())
    }, this.options));
  },

  /** Convert xyz tile coordinates to a single quadtree key string.
   *
   * The length of the quadkey equals the zoom level. Note: zoom > 0.
   *
   * Adapted from http://msdn.microsoft.com/en-us/library/bb259689.aspx
   */
  _xyzToQuad: function(x, y, zoom) {
    var quadKey = '', digit, mask;
    for (var z = zoom; z > 0; z--) {
      digit = 0;
      mask = 1 << (z - 1);
      if ((x & mask) !== 0) {
        digit = 1;
      }
      if ((y & mask) !== 0) {
        digit += 2;
      }
      // At this point digit equals 0, 1, 2 or 3. Append digit to quad key and
      // advance to the next zoom level to calculate the next digit.
      quadKey += digit;
    }
    return quadKey;
  }
});

L.tileLayerQuad = function(urlTemplate, options) {
  return new L.TileLayerQuad(urlTemplate, options);
};

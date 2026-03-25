(function ($) {
  'use strict';

  /**
   * Initializes Cropper.js on the crop form image.
   */
  Backdrop.behaviors.imageToolsCrop = {
    attach: function (context, settings) {
      var imageEl = document.getElementById('image-tools-crop-image');
      if (!imageEl || imageEl.dataset.cropperInitialized) {
        return;
      }
      imageEl.dataset.cropperInitialized = 'true';

      var wrapper = imageEl.closest('.image-tools-crop-wrapper');
      var infoEl  = document.getElementById('image-tools-crop-info');
      var cfg     = (Backdrop.settings.imageTools) || {};

      // Pre-size the wrapper to the image's exact display dimensions
      // before Cropper initializes, so Cropper has no reason to add padding.
      if (cfg.naturalWidth && cfg.naturalHeight && wrapper) {
        var maxW   = wrapper.offsetWidth || 600;
        var maxH   = Math.round(window.innerHeight * 0.75);
        var scale  = Math.min(maxW / cfg.naturalWidth, maxH / cfg.naturalHeight, 1);
        var dispW  = Math.round(cfg.naturalWidth  * scale);
        var dispH  = Math.round(cfg.naturalHeight * scale);
        wrapper.style.width  = dispW + 'px';
        wrapper.style.height = dispH + 'px';
      }

      var cropper = new Cropper(imageEl, {
        // viewMode 3: canvas fills container — no gray space around image.
        viewMode: 3,
        autoCropArea: 1,
        movable: false,
        zoomable: false,
        crop: function (event) {
          var d = event.detail;
          var x = Math.round(d.x);
          var y = Math.round(d.y);
          var w = Math.round(d.width);
          var h = Math.round(d.height);

          _setVal('crop_x', x);
          _setVal('crop_y', y);
          _setVal('crop_width', w);
          _setVal('crop_height', h);

          if (infoEl) {
            infoEl.textContent = w + ' \u00d7 ' + h + ' px  (at ' + x + ', ' + y + ')';
          }
        }
      });

      // Aspect ratio lock toggle.
      var lockEl = document.querySelector('[name="lock_aspect"]');
      if (lockEl) {
        lockEl.addEventListener('change', function () {
          if (this.checked) {
            var d     = cropper.getData();
            var ratio = (d.width && d.height) ? d.width / d.height : NaN;
            cropper.setAspectRatio(ratio);
          } else {
            cropper.setAspectRatio(NaN);
          }
        });
      }
    }
  };

  function _setVal(name, value) {
    var el = document.querySelector('[name="' + name + '"]');
    if (el) {
      el.value = value;
    }
  }

}(jQuery));

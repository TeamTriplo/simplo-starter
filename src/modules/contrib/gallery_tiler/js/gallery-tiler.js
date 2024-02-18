(function ($) {
  Backdrop.behaviors.galleryTilerColorbox = {
    attach: function (context, settings) {
      if (settings.gallery_tiler.lightboxType == 'colorbox') {
        $('.gallery-tiler .field-items a').colorbox({'rel': 'tiled-gallery'});
      }
    }
  }
})(jQuery);

/**
 * @file
 * Sets options and initiates jquery plugins.
 */
(function($) {

  Backdrop.behaviors.masonry_gallery = {
    attach: function (context, settings) {
      for (let key in settings.masonry_gallery) {
        if (key == 'lazyload') {
          // Set up a function that removes loader as soon as image is loaded.
          var onLoad = null;
          if (settings.masonry_gallery[key].loader) {
            var onLoad = function(el) {
              var galleryItem = el.closest('.masonry-gallery-item');
              var loaderEl = galleryItem.getElementsByClassName('masonry-gallery-loader')[0];
              if (loaderEl) {
                loaderEl.remove();
              }
            }
          }
          // Setup LazyLoad plugin.
          var lazyLoad = new LazyLoad({
            threshold: settings.masonry_gallery[key].threshold,
            callback_loaded: onLoad,
          });
        }
        else {
          // Initiate masonry layout script.
          const masonry = MasonryGallery({
            containerId: key,
            ratio: settings.masonry_gallery[key].ratio,
            gap: Number(settings.masonry_gallery[key].gap),
            captionHeight: Number(settings.masonry_gallery[key].caption_height),
            threshold: Number(settings.masonry_gallery[key].threshold),
          });
        }
      }
    }
  };

})(jQuery);

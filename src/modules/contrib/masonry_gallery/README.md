Masonry Gallery
===============

Masonry Gallery is a Views style plugin that allows to display images layed out in traditional masonry grid. The module distributes images in columns by their height so that they fill available space as evenly as possible with minimum jags at the bottom of the gallery. At the same time it tries to keep left to right visible order of the images. Here is a short list of features:

- Responsive masonry grid layout for images.
- Optimised image loading (lazy loading).
- Captions that either placed statically below the image or on mouse hover over the image bottom.
- Ability to tweak caption style without writing extra CSS.
- Ability to use rich text for captions (by adding text fields in views).
- Several loader icon styles to choose from.
- Ability to show a low resolution preview while image is being loaded.

Technically this gallery utilizes CSS for basic columns layout and Javascript to control the order of images.

Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules

- Visit the configuration page under Administration > Configuration > Media >
  Masonry Gallery (admin/config/media/masonry_gallery) to configure the global
  settings.

- Enable and configure the Masonry Gallery in a view's format section. This plugin requires
  one image field and may optionally use one text field as a source for captions.


Issues
------

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/masonry_gallery/issues

Known Issues
------------

Some lightbox modules may display images in a wrong sequence following columns direction instead of intended left to right order.

Current Maintainers
-------------------

- [Pavel](https://github.com/korontari)
- Co-maintainers are welcome.

Credits
-------

- Written for Backdrop by [Pavel](https://github.com/korontari).
- [LazyLoad](https://github.com/verlok/vanilla-lazyload) plugin(MIT license) by [Andrea Verlicchi](https://github.com/verlok).

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.


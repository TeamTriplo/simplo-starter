Testimonial Recipe
======================

This module is a simple config recipe developed to help illustrated what can
be done with a module that only contains config files and CSS. 

This particular module creates a Testimonial content type with fields for 
- author
- image
- body (text of testimonial)

It also comes with
- a custom image style
- a view page and view block that display a 3 column grid of testimonials
- a CSS file with stying for testimonials


Requirements
------------

Requires [BackdropCMS 1.20](https://github.com/backdrop/backdrop/releases/tag/1.20.0) or greater.

Installation
------------

- This recipe can be found in the Recipes package on the modules 
  page (admin/modules/list).

- Install this like any other module using the official Backdrop CMS 
  instructions at https://backdropcms.org/guide/modules.

- Disabling and uninstalling this module will not delete any of the 
  configuration that this module provides, but will disable the CSS
  files that came with this module. 

- Either add the testimonial page to your menu OR place the views 
  testimonial block on any page. 

Instructions
------------

After enabling the module, just create a couple of 

- Go to `Content > Add Content > Testimonial` or `/node/add/testimonial`
- Create several testimonials with or without images
- View results at `/testimonials`

Issues
------

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/testimonial-recipe/issues.

Please, use the issue queue to tell us how the recipe worked for you and
to share your ideas on how to improve it for other users. 

Feedback
--------

We are experimenting with config recipes and welcome your feedback. Please,
let us know how this config recipe worked for you and how you think we 
could improve it for other users in the future. 
https://github.com/backdrop-contrib/testimonials/issues/1

Current Maintainers
-------------------

- [Tim Erickson](https://github.com/stpaultim).

Credits
-------

- Sponsored by [Simplo](https://www.simplo.site)

License
-------

This project is GPL v2 software. 
See the LICENSE.txt file in this directory for complete text.

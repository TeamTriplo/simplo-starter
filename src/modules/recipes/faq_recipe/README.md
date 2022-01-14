FAQ Recipe
======================

This module is a simple config recipe developed to help illustrated what can
be done with a module that only contains config files and CSS. 

We have also added an install file to add some sample content. 

This recipe creates

 - A FAQ content type
 - A view using Bootstrap Accordion module to display FAQs. 
   Bootstrap Accordion module must be enabled and configured to 
   "Include Bootstrap CSS & JS when used by Views"
 - A menu item in main menu
 - Four FAQ items (nodes).

 While some users may be happy with the configuration of this FAQ feature 
 exactly as defined in the recipe. We expect that most users will use 
 this recipe as a starting point and customize it to meet their unique needs. 


Requirements
------------

Requires [BackdropCMS 1.20](https://github.com/backdrop/backdrop/releases/tag/1.20.0) or greater.

This recipes is currently configured to use the Bootstrap Accordion module 
to display the FAQ. You will need to enable and configure the Bootstrap 
Accordion module to benefit from that formatting. 

Installation
------------

- This recipe can be found in the Recipes package on the modules 
  page (admin/modules/list).

- Install this like any other module using the official Backdrop CMS 
  instructions at https://backdropcms.org/guide/modules.

- Disabling and uninstalling this module will not delete any of the 
  configuration that this module provides, but will disable any CSS
  files that came with the recipe.

Uninstall or Upgrate Options
----------------------------

It is not currently possible to uninstall or upgrade this recipe.
If you no longer wish to keep this functionality, you will need 
to remove the items added by the recipe manually.


Issues
------

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/faq/issues.

Feedback
--------

We are experimenting with config recipes and welcome your feedback. Please,
let us know how this config recipe worked for you and how you think we 
could improve it for other users in the future. 
https://github.com/backdrop-contrib/faq/issues/1

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



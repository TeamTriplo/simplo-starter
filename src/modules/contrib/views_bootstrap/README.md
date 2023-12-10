Views Bootstrap
===============

This module provides a set of styles for the Views module. It allows you to
create responsive Bootstrap components, such as grids, carousels, tabs, and
tables, all within the configuration settings of the powerful Views module.

This module uses the Bootstrap 3.x library. If your theme uses Bootstrap 4 or
later, you may experience some incompatibilities.

If you are using this module, [please let us know](https://github.com/backdrop-contrib/views_bootstrap/issues/27).
It motivates us to work on the module when we hear from actual users.

Installation
------------

1) Install this module using [the official Backdrop CMS instructions](https://backdropcms.org/guide/modules).

2) Set the config options at `admin/config/user-interface/views_bootstrap`. **Important**: If you _are not_ using a Bootstrap theme, you must check "Include Bootstrap CSS & JS when used by Views" on the configuration page. If you _are_ using a Bootstrap theme such as [Bootstrap Lite](https://backdropcms.org/project/bootstrap_lite), leave that setting unchecked. Checking it with a Bootstrap theme can result in two copies of the Bootstrap library being loaded, which can give rise to undesired and puzzling behavior.

Components
----------

The following components of this module have been tested to have a working Views configuration:

- Accordion
- Carousel
- Dropdown
- Grid
- List Group
- Media Object
- Panels
- Tabs
- Table
- Thumbnails

Usage
-----

If you use a Bootstrap 3-based theme, this module will work "out of the box"; enable it and start creating your Views. If you do not use a Bootstrap 3-based theme, then you can configure the Views Bootstrap module to load the Bootstrap library from a CDN at `/admin/config/user-interface/views_bootstrap`. You can choose from several different versions. In general, the latest (3.4.1) is a good choice, but older versions are supported for compatibility reasons.

To use, when you create your View and select its Format, where you would once choose from "List", "Table", and so forth, now you can also choose "Bootstrap Carousel, Bootstrap Grid," and many more options.

Knowledge of how to use different Views formats is required. Note that you must configure each View appropriately with fields to get the desired behavior. For example, making an accordion of one field does not work—these require a clickable title and hidden body content.  Making a table of one content teaser does not work—tables consist of fields in rows.

You may have to write some CSS to fine-tune the appearance.

JavaScript is required for some components to work properly, and accessibility is not guaranteed. Internet Explorer and older browsers are not supported.

To become familiar with what this module provides, see the Bootstrap 3 documentation:

* [Bootstrap 3.4 Documentation](https://getbootstrap.com/docs/3.4/)

To see examples of all of the displays, enable the Views Bootstrap Demo sub-module that is part of this package.

### What is the difference between using a Bootstrap-based theme for your website and using this module?

Let's take the use-case of making a "Pinterest-style", responsive card style listing of content on your front page.  On large screens, you want it to be 4 columns wide with a sidebar on the right side.  On smaller screens, it should be 2 columns wide with a sidebar on the right side.  On a small screen, it should be 1 column wide with the sidebar underneath.  Using something like Bartik theme, you can install this module, select it, and it will happen, but the rest of the page will look like Bartik (or your theme).  If you were using something like Radix theme, the theme would look like Bootstrap but you would have to manually add the grid classes to your "cards" in your theme files to make the same effect.

Please [open an issue](https://github.com/backdrop-contrib/views_bootstrap/issues) if you find anything that is broken or incomplete.

Upgrading From Drupal
---------------------

This module is very similar in functionality to [its Drupal 7 counterpart](https://drupal.org/project/views_bootstrap), but there are some changes to reflect differences between Backdrop and Drupal (and there is some added functionality in the Backdrop version). When you upgrade your website from Drupal 7, any Views that made use of the Drupal 7 version of Views Bootstrap will be converted to their Backdrop equivalents.

Known Issues
------------

[See the issue queue for known issues.](https://github.com/backdrop-contrib/views_bootstrap/issues)

Special Thanks
--------------

Sponsored by WikiJob <https://www.drupal.org/wikijob>.

Maintainers
-----------

- [Tim Erickson (stpaultim)](https://github.com/stpaultim)
- [Robert J. Lang (bugfolder)](https://github.com/bugfolder)

Credits
-----------

- Initial development by [Dmitry Demenchuk](http://www.demenchuk.me/)
- Support for this module provided by [Simplo.site](https://www.simplo.site)
- Initial port by [biolithic](https://github.com/biolithic)
- Original [Drupal 7 module](https://www.drupal.org/project/views_bootstrap)

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

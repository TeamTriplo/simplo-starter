Views Bootstrap Demo
======================

This module provides a set of demonstration pages for the Views Bootstrap module.

It creates some initial content nodes and sets up a sample View with displays for all supported Views Bootstrap handlers. You can examine how the Views are configured to see how to use each of the individual handlers.

The bare minimum of CSS is provided for some of the pages, but it is assumed that you will be doing your own CSS styling for fine-tuning.

Installation
------------

Install this module using [the official Backdrop CMS instructions](https://backdropcms.org/guide/modules).

Usage
-----

When you enable this module, it will create a new content type, "Views Bootstrap Demo", and a View that contains sample pages of all the types of Views.

It also creates six new sample nodes of type "Views Bootstrap Demo" that will be used to populate the Views.

(Note: these nodes, and the content type, will be removed when you uninstall the module.)

These are the paths for the View pages that demonstrate the Views Bootstrap settings with links to the corresponding Bootstrap documentation pages.

* **Accordion** — `/views-bootstrap-demo-accordion` [[Bootstrap docs]](https://getbootstrap.com/docs/3.4/javascript/#collapse-example-accordion)
* **Carousel** — `/views-bootstrap-demo-carousel` [[Bootstrap docs]](https://getbootstrap.com/docs/3.4/javascript/#carousel)
* **Dropdown** — `/views-bootstrap-demo-dropdown` [[Bootstrap docs]](https://getbootstrap.com/docs/3.4/javascript/#dropdowns)
* **Grid** — `/views-bootstrap-demo-grid` [[Bootstrap docs]](https://getbootstrap.com/docs/3.4/css/#grid)
* **List Group** — `/views-bootstrap-demo-list-group` [[Bootstrap docs]](https://getbootstrap.com/docs/3.3/components/#list-group)
* **Media** — `/views-bootstrap-demo-media` [[Bootstrap docs]](https://getbootstrap.com/docs/3.3/components/#media)
* **Panels** — `/views-bootstrap-demo-panels` [[Bootstrap docs]](https://getbootstrap.com/docs/3.3/components/#panels)
* **Table** — `/views-bootstrap-demo-table` [[Bootstrap docs]](https://getbootstrap.com/docs/3.4/css/#tables)
* **Tabs** — `/views-bootstrap-demo-tabs` [[Bootstrap docs]](https://getbootstrap.com/docs/3.4/javascript/#tabs)
* **Thumbnails** — `/views-bootstrap-demo-thumbnails` [[Bootstrap docs]](https://getbootstrap.com/docs/3.3/components/#thumbnails)

For best results, set the default layout template to Boxton. Be sure you configure Views Bootstrap at `/admin/config/user-interface/views_bootstrap` to ensure that either your theme or Views Bootstrap (but not both) loads the Bootstrap libraries.

Known Issues
------------

[See the issue queue for known issues.](https://github.com/backdrop-contrib/views_bootstrap/issues)

Maintainers
-----------

- [Tim Erickson (stpaultim)](https://github.com/stpaultim)
- [Robert J. Lang (bugfolder)](https://github.com/bugfolder)

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

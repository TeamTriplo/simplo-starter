# insert-view
- Initial port of drupal.org/project/insert_view to Backdrop.

Insert view filter allows to embed views using tags. The tag syntax is
relatively simple: [view:name=display=args]. The parameters are: view name, view
display id, view arguments. For example [view:tracker=page=1] says, embed a view
named "tracker", use the "page" display, and supply the argument "1". The
display and args parameters can be omitted. If the display is left empty, the
view's default display is used. Multiple arguments are separated with slash. The
args format is the same as used in the URL (or view preview screen).

Valid examples:
[view:my_view]
[view:my_view=my_display]
[view:my_view=my_display=arg1/arg2/arg3]
[view:my_view==arg1/arg2/arg3]

Installation
------------
- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules

- Once Insert view is installed, visit the the input formats page at
/admin/config/content/formats and click the "configure" link for the input format(s)
for which you wish to enable the Insert view filter. Then simply check the
checkbox for the filter.

How to find the Display ID
------------------------
On the edit page for the view in question, you'll find a list of displays at the
left side of the control area. "Defaults" will be the first one of that list. Hover
your mouse pointer over the name of the display you want to use. A URL will
appear in the status bar of your browser.  This is usually at the bottom of the
window, in the chrome. After /views/view you see name_of_the_view/edit/name_of_the_displayID. For
example in http://xxx.myhomepage.com/admin/structure/views/view/frontpage/edit/page
the view name is "frontpage" and the display ID would be "page".

SECURITY WARNING
----------------
This filter is powerful, therefore it should be granted to trusted users only.
If you allow this filter to untrusted users, then you have to make sure that
EVERY VIEW EVERY DISPLAY has correct views access
settings.

Performance
-----------
To display views correctly, Insert view turns off caching for the input formats
for which it is enabled. That means every node using this input format will not
be cacheable. This can impact site performance. In these cases, it is
recommended to create a special input format for use when inserting views.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Current Maintainers
-------------------
- Joerg Kienitz (https://github.com/vtad)
Maintenance status: Minimally maintained

Credits
-------
This module was originally written for Drupal by Michael Samuelson
(https://www.drupal.org/u/mlsamuelson)

Switchtheme
===========

Switchtheme allows you to create a block to allow users to switch themes on the
fly.  The module presents a list of all enabled themes and allows the user
to choose among them. Anonymous users have their choices tracked in a
session variable.  For logged in users, the user record is updated with their
choice so that their last selection will stay the same the next time they log in. 

For a full description visit the Drupal project page:
  http://drupal.org/project/switchtheme
Bug reports, feature suggestions and latest developments:
  http://drupal.org/project/issues/switchtheme

Installation
------------
- Install this module using the official Backdrop CMS instructions at
https://backdropcms.org/guide/modules

Configuration
-------------

* Configure who should be allowed to use the Switchtheme blocks in
  Administration » People » Permissions » Switchtheme.

  To permanently store the chosen theme for registered users, grant the
  "Permanently use a custom theme" permission to selected user roles.

* Customize the settings in Administration » Configuration » User interface »
  Switchtheme.

  The theme names may not be very meaningful to regular users, so the settings
  page allows you to create custom labels to show for each theme.

* Go to Administration » Structure » Blocks and enable the desired Switchtheme
  block(s), if any. Make sure that it is enabled in all enabled themes.


Usage
-----

You can use this module to present users with a small, medium, and large text
version of your site.  To do that:

* Select the theme that you want to use for this purpose.  Create a subfolder
  below the theme folder for every subtheme that you want to create (i.e.
  medium, large, and extra-large).  The folder names will be used as the theme
  names, so make sure they don't duplicate any existing themes.  One way to do
  that is to use the regular theme name as a prefix, then append something to
  the name (i.e. bluemarine_large).

* Copy the style.css and any theme images from the main folder to each of these
  sub-folders.

* Change style.css in each subfolder to display appropriately for the chosen
  font size.  Well-designed CSS may only need to be changed in a few places.
  For instance, the Bluemarine theme sets a basic font size at the very top of
  the CSS to 76%.  Make that percentage higher and almost everything in the
  theme will be displayed in larger text.  (You may need to change a few other
  CSS settings to get it working completely.)

  Any style.css files and images in the subfolders will override those in the
  main folder, but all folders will use the template.php and tpl.php files in
  the main theme folder, so you don't need to duplicate all the template files,
  only the css and image files.

* Go to the settings page and give the themes user-friendly names like
  'Regular', 'Medium', and 'Large', and turn the blocks on as noted above.

Now your users will have a drop-down selection list they can use to increase
the font size of their pages.


Customization
-------------

* When Switchtheme module is enabled, users are able to switch to a different
  theme any time they follow a link that includes a query string like in this
  URL: http://www.example.com/foo/bar?theme=exampletheme

  That means, you can implement specially crafted links into your site or themes
  which allow users to switch to pre-defined themes.  For example, using
  Drupal's l() function in page.tpl.php (without code tags):
<code>
print l('Red theme', $_GET['q'], array('query' => array('theme' => 'red'))); 
</code>

License
-------
This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Current Maintainers
-------------------
- Joseph Flatt [hosef](https://github.com/hosef)

Credits
-------
Ported to Backdrop by Joseph Flatt [hosef](https://github.com/hosef).
Theme display as unordered list added by Luke McCormick [cellear](https://github.com/cellear)

Originally developed and maintained for Drupal by:
* Daniel F. Kudwien (sun) - http://drupal.org/user/54136
* Karen Stevenson (KarenS) - http://drupal.org/user/45874

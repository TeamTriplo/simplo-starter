Masquerade
============================

Description
----------------------------

The Masquerade module allows users to temporarily switch to another user
account. It keeps a record of the original user account, so users can easily
switch back to the previous account.

It adds a link on a user\'s profile page that allows permitted users to
masquerade as that user. Upon masquerading, a link to "switch back" to the
original user will appear in the menu. While masquerading, the option to 
masquerade as another user will not appear. All masquerading transactions 
are logged, and `$user->masquerading` will be set; this could be displayed
via theme.

In the masquerade settings a list of roles are presented; any checked role
is considered an "administrator" and requires the second level "masquerade as admin"
permission to masquerade as. User #1 is automatically considered an administrator,
regardless of roles.

Installation
----------------------------

To install the Masquerade module, extract the module to your modules folder.
After enabling the module, it can be configured under 
Configuration > User accounts > Masquerade. To enable users to
masquerade, assign the appropriate "masquerade module" permissions to the roles
available on your site. For example:

* To allow members of the 'customer support' role to masquerade as any
  non-admin user, add the 'masquerade as user' permission to the role. In the
  Masquerade configuration, set 'administrator' as an administrator role
  to prevent customer support users from masquerading as those users.

* To allow members of the 'tech support' role to masquerade as 'administrator',
  add the 'masquerade as admin' permission to the role. Then,
  in the Masquerade configuration, set 'administrator' as an
  administrator role.

	 
Usage / Quick Switch Menu
----------------------------

By default, when a user is selected for the 'Menu Quick Switch user', the Masquerade module adds two menu items to the 'Navigation' menu:

 * Masquerade as 'the user selected': When clicked, the user can quick switch to the user selected.
 * Switch back: This menu item appears while masquerading so that you can switch back to your original user.
 
License
----------------------------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.


Current Maintainers
----------------------------

- BSCOOL (JugglingCoder) (https://github.com/JugglingCoder/, http://www.thedreamchest.com/)
- Herb v/d Dool (https://github.com/herbdool).


Credits
----------------------------

- Ported to Backdrop CMS by BSCOOL (JugglingCoder) (https://github.com/JugglingCoder, http://www.thedreamchest.com/)

This module was developed for Drupal by a number of contributors. For more information
about this module, see:

Project Page: http://drupal.org/project/masquerade
Issue Queue: http://drupal.org/project/issues/masquerade

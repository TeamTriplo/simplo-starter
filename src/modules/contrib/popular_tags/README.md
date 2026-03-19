Popular Tags
============

Popular Tags allows you to choose taxonomy terms (tags) by clicking on them,
rather than typing them (autocomplete) or selecting them (select box,
checkboxes, radios, etc.). It shows them as a list of tags, sorted by popularity
(based on how many nodes are tagged with them), up to a configurable maximum
number. Also, when you hover over a listed tag, it shows how many times it has
been used.

![Popular Tags demonstration](.assets/demo.gif)

This helps you to see what the most commonly-used tags are, rather than having
to guess. It also avoids the creation of numerous, closely-related and similar
tags, because you can see if such a tag already exists. The fact that tags are
only a click away encourages you to coalesce around a tighter vocabulary than
you would otherwise have.

Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules.

- Visit the term-reference field configuration page in your content type
  (Administration > Structure > Content Types > [content type] > Manage Fields >
  [field]) and, under the 'Popular Tags' fieldset, check the 'Enable popular
  tags' checkbox.

- Create or edit a node of that content type and you'll see a list of clickable
  popular tags under the tag field (provided that there are existing tags).

Issues
------

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/popular_tags/issues.

Current Maintainers
-------------------

- Peter Anderson (https://github.com/BWPanda).

Credits
-------

- Ported to Backdrop CMS by Peter Anderson (https://github.com/BWPanda).
- Originally written for Drupal by Jitesh Doshi
  (https://www.drupal.org/u/jitesh-doshi).

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.


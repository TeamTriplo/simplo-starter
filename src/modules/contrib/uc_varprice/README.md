#  UC Variable Price

The UC Variable Price product feature allows you to turn any product in your Ubercart store into a variable priced product. Instead of using a fixed price entered by an administrator, customers can specify their own price for a product on the add to cart form. This makes it a useful module for donation sites! The module will automatically take care of the product add to cart shopping cart forms to accommodate the variable price field. It affords you some measure of control over the titles of various fields and also lets you specify a minimum and maximum price for any given product. Extra integration with Ubercart's product classes interface allows you to set default Variable Price product features on every product created of any given product class.

## Documentation

You may add the Variable Price product feature to any product on your site through the Features tab on its Edit form. If you're entering multiple variable price products on the same site, you can create a new product class with a default Variable Price product feature on it through the normal classes interface. Finally, this module adds a unique ID to variable priced products as they're added to the cart, so their quantity will always be 1. The cart form is altered to show a text-representation of this in the Qty. column, but you can change what appears here by overriding the theme function theme_varprice_qty().

## Notes

We ran this through Coder Upgrade and it worked for our use case. 

Please, be cautious and report issues in the [issue queue](https://github.com/backdrop-contrib/uc_varprice/issues).

## Drupal 7 Version

- https://www.drupal.org/project/uc_varprice

## Installation

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules.

## Maintainers for Backdrop

 - [Tim Erickson](https://github.com/stpaultim)
 - Help wanted (open a PR and ping me)

## Credits

 - Port sponsored by: Simplo (https://simplo.site) by Triplo (https://triplo.co)
 - Drupal 7 version maintained by: [jrust](https://www.drupal.org/user/124030), [rszrama](https://www.drupal.org/u/rszrama)
 - Supported development of the original D7 module - [Churches Group](http://groups.drupal.org/churches) following [these specs](https://groups.drupal.org/node/19981). 

## LICENSE

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

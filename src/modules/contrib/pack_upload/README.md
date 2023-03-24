# Pack and Upload Module

There are certain situations where a user would require a bulk of files to be uploaded at server. You can take an example of feed import activity where a user uploads node using feed importer. There could be an image or file field which requires files to be uploaded however there is no simple way to move all files via FTP first and then refer the path of uploaded files in your CSV or excel files for a specific node to be created.

By using Pack & Upload this process can be very easy just by following the steps:

1. Create a tar.gz or zip of files you want to upload.
2. Change pack & upload settings and specify a directory where you want to move all uploaded files.
3. Now go to uploader form, provide path to your pack (zip or tar.gz) and upload.
4. Once uploading is completed it will extract all files in the specified directory.

Note: Make sure you have the right permissions to the directory where you want to upload.

## Requirements

* This module has no requirements yet.

## Installation

* Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules.

* Visit the configuration page under Administration > Configuration > Pack & Upload 
  (admin/config/pack-upload) and enter the required information.

## Usage
   
 * Install pack_upload module from admin panel.
 * Goto url admin/config/pack-upload.
 * Provide the pack_upload directory for extraction.
 * Click on save settings.
 * Create a zip or tar of files you want to upload.
 * Use importer on pack & upload settings page.
 * Provide the path of packed file and upload.
 * After completion of upload your pack will be extracted to specified directory.

## General notes

 * Currently path can be provide as stream wrapper as default path is public://bulk_media.
 * Paths can be set dynamically using tokens. 
 
 ## Issues

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/pack_upload/issues.

## Current Maintainers

- [Tim Erickson](https://github.com/stpaultim)
- Co-maintainers welcome

## Credits

* Ported to Backdrop by [djzwerg](https://github.com/djzwerg)
* Originally developed for Drupal by [diveshkumar](https://github.com/diveshkumar)
* Token support added by [ram4nd](https://www.drupal.org/u/ram4nd)

## License

This project is GNU GPL v2 software. See the LICENSE.txt file in this directory for complete text.

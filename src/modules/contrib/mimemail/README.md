# Mime Mail

This Mime Mail module provides functions that can be used by other 
modules to accept an HTML message body and header, mime-endcode it 
and send it via Backdrop's core mail system. 
It also permits users to receive HTML email. 

If the HTML has embedded graphics, these graphics are MIME-encoded 
and included as a message attachment. It will also send uploaded 
files as attachments.

Sent messages include your theme's CSS stylesheet files in HTNL messages. When 
used in conjunction with the module Mail System, Mime Mail will use the 
designated theme's CSS stylesheets as configured by Mail System. 
Styles can be converted into inline style attributes with the help of the 
submodule Mime Mail CSS Compressor.

Site users can choose to receive plaintext emails through a setting in their
account profile configuration. The HTML will be converted to plain text and 
sent as-is. Otherwise, the email will be sent in HTML with a plaintext alternative.

This module provides two custom Rules actions:
- Send HTML email
- Send HTML mail to all users of a role

## Installation

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules.

- Use the configuration page at /admin/config/system/mimemail to
  set values for:
  + sender name
  + sender email address
  + use simple address format
  + include site style sheets
  + send plain text email only 
  + link images only
	+ preserve class attributes
	+ text format (HTML or plain text) 
	
## Supplementary modules

There are three submodules:

- Mime Mail Example is a simple form for input of an
  email to send via Mime Mail.
	
- Mime Mail CSS Compressor converts CSS to inline styles in
  an HTML message. (Requires the PHP DOM extension.)
	
- Mime Mail Action provides additional actions for Mime Mail.
  (Not yet tested with Backdrop.)
	
## Help & Documentation

This module may be required by other modules, but in light of the system actions
and Rules integration, it can be useful by itself too.

Once installed, any module can send MIME-encoded messages by specifing
MimeMailSystem as the responsible mail system for a particular message
or all mail sent by one module.

This can be done through the web by **installing the module [Mail System](https://backdropcms.org/project/mailsystem)**, and 
visiting `admin/config/system/mailsystem` or programmatically as follows:

```php
mailsystem_set(array(
  '{$module}_{$key}' => 'MimeMailSystem', // Just messages with $key sent by $module.
  '{$module}' => 'MimeMailSystem', // All messages sent by $module.
));
```

You can send email with `backdrop_mail()` and specify the following optional parameters 
(5th argument) to build the email and send it:
- 'plain':
    Boolean, whether to send messages in plaintext-only (optional, default is FALSE).
- 'plaintext':
    Plaintext portion of a multipart email (optional).
- 'attachments':
    Array of arrays with the path or content, name and MIME type of the file (optional).
- 'headers':
    A keyed array with headers (optional).

You can set these in $params either before calling `backdrop_mail()` or in 
hook_mail() and of course hook_mail_alter().

Normally, Mime Mail uses email addresses in the form of `"name" <address@host.com>`,
but PHP running on Windows servers requires extra SMTP handling to use this format.
If you are running your site on a Windows server and don't have an SMTP solution such
as the SMTP module installed, you may need to set the 'Use the simple format of
user@example.com for all email addresses' option on the configuration settings page.

This module creates a user preference for receiving plaintext-only messages.
This preference will be honored by all messages if the format is not explicitly set
and the user has access to edit this preference (allowed by default).

Email messages are formatted using the mimemail-message.tpl.php template.
This includes a CSS style sheet and uses an HTML version of the text.
The included CSS is either:
  - The `mail.css` file found anywhere in your theme folder or
  - The combined CSS style sheets of your theme if enabled.

If you have installed the module Mail System, you can spedify which theme's CSS
to use in "Theme to render rich text emails" at `admin/config/system/mailsystem`. 

Since some email clients (namely Outlook 2007 and GMail) tend to only regard
inline CSS, you can use the Mime Mail CSS Compressor submodule to convert CSS
styles into inline style attributes. It transmogrifies the HTML source by parsing
the CSS and inserting the CSS definitions into tags within the HTML based on the
CSS selectors. To use the Compressor, just enable it.

To create a custom mail template copy the mimemail-message.tpl.php file from
the mimemail/theme directory into your default theme's folder. Both general and
by-mailkey theming can be performed:
- `mimemail-message--[module]--[key].tpl.php` (for messages with a specific module and key)
- `mimemail-message--[module].tpl.php` (for messages with a specific module)
- `mimemail-message--[key].tpl.php` (for messages with a specific key)
- `mimemail-message.tpl.php` (for all messages)

NOTE: if you place any of those suggested template files in your theme's `templates` folder, 
Mail System will likely ignore the setting "Theme to render rich text emails". Instead, 
the CSS stylesheets for the current page path theme will be used. This seems to be a bug in Mail System.

When the module Mail System is installed, messages can be rendered using 
different themes. You can choose the following settings to render the email:
- 'current': Theme currently used by the user who runs backdrop_mail().
- 'default': Default theme, obtained via variable theme_default.
- 'domain': Theme obtained via Domain Theme module.
- Or any other active theme.

Images with absolute URL will be available as remote content. To embed images
into emails you have to use a relative URL or an internal path. Due to security
concerns, only files residing in the public file system (e.g sites/default/files)
can be used by default.

For example:
- instead of `http://www.mysite.com/sites/default/files/mypicture.jpg`
- use `/home/www/public_html/backdrop/sites/default/files/mypicture.jpg`
- or `/sites/default/files/mypicture.jpg`
- or `public://mypicture.jpg`

The 'send arbitrary files' permission allows you to attach or embed files located
outside Backdrop's public files directory. Note that this has security implications:
arbitrary means even your settings.php! Give to trusted roles only!

## License

This project is GPL v2 software. See the LICENSE.txt 
file in this directory for complete text.
    
## Current Maintainers for Backdrop

+ Graham Oliver (github.com/Graham-72/)
+ [argiepiano](https://github.com/argiepiano)

## Credits

### Port to Backdrop

+ Graham Oliver (github.com/Graham-72)
+ Andy Martha (github.com/biolithic)

### Maintainers for Drupal:

+ Robert Castelo <https://www.drupal.org/user/3555>
+ Gerhard Killesreiter <https://www.drupal.org/user/227>
+ Allie Micka <https://www.drupal.org/user/15091>
+ Gabor Seljan <https://www.drupal.org/user/232117>

### Acknowledgement

This port to Backdrop would not, of course, be possible without all
the work done by the developers and maintainers of the Drupal module.

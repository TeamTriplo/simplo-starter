Dashboard Plus
==============

This module provides two things:

  * The option to make all dashboard blocks collapsible (on by default).

![image](https://github.com/user-attachments/assets/9a559f1f-277a-451e-82cd-838830bc3aaf)

  * A set of additional blocks that can be placed on the Dashboard provided by Backdrop CMS.

At this point, we are looking for good ideas for blocks to include in this module, please use the issue queue to submit requests. 

https://github.com/backdrop-contrib/dashboard_plus/issues

## This module currently contains the following blocks:

 * Dashboard Plus - Manage content
   - *Contains links to add or manage existing content for each content type. Meant to replace the existing Dashboard - Create content block.*

![image](https://github.com/user-attachments/assets/d4b038af-b1e6-44f2-94b6-f31ae516ec86)

 * Dashboard Plus - Recent unpublished comments
   - *Contains a block with a list of recent comments that are not published a link to "moderate" them.*

![image](https://github.com/user-attachments/assets/b9521fa4-ea38-4675-b501-f9dfc127d695)

 * Dashboard Plus - Recent log messages
   - *Contains a block with 10/25/50/100 recent log messages along with a link to the log page.*

![image](https://github.com/user-attachments/assets/e5a9cb50-cc6c-45fd-8fa3-ac7149275a8f)

 * Dashboard Plus - Backdrop release notes
   - *Contains option to view release notes for either the latest minor release of Backdrop CMS or the latest bugfix release.*

![image](https://github.com/user-attachments/assets/cb098a2f-f2f5-4b95-aaf0-4e7da86bf77e)

 * Dashboard Plus - Resources
   - *Contains hardcoded links to some important and useful Backdrop CMS resources.*

![image](https://github.com/user-attachments/assets/663234db-08cc-4e30-80a4-ab20b3572459)

 * Dashboard Plus - Cron status
   - *Shows when cron last ran (with a warning or error indicator if overdue) and provides a link to run it manually.*

 * Dashboard Plus - Current weather
   - *Shows current weather conditions for a configured location. Displays temperature, feels-like temperature, humidity, and wind speed using WMO weather codes. Configurable temperature units (°C/°F) and an optional link to an external forecast page. Powered by [Open-Meteo](https://open-meteo.com) — no API key required.*

 * Dashboard Plus - Module of the day
   - *Displays a randomly selected Backdrop CMS module each day, pulled live from the BackdropCMS.org project directory. Shows the module title (linked to its project page), a truncated description with a "Read more" toggle, and install/maintenance stats. Includes a "See another" button to browse additional modules without leaving the dashboard.*

 * Dashboard Plus - New module releases
   - *Lists the most recently published modules on BackdropCMS.org (first releases only, not updates). Pulls from the BackdropCMS.org RSS feed and displays modules sorted by release date, newest first. Configurable to show 5, 10, 15, or 20 entries. Includes a link to browse all releases on BackdropCMS.org.*

 * Dashboard Plus - Manage taxonomy
   - *Shows all taxonomy vocabularies (or a configurable subset) with a term count and links to add a new term or manage existing terms for each vocabulary.*

 * Dashboard Plus - JSON Feed
   - *Displays items from any [JSON Feed](https://www.jsonfeed.org/)-compliant URL. Configurable URL, item count, date display, content display, and cache duration. Can be placed multiple times with different feeds. Useful for following external blogs or news sources directly from the dashboard.*

## Installation

 - Install this module using the official Backdrop CMS instructions at
   https://backdropcms.org/guide/modules.
   
   This module makes several blocks available for you to place in any layout. 
   While these blocks have been created with the editor dashboard in mind, they 
   can be used anywhere on the site.
   
   Blocks installed with this module will have a title starting with 
   Dashboard Plus
   
   **Example**: *Dashboard Plus - Backdrop Resources*
   
 - Enable blocks 
   https://backdropcms.org/user-guide/blocks
   
   

## Documentation

 Additional documentation is located in the Wiki:
 https://github.com/backdrop-contrib/dashboard_plus/wiki/Documentation.

## Issues

 Bugs and Feature requests should be reported in the Issue Queue:
 https://github.com/backdrop-contrib/dashboard_plus/issues.

## Current Maintainers

 - Tim Erickson (https://github.com/stpaultim)
 - Openings available....

## Credits

 - Work sponsored by: Simplo (https://simplo.site) by Triplo (https://triplo.co)

## License

 This project is GPL v2 software. See the LICENSE.txt file in this directory for
 complete text.


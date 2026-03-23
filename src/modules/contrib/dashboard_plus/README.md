Dashboard Plus
==============

Dashboard Plus extends the Backdrop CMS administrator dashboard with additional
blocks, self-management tools, and a site architecture report — all without
leaving the dashboard.

## Features

### Dashboard self-management

Site architects can manage the dashboard layout directly from the dashboard
itself, without visiting the layout manager.

**Block installer** — The Welcome block includes a compact inline form for
adding blocks to any dashboard region. Choose a block and a region, click
"Add to dashboard," done.

**Remove blocks** — Every dashboard block gains a "Remove from dashboard" option
in its contextual links menu (the gear icon). No layout manager visit required.

**Reorganize layout** — A "Reorganize layout" link opens a modal showing all
dashboard regions as drag-and-drop columns. Reorder blocks across regions and
save in one step.

### Architecture tab

An **Architecture** tab appears between Overview and Settings on the dashboard.
It provides a full read-only report covering:

- Content types and field usage
- Enabled modules and themes
- User roles and permissions
- Menus, taxonomies, views, and layouts
- Image styles, text formats, and more

Access is controlled by the `view architecture report` permission. The report is
also available at `admin/reports/site-architect`.

A compact **Architecture summary block** can be placed on the dashboard to show
key counts (total content, content types, modules, recipes, last cron run) with
a link to the full report.

### Collapsible dashboard blocks

All dashboard blocks can be individually collapsed and expanded. State is saved
per user. This feature is on by default and can be toggled in the dashboard
settings at `admin/dashboard/settings`.

---

## Blocks

### Management and setup

**Dashboard Plus - Welcome**
Combines the block installer, a link to the layout manager, and a "Reorganize
layout" trigger in one block. Placed automatically on first install. Remove it
once your dashboard is set up to your liking.

**Dashboard Plus - Architecture summary**
Compact at-a-glance stats: total content, content type count, enabled module
count, installed recipe count, and time since cron last ran. Links to the full
Architecture report. Requires the `view architecture report` permission.

### Content

**Dashboard Plus - Manage content**
Links to add or manage existing content for every content type on the site.
A more useful replacement for the core "Create content" block.

**Dashboard Plus - Content block** *(multiple instances supported)*
Displays a specific piece of content chosen by the site architect. Place the
block, then pick the content directly on the dashboard — no layout manager
visit required. Two options:

- *Find existing content* — filter by content type and select from published
  nodes.
- *Create a new Card* — creates a published Card node, wires it to the block,
  and takes you to the node edit form. When you save, the dashboard is already
  updated.

**Dashboard Plus - Quick links** *(multiple instances supported)*
A customizable collection of links to pages, forms, or configuration the site
architect uses often. Add links inline on the dashboard (URL + optional label).
Links are sorted alphabetically and displayed in a responsive grid that expands
to 3–4 columns in wide regions.

### Monitoring

**Dashboard Plus - Recent log messages**
The 10 most recent watchdog log entries with a link to the full log page. The
number of entries shown is configurable.

**Dashboard Plus - Recent unpublished comments**
A list of comments awaiting moderation with a link to the comment moderation
queue.

**Dashboard Plus - Cron status**
Shows when cron last ran with a warning or error indicator if it is overdue.
Includes a link to run cron manually.

### Community and resources

**Dashboard Plus - Resources**
Hardcoded links to key Backdrop CMS resources: forums, user guide, and the
core issue queue.

**Dashboard Plus - Backdrop release notes**
View release notes for the latest minor or bugfix release of Backdrop CMS.

**Dashboard Plus - Module of the day**
A randomly selected Backdrop CMS module pulled live from the BackdropCMS.org
project directory. Shows the title, a truncated description with a "Read more"
toggle, and install statistics. Includes a "See another" button to browse
without leaving the dashboard.

**Dashboard Plus - New module releases**
Lists the most recently published modules on BackdropCMS.org (first releases
only, not updates). Configurable to show 5, 10, 15, or 20 entries.

**Dashboard Plus - Current weather**
Current weather for a configured location: temperature, feels-like, humidity,
and wind speed. Configurable units (°C/°F) and an optional forecast link.
Powered by [Open-Meteo](https://open-meteo.com) — no API key required.

### Taxonomy and feeds

**Dashboard Plus - Manage taxonomy**
All taxonomy vocabularies (or a configurable subset) with a term count and
links to add or manage terms.

**Dashboard Plus - JSON Feed**
Displays items from any [JSON Feed](https://www.jsonfeed.org/)-compliant URL.
Configurable item count, date display, content display, and cache duration.
Can be placed multiple times with different feeds.

---

## Installation

Install this module using the official Backdrop CMS instructions:
https://backdropcms.org/guide/modules

On first install, the **Welcome block** is automatically placed in the top
region of the dashboard. Use it to add other Dashboard Plus blocks, then remove
it when your dashboard is configured.

All blocks provided by this module are prefixed with **Dashboard Plus** in the
block list. Most blocks are designed for the admin dashboard but can be placed
in any layout.

The `view architecture report` permission must be granted before people can
access the Architecture tab or Architecture summary block.

## Documentation

Additional documentation is located in the Wiki:
https://github.com/backdrop-contrib/dashboard_plus/wiki/Documentation

## Issues

Bugs and feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/dashboard_plus/issues

## Current Maintainers

- Tim Erickson (https://github.com/stpaultim)
- Openings available...

## Credits

- Work sponsored by: Simplo (https://simplo.site) by Triplo (https://triplo.co)

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

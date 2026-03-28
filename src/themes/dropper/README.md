# Dropper

Dropper is the application theme for the Drop Suite (DropCRM, DropTasks, DropTime, DropTickets, and related modules). It is a fork of [Tatsu](https://github.com/backdrop-contrib/tatsu) with Drop Suite-specific customizations.

## Features

- Bootstrap 4 grid and JS bundled
- Color module support for custom palettes
- Drop Suite layout (`drop_suite`) with header, navigation, content, and sidebar regions
- Custom CSS textarea in theme settings for quick overrides

## Installation

1. Place this theme in your `themes/` directory.
2. Enable it at *Administration > Appearance*.
3. The `drop_base` module serves `admin/drop/*` pages through Dropper automatically via `hook_custom_theme()`.

## Customization

Go to *Administration > Appearance > Settings* to adjust color palettes or add custom CSS.

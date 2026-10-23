# fedii

A cute, pink and white femboyish Mastodon fediverse client for Nintendo 3DS.

## Features

- View your Mastodon timeline
- Post toots
- Pink and white UI theme
- Touch screen support

## Installation

1. Install the .cia file via FBI or your preferred CIA installer
2. Launch from the home menu

## Building

### Requirements
- devkitARM
- libctru
- citro3d
- jansson (for JSON parsing)

### Build Steps
```bash
make
```

This will produce a .3dsx file. To build a .cia, use:
```bash
make cia
```

## Controls

- **A**: Select/Confirm
- **B**: Back/Cancel
- **D-Pad**: Navigate
- **Touch**: Select/Scroll

## Configuration

Edit `config.json` to set your Mastodon instance and credentials.

## License

MIT License

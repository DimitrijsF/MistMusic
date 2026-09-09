# MistMusic — USB MP3 Player for Blaupunkt CD30 MP3

**MistMusic replaces the original internal CD drive of a Blaupunkt CD30 MP3
head unit with a digital MP3 player based on an ESP microcontroller.**

It allows you to play MP3 music from a USB flash drive while keeping the
original CD30 MP3 head unit, factory controls and audio path.

In other words:

**CD30 MP3 + MistMusic = USB MP3 playback without replacing the head unit.**

## What is MistMusic?

MistMusic is an open-source hardware and firmware project that replaces the
original single-disc CD drive inside a **Blaupunkt CD30 MP3** head unit.

Instead of reading audio from a physical CD, MistMusic emulates the original
CD drive communication and provides music playback from a USB flash drive.

The original head unit remains in place and continues to handle the user
interface, display, buttons and audio system.

MistMusic is designed specifically as an **internal CD drive replacement**.

It is **not an external CD changer emulator**.

## Features

- Replacement for the original internal CD drive
- MP3 playback from a USB flash drive
- Works with the original CD30 MP3 controls
- Original head unit display and user interface
- Track selection using the factory controls
- Multiple virtual CD pages
- Automatic track switching
- Playback position tracking
- Resume playback
- Restoration of the previous track and virtual CD page
- Native status reporting to the head unit
- Uses the original head unit audio path
- ESP-based hardware

## Compatibility

MistMusic was developed and tested with:

- **Blaupunkt CD30 MP3**
- Opel vehicles using the compatible CD30 MP3 head unit

The project communicates with the head unit by emulating the behavior of its
original internal CD drive.

Other head units may use different communication protocols or hardware
configurations and are currently **not verified**.

## Why replace the CD drive?

The original CD drive is a mechanical device that is increasingly difficult
to maintain.

MistMusic allows the original head unit to remain in the vehicle while
replacing the mechanical media source with modern digital playback.

Music can be stored on a USB flash drive instead of burning CDs.

This keeps the original appearance and controls of the car while adding
convenient digital MP3 playback.

There is no need to replace the factory head unit with a modern aftermarket
radio just to get USB music playback.

## How It Works

MistMusic sits where the original internal CD drive was located.

The ESP-based hardware communicates with the CD30 MP3 head unit using the
same communication interface used by the original CD drive.

From the point of view of the head unit, MistMusic behaves like the original
CD mechanism.

When the user selects a track, changes a virtual CD page or uses the factory
controls, the firmware interprets these commands and controls the digital
media player accordingly.

Music files are read from the connected USB flash drive and played through
the original audio path of the head unit.

This allows the factory controls and display to continue to be used without
replacing the head unit itself.

## USB MP3 Playback

Music is played directly from a USB flash drive connected to MistMusic.

The USB storage acts as the digital replacement for physical CDs.

MP3 files can be organized into virtual CD pages and tracks, allowing the
original CD30 MP3 interface to be used for navigating the music collection.

The firmware handles the USB storage, file selection, playback state and
communication with the head unit.

## Playback Model

MistMusic presents the music stored on the USB flash drive to the head unit
as a collection of virtual CD pages and tracks.

The head unit therefore continues to operate using its familiar CD-based
interface while MistMusic maps those commands to MP3 files on the USB media.

Tracks can be selected using the original controls, and the firmware keeps
track of the current playback position and virtual CD page.

## Resume Playback

MistMusic keeps track of the current playback state.

After restarting the system, playback can be restored to the previously
selected virtual CD page and track.

This allows MistMusic to behave more like a permanent replacement for the
original CD drive rather than a completely separate media player.

## Audio Playback

Audio playback uses the original audio path of the CD30 MP3 head unit.

MistMusic provides the digital playback side while the factory head unit
continues to handle the audio system.

No separate aftermarket audio input is required.

## Controls

MistMusic is designed to work with the original controls of the Blaupunkt
CD30 MP3 head unit.

The factory controls can be used for operations such as:

- Selecting tracks
- Changing virtual CD pages
- Starting and stopping playback
- Moving between tracks
- Controlling normal playback functions supported by the head unit

The goal is to keep the user experience as close as possible to using the
original CD drive.

## Project Structure

The firmware is divided into several major components:

### CDC

Handles communication with the CD30 MP3 head unit and emulates the original
CD drive behavior.

### Media Player

Controls the playback state, track selection, virtual CD pages, resume
functionality and related playback logic.

### Media Decoder

Handles decoding of supported audio files.

### Media Output

Handles sending decoded audio to the appropriate audio output path.

## Hardware

MistMusic is based on an ESP microcontroller and additional hardware required
to interface with the original CD30 MP3 electronics.

The hardware replaces the original internal CD drive and connects to the head
unit through the corresponding interface.

The project is intended to be installed inside the original head unit in place
of the CD drive.

For detailed information about the physical hardware integration, wiring,
connections and installation, feel free to contact the author directly.

## Development Status

**Version 1.0 — working release**

The core USB MP3 playback functionality has been implemented and tested with
the Blaupunkt CD30 MP3.

The project is **still under active development**.

The current release provides the core functionality required to replace the
original CD drive with USB-based MP3 playback, while additional features and
improvements are planned for future versions.

Some parts of the project may therefore change as development continues.

## Building Your Own MistMusic

MistMusic is an open-source project intended for people who want to build
their own digital media replacement for the original CD drive.

You will need:

- Compatible ESP-based hardware
- The required interface electronics
- A USB flash drive for digital media
- A compatible Blaupunkt CD30 MP3 head unit
- Firmware from this repository
- Appropriate hardware assembly and wiring

Before connecting anything to a vehicle head unit, make sure you understand
the electrical connections and the required interface.

## Important Notes

This project was developed specifically around the behavior of the
**Blaupunkt CD30 MP3** and its original internal CD drive communication.

Other head units may use different protocols or hardware configurations.
Compatibility with other models is not guaranteed.

The project is still being developed, so hardware and firmware behavior may
change in future versions.

### Hardware and Installation Disclaimer

MistMusic involves modifying the internal hardware of a vehicle head unit.

The author of this project is **not responsible for any damage** caused by
incorrect assembly, wiring, installation, modification or use of the project.

If you are not sure what you are doing when working with the head unit or
vehicle electronics, stop and make sure you understand the hardware before
applying power.

Incorrect wiring, careless modifications or improper installation can damage
the head unit, the MistMusic hardware, or other vehicle electronics.

**You modify your hardware at your own risk.**

## Hardware Integration Help

The firmware repository primarily contains the software side of MistMusic.

If you need more detailed information about the **physical integration of
the hardware into the Blaupunkt CD30 MP3**, including wiring, connections,
installation or hardware assembly, feel free to contact me directly.

I will be happy to help with questions about the physical integration of
MistMusic into the head unit.

## License

This project is open source.

See the repository license for the exact licensing terms.

## Disclaimer

MistMusic is an independent project and is not affiliated with, endorsed by
or sponsored by Opel, Blaupunkt or any other manufacturer mentioned in this
repository.

All product names and trademarks belong to their respective owners.

Use the project at your own risk.

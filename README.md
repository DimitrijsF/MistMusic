# MistMusic

MistMusic is a firmware project that replaces the original single-disc CD drive inside a **Blaupunkt CD30 MP3** head unit with a modern digital music player.

Instead of reading audio CDs, the original CD drive communication is emulated, allowing the head unit to control MP3 playback through its original interface and controls.

The goal of the project is to preserve the original appearance and user experience of the factory head unit while replacing the mechanical CD playback system with modern digital audio playback.

## Features

- Replacement for the original single-disc CD drive
- MP3 playback from USB storage
- Control using the original head unit controls
- Track selection through the factory CD interface
- Multiple virtual CD pages
- Automatic track switching
- Playback position tracking
- Resume playback support
- Restoration of the previously selected track and page
- Native playback status reporting to the head unit
- Audio output through the original head unit audio path
- Support for Bluetooth audio playback
- Wi-Fi can be used as a service/configuration feature

The project does not emulate an external CD changer.  
It emulates the original **internal single-disc CD drive mechanism** of the head unit.

## Compatibility

MistMusic is currently developed and tested with:

- **Blaupunkt CD30 MP3**

The firmware communicates with the head unit using the protocol originally used by its internal CD drive.

Compatibility with other head units has not been verified.

## How It Works

The original CD mechanism is replaced by a microcontroller-based system.

The head unit communicates with the replacement firmware as if it were communicating with its original CD drive.

The firmware handles:

- CD drive communication and protocol responses
- Playback state management
- Track and page selection
- Playback status reporting
- MP3 decoding
- Audio output
- Resume position handling

From the head unit's perspective, the replacement behaves like the original CD playback system while the actual audio source is digital media.

## Playback Model

Tracks stored on the media are presented to the head unit using a virtual CD-style structure.

The firmware maps real media tracks to the track and page structure expected by the original CD interface.

This allows the original controls of the head unit to be used for navigation without modifying the factory user interface.

When the end of a track is reached, playback automatically continues with the next available track.

Track changes across virtual page boundaries are also handled automatically.

## Resume Playback

MistMusic stores the current playback position when playback is stopped.

When playback is started again, the firmware attempts to resume from the previously stored position.

The resume information includes:

- Current track
- Playback position within the media file

Playback time is restored together with the playback position so that the head unit continues displaying the correct track time after resume.

## Audio Playback

MP3 files are decoded by the firmware and sent to the audio output used by the head unit.

The audio output is configured to work with the existing hardware audio path while preserving compatibility with the original head unit.

The player handles audio decoder startup and format configuration automatically when playback begins.

## Project Structure

The firmware is divided into several functional parts.

### CDC

Handles communication with the head unit and emulates the behavior of the original CD drive.

Responsibilities include:

- Receiving commands from the head unit
- Sending protocol responses
- Playback status reporting
- Track selection handling
- Play and stop state management

### Media Player

Controls the overall playback process.

Responsibilities include:

- Starting and stopping playback
- Opening tracks
- Switching tracks
- Handling end-of-file events
- Playback resume
- Playback time tracking
- Track and page management

### Media Decoder

Handles MP3 decoding and media file access.

Responsibilities include:

- Opening media files
- Seeking to playback positions
- Decoding MP3 frames
- Providing PCM audio samples
- Detecting end-of-file conditions

### Media Output

Handles PCM audio output.

Responsibilities include:

- Audio format configuration
- Starting and stopping the audio output
- Sending decoded PCM samples to the audio hardware

## Controls

The project is designed to work with the original controls of the **Blaupunkt CD30 MP3** head unit.

Track selection and playback control are handled through the factory CD interface.

The intention is to preserve the original user experience as much as possible without adding a separate user interface for normal operation.

## Configuration

MistMusic is intended to operate primarily through the original head unit controls.

Additional connectivity features, such as Wi-Fi, are intended for service or configuration purposes rather than normal everyday playback.

The head unit should remain the primary user interface.

## Development Status

MistMusic **v1.0** is considered a working release.

The core functionality has been tested with a **Blaupunkt CD30 MP3** head unit, including:

- Head unit communication
- CD drive emulation
- MP3 playback
- Track selection
- Virtual page switching
- Automatic track changes
- Playback status reporting
- Playback resume
- Playback time restoration

Further development may include additional features and hardware improvements.

## Important Notes

This project was developed specifically around the behavior of the **Blaupunkt CD30 MP3** and the communication of its original internal CD drive.
Other head units may use different protocols or hardware configurations.
Compatibility with other models is not guaranteed.

## License

The license for this project is defined in the repository.

## Disclaimer

This project involves modification or replacement of the original hardware inside a vehicle head unit.
Use the project at your own risk.
Always make sure that modifications to vehicle electronics are performed safely and do not interfere with vehicle operation.

## Support

If you are building MistMusic yourself and need help with the installation, wiring, or assembling the required hardware around the ESP, feel free to contact me.
I will be happy to help with questions related to the hardware setup and installation.

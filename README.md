# SimplePlayerQt

A lightweight, cross-platform music player built with Qt6.

## Features

- **Audio Playback**: Support for MP3, FLAC, OGG, WAV, M4A, AAC, and WMA formats
- **Playlist Management**: Add, remove, and organize your music tracks
- **File Explorer**: Built-in file browser with drag-and-drop support
- **Playback Modes**: Shuffle and repeat (off/all/one) modes
- **Media Controls**: Play, pause, stop, next, previous, and seek
- **Volume Control**: Adjustable volume with visual feedback
- **Keyboard Shortcuts**: Quick access to all major functions
- **MPRIS2 Integration**: System media key support on Linux
- **Persistent Settings**: Remembers your last browsing location
- **Metadata Display**: Shows artist, album, title, and track information

## Screenshots

![SimplePlayerQt Interface](screenshot.png)

## Requirements

### Build Dependencies
- **Qt6** (6.0 or later)
  - Qt6::Core
  - Qt6::Gui
  - Qt6::Widgets
  - Qt6::Multimedia
  - Qt6::MultimediaWidgets
  - Qt6::DBus (Linux only)
- **CMake** 3.16 or later
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Runtime Dependencies
- Qt6 libraries
- System audio drivers
- D-Bus (for MPRIS2 support on Linux)

## Building from Source

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev

# Clone and build
git clone https://github.com/OCSPG/simple-player-qt.git
cd simple-player-qt
mkdir build && cd build
cmake ..
make
```

### macOS

```bash
# Install dependencies
brew install qt6 cmake

# Clone and build
git clone https://github.com/OCSPG/simple-player-qt.git
cd simple-player-qt
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt6) ..
make
```

### Windows

```bash
# Install Qt6 from https://www.qt.io/download
# Install CMake from https://cmake.org/download/

# Clone and build (in Qt Command Prompt)
git clone https://github.com/OCSPG/simple-player-qt.git
cd simple-player-qt
mkdir build && cd build
cmake -G "NMake Makefiles" ..
nmake
```

## Running

After building:

```bash
# From the build directory
./SimplePlayerQt
```

## Usage

### Adding Music
- **Drag & Drop**: Drag files from the built-in file explorer or external file manager onto the playlist
- **Double-click**: Double-click audio files in the file explorer to add them to the playlist
- **Browse**: Navigate folders using the back/forward/up buttons

### Playback Controls
- **Play/Pause**: Click the play/pause button or press `Space`
- **Stop**: Click the stop button
- **Next/Previous**: Use the skip buttons or press `N`/`P`
- **Seek**: Drag the position slider or press `Left`/`Right` arrows (±5 seconds)

### Playlist Management
- **Play Track**: Double-click any track in the playlist
- **Remove Track**: Right-click and select "Remove"
- **Clear Playlist**: Right-click and select "Clear All"

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Play/Pause |
| `N` | Next track |
| `P` | Previous track |
| `S` | Toggle shuffle |
| `R` | Cycle repeat mode (Off → All → One) |
| `←` / `→` | Seek backward/forward 5 seconds |
| `↑` / `↓` | Volume up/down |

### Playback Modes
- **Shuffle** (S): Randomizes playback order
- **Repeat Off**: Stops after playlist ends
- **Repeat All**: Loops the entire playlist
- **Repeat One**: Repeats current track

## Configuration

Settings are automatically saved to:
- **Linux**: `~/.config/SimplePlayerQt/SimplePlayerQt.conf`
- **macOS**: `~/Library/Preferences/com.SimplePlayerQt.SimplePlayerQt.plist`
- **Windows**: Registry under `HKEY_CURRENT_USER\Software\SimplePlayerQt\SimplePlayerQt`

Currently saved settings:
- Last browsed folder location

## Known Issues

1. **Performance**: Loading metadata for many files at once may temporarily freeze the UI
2. **MPRIS2**: Some media control features are not fully implemented yet
3. **File Explorer**: Deep folder hierarchies are limited to 2 levels for performance

See [CHECKUP_REPORT.md](CHECKUP_REPORT.md) for detailed technical analysis.

## Roadmap

- [ ] Asynchronous metadata loading
- [ ] Complete MPRIS2 implementation
- [ ] Playlist save/load functionality
- [ ] Equalizer support
- [ ] Album artwork display
- [ ] Lyrics support
- [ ] Library management
- [ ] Theme customization

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### Development Setup

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test on your platform
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [Qt6](https://www.qt.io/)
- Inspired by classic desktop music players
- MPRIS2 specification for Linux desktop integration

## Support

For bug reports and feature requests, please use the [GitHub Issues](https://github.com/OCSPG/simple-player-qt/issues) page.

---

**Author:** SPG
**Version:** 1.0.0
**Last Updated:** 2025-11-05

# SIEdit

SIEdit is a fully graphical viewer and editor of the SI streaming asset pack format used by LEGO Island. It can view, extract, and replace data in all of the game's SI files, allowing textures, sounds, animations, path information, FMVs, and even some game logic/metadata to be modified. It is currently still under development, but should work with all files.

![SIEdit Screenshot](https://raw.githubusercontent.com/itsmattkc/SIEdit/master/packaging/screenshot.png)

## libweaver

All of the SI-specific code is contained in a separate C++ library called **libweaver**, enabling other projects to re-use the discoveries here and work with SI files too. Currently there is no documentation or stable API, however it should be fairly straightforward, and I plan to add those things in the future.

## Building

SIEdit and libweaver use the fairly standard CMake build system.

**SIEdit** requires Qt 6+ and FFmpeg for the UI and media playback respectively.

**libweaver** uses only standard libraries from C++98, and shouldn't require anything special.

## Future

Here's a non-exhaustive list of things I'd like to add in the future:

- Auto-conversion/conforming when replacing files. Right now, when you replace files, you have to manually convert them into the right format before doing so (e.g. 256-color bitmap with specific paletting, Smacker 2, FLIC, WAV, et al.) It would be nice if SIEdit did this automatically.
- Stable API for libweaver. The library is fairly usable as-is, but it isn't the cleanest thing in the world right now, and I'd like for other people to be able to use it with as little headache as possible. This is my first time providing a library for use in other projects so I also may have made some mistakes.
- Ability to create SI files from scratch. Currently any changes made must use an existing SI as a base. It would be interesting to be able to load completely custom SIs into the game. This may require some more reverse engineering, though I think we have most of the work down.
  - As a corollary from this, also the ability to add/delete objects, as opposed to just replacing what already exists.
- More packaging options. Currently we only provide a Windows build, which is ironic since I'm not even a Windows user. We may add a build for macOS and an AppImage for Linux, and I might try setting up a package on the AUR since I use Arch (btw)

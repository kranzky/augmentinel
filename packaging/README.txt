Augmentinel
===========

A re-skinned version of Geoff Crammond's classic The Sentinel (aka The Sentry).
It runs the original ZX Spectrum game under emulation for authentic gameplay,
with accelerated 3D graphics, sound and music from the other versions, and all
57,344 landscapes.

Original Windows version by Simon Owen (https://simonowen.com/spectrum/augmentinel/).
macOS and Windows port by Jason Hutchens.

Starting the game
-----------------

Windows: run Augmentinel.exe. No installation is needed.

macOS: open Augmentinel.app (if you downloaded the disk image, drag it into
Applications first).

Controls
--------

Title screen
  Any key            continue
  Escape             quit

Landscape selection
  Left / Right       previous / next landscape
  Page Up / Down     jump back / forward through the list
  Home / End         first / last landscape
  Return or click    play the landscape
  Escape             back to the title screen

Game
  Mouse              look around
  Left click or A    absorb
  Right click or Q   transfer to the robot you're looking at
                     (looking steeply up into the sky: hyperspace)
  Middle click or R  create a robot
                     (placing one in the sky shows the sky view)
  T                  create a tree
  B or mouse X1      create a boulder
  H                  hyperspace
  U                  U-turn
  Arrow keys         turn and look up/down
  Page Up / Down     turn 45 degrees
  P or Pause         pause
  Escape             abandon the landscape

Anywhere
  1 2 3 4            Amiga, Commodore 64, BBC Micro or Spectrum sounds
  M                  music on/off
  N                  tunes on/off
  - / =              music volume
  F11 or Alt+Enter   toggle fullscreen
  Tab                show or hide performance stats

Settings
--------

Settings and the landscapes you have unlocked are saved in:

  Windows: %APPDATA%\Augmentinel\settings.ini
  macOS:   ~/Library/Application Support/Augmentinel/settings.ini

Advanced options can be set by editing that file, in the [Main] section:
MouseSpeed, InvertMouse, GameSpeed, VerticalFov, MsaaSamples (1, 2 or 4),
HexLandscapes, RotateLandscape.

License
-------

Augmentinel is free software under the GNU GPL v3. Source code:
https://github.com/kranzky/augmentinel

This is an unofficial fan creation, distributed without charge, with no
affiliation with the original developer, publisher, or other rights holders.

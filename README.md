# StickyNotes

Sticky notes module for the Moksha desktops.

The Sticky Notes module is intended to be a small project that enables adding elegant notes directly on the screen. While there are several sticky note apps available, I couldn’t find an Enlightenment solution that met my needs. This project also serves as a coding exercise to deepen my understanding of the Edje layer and module programming.

Features:

- Word wrap
[<img align="right" src="http://i.imgur.com/QuEYaJf.png">](http://i.imgur.com/QuEYaJf.png)
- Any command output on the screen
- Refresh interval for command output up to 1 hour
- Refresh command after system resume
- Refresh command after clicking header
- Font sizes from 8px up to 16px supported (click on the header icon)
- Text markdown-like tags formating support:
  [TEXBLOCK object](https://www.enlightenment.org/develop/legacy/program_guide/evas/textblock_objects)
- Note text displayed in the header
- Notification on the text changes
- ncal support for day highlight (best result with ncal -hwbM)
- Nice transparent look
- Light on system resources

## Settings dialog

  [<img src="https://i.imgur.com/Jo0uBJk.jpg ">](https://i.imgur.com/L02F1L2.jpg)

# Installation

It is recommended Bodhi users install from Bodhi's repo:

```ShellSession
sudo apt update
sudo apt install moksha-module-stickynotes
```

Other users need to compile the code:

First install all the needed dependencies. Note this includes not only EFL but the EFL header files. If you have compiled and installed EFL and Moksha from source code this should be no problem.

Then the usual:

```ShellSession
meson . build
ninja -C build
sudo ninja -C build
```

## Reporting bugs

Please use the GitHub issue tracker for any bugs or feature suggestions:

>[github.com/thewaiter/StickyNotes/issues](https://github.com/thewaiter/StickyNotes/issues)

## Contributing

Help is always Welcome, as with all Open Source Projects the more people that help the better it gets!
More translations would be especially welcome and much needed.

Please submit patches to the code or documentation as GitHub pull requests!

Contributions must be licensed under this project's copyright (see COPYING).


2022 04 16
<Stefan Uram thewaiter@centrum.sk>

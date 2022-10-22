# Fork info

This is the Wizard of Gnomes fork of AACS (https://github.com/tomasz-grobelny/AACS) designed specifically to allow Android Auto support in a headunit to be used to implement a carputer. As such, it removes the 'client' and 'getevents' code, and modifies the library to stream from the framebuffer instead of a video source.

Features:
* Raspberry Pi 4 support including hardware video encoding
* Touch HID events generated in response to head unit touches

Planned/todo:
* Audio support for playback and recording from the headunit
* GPS data (and other sensors?) from the headunit

Issues:
* Certs need updated (see https://github.com/tomasz-grobelny/AACS/issues/15)
* The HID descriptor seems to upset libevdev and requires a workaroud (see below). This is presumably fixable.

To use:
* Configure the Pi to manually enable a framebuffer of the desired size (as HDMI detection will fail, and framebuffers are disabled by default)
* Build on Raspberry Pi
* `sudo modprobe libcomposite`
* `sudo ./AAServer` from the relevant directory
After AAServer is launched a new HID device will appear in /dev/input. It is currently necessary to 'fix' the resolution:
* `libevdev-tweak-device  --resolution 320,240 /dev/input/event3`
After this X Windows may need restarted (if that's the environment you wish to stream).

Instead of X11 you can also try any app that can write directly into the framebuffer, e.g. mplayer


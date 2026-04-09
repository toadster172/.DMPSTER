# .DMPSTER: Leapster 2 cart dumper

To install / run, copy SDMenu.bin to the internal storage / SD card, and choose  "game
downloads" (arcade icon, internal storage models) or "more activities and games" (SD
card icon, SD models) at the system main menu. If you have anything you care about on
your SD card / storage, you should back it up beforehand just in case something corrupts.

Note that his homebrew currently replaces the SD menu entirely; if you know what you're
doing, you can package it as its own downloadable instead, putting it into a folder
along with its own Flash icon .swf and a meta.inf (can be copied from another game w/
the filename in the meta.inf changed), but this isn't recommended as booting directly
into the cartridge dumper is a lot faster than choosing it in the SD menu every single
time, especially if dumping multiple cartridges.

---

To mount Leapster2 units on a computer without an older copy of LeapFrog Connect, use
[LFTools](https://github.com/lfhacks/LFTools) (particularly useful for models with
internal storage). Alternatively, an older copy of LeapFrog Connect may be used with
the "Alternate connection method for Leapster2, Didj, and Crammer" option enabled,
but your mileage may vary, especially on modern versions of Windows.

---

Dumps are made to `/Leapster/[BIOS/CART]_0x[ROM_CHECKSUM]/bin`. Attempting to reboot will
just reload the SD menu, so batteries will need to be removed after powering off to exit
the homebrew for now.

Note that cartridge ROMs cannot be booted as or converted to downloadable ROMs, as the
structure of downloadables and cartridge ROMs differ in such a way that we cannot convert
between the two as of now. Cartridge ROMs are also usually too large to fit in RAM anyway.

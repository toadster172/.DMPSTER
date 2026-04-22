# .DMPSTER: Leapster 2 cart dumper

To install and run, copy SDMenu.bin to the internal storage / SD card, sign into any
profile except Guest, and choose "Game Downloads" (arcade icon, internal storage models)
or "More Activities and Games" (SD card icon, SD models) at the system main menu. If you
have anything you care about on your SD card / storage, you should back it up beforehand
just in case something corrupts.

Note that this homebrew currently replaces the SD menu entirely; if you know what you're
doing, you can package it as its own downloadable instead, putting it into a folder
along with its own Flash icon .swf and a meta.inf (can be copied from another game w/
the filename in the meta.inf changed), but this isn't recommended as booting directly
into the cartridge dumper is a lot faster than choosing it in the SD menu every single
time, especially if dumping multiple cartridges.

---

To mount Leapster2 units on a computer, use [LFTools](https://github.com/lfhacks/LFTools)
(particularly useful for models with internal storage). Alternatively, an older copy of
LeapFrog Connect may be used with the "Alternate connection method for Leapster2, Didj,
and Crammer" option enabled, but your mileage may vary, especially on modern versions of
Windows.

---

Dumps are made to `/Leapster/[BIOS/CART]_0x[ROM_CHECKSUM]/bin`. To exit the dumper,
remove the batteries and put them back in. Attempting to reboot via other means (eg. cart
pull, holding power button) will just reload the dumper due to weird Leapster2 downloadable
boot behavior, so batteries will need to be removed after powering off to exit the dumper
for now.

Note that cartridge ROMs cannot be booted as or converted to downloadable ROMs, as the
structure of downloadables and cartridge ROMs differ in such a way that we cannot convert
between the two as of now. Cartridge ROMs are also sometimes too large to fit in RAM anyway.
In other words, you can't dump cartridges to a format that you can play natively.

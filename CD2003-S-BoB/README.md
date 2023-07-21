Update (September-2023): This design finally worked! I had to tweak the BFO
frequency a bit via a brute-force loop in the code.

With a 3pF coupling capacitor and a `SFU455B` ceramic filter from eBay, the
439500 Hz BFO frequency produced the strongest audio output. Your results WILL
vary - please note!

Update 2: The BFO tuning process is finicky - I don't get it :-(

Hack: Converting `CD2003-S-BoB` in to a DCR `CD2003-BoB-v2` PCB.

- Remove the ceramic filter (`FL`), if installed.

- Chop off the Pin 11 of the CD2003 IC

- Feed 5V to the Pin 7 of CD2003 - this can be done by bridging the two pads
  (out of the three pads) of the `DIP-3 ceramic filter`.

- Connect Pin 4 of CD2003 to the right-side of the C1 capacitor using a short
  piece of wire.

- Bridge C25. The output audio now comes from Pin 4 (the BFO) of the JP1
  header.

These mods hardly take 5 minutes, so do give them a go!

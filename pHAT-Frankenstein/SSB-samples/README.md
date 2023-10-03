The quality of the microphone used makes a HUGE / BIGGEST difference. Use the
Blue Yeti microphone as a reference / debugging aid.

The cheap 2 USD USB micrphone from Robu sucks it seems.

Best results with ontained with the Blue Yeti microphone and my homebrew PUI
5024 one (in the shell of the 2 USD USB microphone from Robu).

Magic:

```
arecord -c1 -r48000 -D plughw:CARD=0 -fS16_LE - | csdr convert_i16_f | csdr gain_ff 1.0 | csdr dsb_fc |  sudo rpitx -i- -m IQFLOAT -f 28e6
```

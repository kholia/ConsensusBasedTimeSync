<QucsStudio Schematic 4.3.1>
<Properties>
View=0,0,1395,1006,1.3967,0,68
Grid=10,10,1
DataSet=*.dat
DataDisplay=*.dpl
OpenDisplay=1
showFrame=0
FrameText0=Title
FrameText1=Drawn By:
FrameText2=Date:
FrameText3=Revision:
</Properties>
<Symbol>
</Symbol>
<Components>
Pac P1 1 210 190 18 -26 0 0 "1" 1 "50" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0 "SUBCLICK" 0
GND * 1 210 220 0 0 0 0
GND * 1 320 220 0 0 0 0
GND * 1 460 220 0 0 0 0
GND * 1 600 220 0 0 0 0
C C1 1 320 190 17 -26 0 1 "120pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C3 1 600 190 17 -26 0 1 "120pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
L L1 1 390 110 -26 10 0 0 "360nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
C C2 1 460 190 17 -26 0 1 "220pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
Pac P2 1 1050 200 18 -26 0 0 "2" 1 "50" 1 "0" 0 "1 GHz" 0 "26.85" 0 "SUBCLICK" 0
GND * 1 1050 230 0 0 0 0
L L2 1 530 110 -26 10 0 0 "360nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
GND * 1 780 200 0 0 0 0
L L3 1 780 170 10 -26 0 1 "1.02uH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
C C4 1 730 110 -26 -55 0 2 "470pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C5 1 830 110 -26 -55 0 2 "470pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
.SP SP1 1 210 290 0 67 0 0 "log" 1 "10kHz" 1 "100MHz" 1 "500" 1 "no" 0 "1" 0 "2" 0 "none" 0
</Components>
<Wires>
210 110 210 160 "" 0 0 0 ""
210 110 320 110 "" 0 0 0 ""
320 110 320 160 "" 0 0 0 ""
460 110 460 160 "" 0 0 0 ""
600 110 600 160 "" 0 0 0 ""
320 110 360 110 "" 0 0 0 ""
420 110 460 110 "" 0 0 0 ""
600 110 700 110 "" 0 0 0 ""
1050 110 1050 170 "" 0 0 0 ""
760 110 780 110 "" 0 0 0 ""
860 110 1050 110 "" 0 0 0 ""
560 110 600 110 "" 0 0 0 ""
460 110 500 110 "" 0 0 0 ""
780 110 800 110 "" 0 0 0 ""
780 110 780 140 "" 0 0 0 ""
</Wires>
<Diagrams>
<Smith 884 550 220 220 31 #c0c0c0 1 00 1 0 1 1 1 0 1 1 1 0 1 1 315 0 225 "" "" "">
	<Legend 10 -100 0>
	<"S[1,1]" "" #0000ff 0 3 0 0 0 0 "">
</Smith>
<Rect 87 908 1199 478 31 #c0c0c0 1 00 1 0 1 1 1 0 1 1 1 0 1 1 315 0 225 "" "" "">
	<Legend 10 -100 0>
	<"dB(S[1,1])" "" #0000ff 0 3 0 0 0 0 "">
	<"dB(S[2,1])" "" #ff0000 0 3 0 0 0 0 "">
</Rect>
</Diagrams>
<Paintings>
Text 370 290 16 #000000 0 low-pass filter, 34MHz cutoff \n 5^{th} order Chebyshev , PI-type, \n impedance 50 ohms
</Paintings>

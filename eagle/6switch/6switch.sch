<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="8.3.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="99" name="SpiceOrder" color="5" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="ESP12E_DEVKIT">
<packages>
<package name="ESP12E_DEVKIT">
<description>ESP12E_DEVKIT</description>
<wire x1="-12.7" y1="-24.13" x2="-12.7" y2="24.13" width="0.2032" layer="51"/>
<wire x1="-12.7" y1="-24.13" x2="-3.81" y2="-24.13" width="0.2032" layer="51"/>
<wire x1="-3.81" y1="-24.13" x2="3.81" y2="-24.13" width="0.2032" layer="51"/>
<wire x1="3.81" y1="-24.13" x2="12.7" y2="-24.13" width="0.2032" layer="51"/>
<wire x1="-12.7" y1="24.13" x2="-7.62" y2="24.13" width="0.2032" layer="51"/>
<wire x1="-7.62" y1="24.13" x2="7.62" y2="24.13" width="0.2032" layer="51"/>
<wire x1="7.62" y1="24.13" x2="12.7" y2="24.13" width="0.2032" layer="51"/>
<wire x1="12.7" y1="24.13" x2="12.7" y2="-24.13" width="0.2032" layer="51"/>
<pad name="AD0" x="-11.43" y="17.78" drill="0.8" diameter="1.9304"/>
<pad name="RSV@0" x="-11.43" y="15.24" drill="0.8" diameter="1.9304"/>
<pad name="RSV@1" x="-11.43" y="12.7" drill="0.8" diameter="1.9304"/>
<pad name="SD3" x="-11.43" y="10.16" drill="0.8" diameter="1.9304"/>
<pad name="SD2" x="-11.43" y="7.62" drill="0.8" diameter="1.9304"/>
<pad name="SD1" x="-11.43" y="5.08" drill="0.8" diameter="1.9304"/>
<pad name="CMD" x="-11.43" y="2.54" drill="0.8" diameter="1.9304"/>
<pad name="SD0" x="-11.43" y="0" drill="0.8" diameter="1.9304"/>
<pad name="CLK" x="-11.43" y="-2.54" drill="0.8" diameter="1.9304"/>
<pad name="GND@0" x="-11.43" y="-5.08" drill="0.8" diameter="1.9304"/>
<pad name="3V3@0" x="-11.43" y="-7.62" drill="0.8" diameter="1.9304"/>
<pad name="EN" x="-11.43" y="-10.16" drill="0.8" diameter="1.9304"/>
<pad name="RST" x="-11.43" y="-12.7" drill="0.8" diameter="1.9304"/>
<pad name="GND@1" x="-11.43" y="-15.24" drill="0.8" diameter="1.9304"/>
<pad name="VIN" x="-11.43" y="-17.78" drill="0.8" diameter="1.9304"/>
<pad name="D0" x="11.43" y="17.78" drill="0.8" diameter="1.9304"/>
<pad name="D1" x="11.43" y="15.24" drill="0.8" diameter="1.9304"/>
<pad name="D2" x="11.43" y="12.7" drill="0.8" diameter="1.9304"/>
<pad name="D3" x="11.43" y="10.16" drill="0.8" diameter="1.9304"/>
<pad name="D4" x="11.43" y="7.62" drill="0.8" diameter="1.9304"/>
<pad name="3V3@1" x="11.43" y="5.08" drill="0.8" diameter="1.9304"/>
<pad name="GND@2" x="11.43" y="2.54" drill="0.8" diameter="1.9304"/>
<pad name="D5" x="11.43" y="0" drill="0.8" diameter="1.9304"/>
<pad name="D6" x="11.43" y="-2.54" drill="0.8" diameter="1.9304"/>
<pad name="D7" x="11.43" y="-5.08" drill="0.8" diameter="1.9304"/>
<pad name="D8" x="11.43" y="-7.62" drill="0.8" diameter="1.9304"/>
<pad name="RX" x="11.43" y="-10.16" drill="0.8" diameter="1.9304"/>
<pad name="TX" x="11.43" y="-12.7" drill="0.8" diameter="1.9304"/>
<pad name="GND@3" x="11.43" y="-15.24" drill="0.8" diameter="1.9304"/>
<pad name="3V3@2" x="11.43" y="-17.78" drill="0.8" diameter="1.9304"/>
<wire x1="-7.62" y1="24.13" x2="-7.62" y2="1.27" width="0.2032" layer="51"/>
<wire x1="-7.62" y1="1.27" x2="7.62" y2="1.27" width="0.2032" layer="51"/>
<wire x1="7.62" y1="1.27" x2="7.62" y2="24.13" width="0.2032" layer="51"/>
<wire x1="-3.81" y1="-24.13" x2="-3.81" y2="-20.32" width="0.2032" layer="51"/>
<wire x1="-3.81" y1="-20.32" x2="3.81" y2="-20.32" width="0.2032" layer="51"/>
<wire x1="3.81" y1="-20.32" x2="3.81" y2="-24.13" width="0.2032" layer="51"/>
<wire x1="-5.08" y1="19.05" x2="-5.08" y2="22.86" width="0.2032" layer="51"/>
<wire x1="-5.08" y1="22.86" x2="-3.81" y2="22.86" width="0.2032" layer="51"/>
<wire x1="-3.81" y1="22.86" x2="-3.81" y2="20.32" width="0.2032" layer="51"/>
<wire x1="-3.81" y1="20.32" x2="-2.54" y2="20.32" width="0.2032" layer="51"/>
<wire x1="-2.54" y1="20.32" x2="-2.54" y2="22.86" width="0.2032" layer="51"/>
<wire x1="-2.54" y1="22.86" x2="-1.27" y2="22.86" width="0.2032" layer="51"/>
<wire x1="-1.27" y1="22.86" x2="-1.27" y2="20.32" width="0.2032" layer="51"/>
<wire x1="-1.27" y1="20.32" x2="0" y2="20.32" width="0.2032" layer="51"/>
<wire x1="0" y1="20.32" x2="0" y2="22.86" width="0.2032" layer="51"/>
<wire x1="0" y1="22.86" x2="1.27" y2="22.86" width="0.2032" layer="51"/>
<wire x1="1.27" y1="22.86" x2="1.27" y2="20.32" width="0.2032" layer="51"/>
<wire x1="1.27" y1="20.32" x2="2.54" y2="20.32" width="0.2032" layer="51"/>
<wire x1="2.54" y1="20.32" x2="2.54" y2="22.86" width="0.2032" layer="51"/>
<wire x1="2.54" y1="22.86" x2="5.08" y2="22.86" width="0.2032" layer="51"/>
<wire x1="-7.62" y1="-20.32" x2="-5.08" y2="-20.32" width="0.2032" layer="51"/>
<wire x1="-5.08" y1="-20.32" x2="-5.08" y2="-22.86" width="0.2032" layer="51"/>
<wire x1="-5.08" y1="-22.86" x2="-7.62" y2="-22.86" width="0.2032" layer="51"/>
<wire x1="-7.62" y1="-22.86" x2="-7.62" y2="-20.32" width="0.2032" layer="51"/>
<wire x1="5.08" y1="-20.32" x2="5.08" y2="-22.86" width="0.2032" layer="51"/>
<wire x1="5.08" y1="-22.86" x2="7.62" y2="-22.86" width="0.2032" layer="51"/>
<wire x1="7.62" y1="-22.86" x2="7.62" y2="-20.32" width="0.2032" layer="51"/>
<wire x1="7.62" y1="-20.32" x2="5.08" y2="-20.32" width="0.2032" layer="51"/>
<hole x="-10.16" y="21.59" drill="3.2"/>
<hole x="10.16" y="21.59" drill="3.2"/>
<hole x="10.16" y="-21.59" drill="3.2"/>
<hole x="-10.16" y="-21.59" drill="3.2"/>
<text x="-10.16" y="17.78" size="0.8128" layer="51">AD0</text>
<text x="-10.16" y="15.24" size="0.8128" layer="51">RSV</text>
<text x="-10.16" y="12.7" size="0.8128" layer="51">RSV</text>
<text x="-10.16" y="10.16" size="0.8128" layer="51">SD3</text>
<text x="-10.16" y="7.62" size="0.8128" layer="51">SD2</text>
<text x="-10.16" y="5.08" size="0.8128" layer="51">SD1</text>
<text x="-10.16" y="2.54" size="0.8128" layer="51">CMD</text>
<text x="-10.16" y="0" size="0.8128" layer="51">SD0</text>
<text x="-10.16" y="-2.54" size="0.8128" layer="51">CLK</text>
<text x="-10.16" y="-5.08" size="0.8128" layer="51">GND</text>
<text x="-10.16" y="-7.62" size="0.8128" layer="51">3V3</text>
<text x="-10.16" y="-10.16" size="0.8128" layer="51">EN</text>
<text x="-10.16" y="-12.7" size="0.8128" layer="51">RST</text>
<text x="-10.16" y="-15.24" size="0.8128" layer="51">GND</text>
<text x="-10.16" y="-17.78" size="0.8128" layer="51">VIN</text>
<text x="10.16" y="-17.78" size="0.8128" layer="51" align="bottom-right">3V3</text>
<text x="10.16" y="-15.24" size="0.8128" layer="51" align="bottom-right">GND</text>
<text x="10.16" y="-12.7" size="0.8128" layer="51" align="bottom-right">TX</text>
<text x="10.16" y="-10.16" size="0.8128" layer="51" align="bottom-right">RX</text>
<text x="10.16" y="-7.62" size="0.8128" layer="51" align="bottom-right">~D8</text>
<text x="10.16" y="-5.08" size="0.8128" layer="51" align="bottom-right">~D7</text>
<text x="10.16" y="-2.54" size="0.8128" layer="51" align="bottom-right">~D6</text>
<text x="10.16" y="0" size="0.8128" layer="51" align="bottom-right">~D5</text>
<text x="10.16" y="2.54" size="0.8128" layer="51" align="bottom-right">GND</text>
<text x="10.16" y="5.08" size="0.8128" layer="51" align="bottom-right">3V3</text>
<text x="10.16" y="7.62" size="0.8128" layer="51" align="bottom-right">~D4</text>
<text x="10.16" y="10.16" size="0.8128" layer="51" align="bottom-right">~D3</text>
<text x="10.16" y="12.7" size="0.8128" layer="51" align="bottom-right">~D2</text>
<text x="10.16" y="15.24" size="0.8128" layer="51" align="bottom-right">~D1</text>
<text x="10.16" y="17.78" size="0.8128" layer="51" align="bottom-right">D0</text>
<text x="-2.54" y="-19.05" size="1.778" layer="25" rot="R90">&gt;NAME</text>
<text x="3.81" y="-19.05" size="1.778" layer="27" rot="R90">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="ESP12E_DEVKIT">
<description>ESP12E_DEVKIT</description>
<pin name="AD0" x="-17.78" y="17.78" visible="pin" length="middle"/>
<pin name="RSV@0" x="-17.78" y="15.24" visible="pin" length="middle"/>
<pin name="RSV@1" x="-17.78" y="12.7" visible="pin" length="middle"/>
<pin name="SD3" x="-17.78" y="10.16" visible="pin" length="middle"/>
<pin name="SD2" x="-17.78" y="7.62" visible="pin" length="middle"/>
<pin name="SD1" x="-17.78" y="5.08" visible="pin" length="middle"/>
<pin name="CMD" x="-17.78" y="2.54" visible="pin" length="middle"/>
<pin name="SD0" x="-17.78" y="0" visible="pin" length="middle"/>
<pin name="CLK" x="-17.78" y="-2.54" visible="pin" length="middle"/>
<pin name="GND@0" x="-17.78" y="-5.08" visible="pin" length="middle"/>
<pin name="3V3@0" x="-17.78" y="-7.62" visible="pin" length="middle"/>
<pin name="EN" x="-17.78" y="-10.16" visible="pin" length="middle"/>
<pin name="RST" x="-17.78" y="-12.7" visible="pin" length="middle"/>
<pin name="GND@1" x="-17.78" y="-15.24" visible="pin" length="middle"/>
<pin name="VIN" x="-17.78" y="-17.78" visible="pin" length="middle"/>
<pin name="D0" x="17.78" y="17.78" visible="pin" length="middle" rot="R180"/>
<pin name="D1" x="17.78" y="15.24" visible="pin" length="middle" rot="R180"/>
<pin name="D2" x="17.78" y="12.7" visible="pin" length="middle" rot="R180"/>
<pin name="D3" x="17.78" y="10.16" visible="pin" length="middle" rot="R180"/>
<pin name="D4" x="17.78" y="7.62" visible="pin" length="middle" rot="R180"/>
<pin name="3V3@1" x="17.78" y="5.08" visible="pin" length="middle" rot="R180"/>
<pin name="GND@2" x="17.78" y="2.54" visible="pin" length="middle" rot="R180"/>
<pin name="D5" x="17.78" y="0" visible="pin" length="middle" rot="R180"/>
<pin name="D6" x="17.78" y="-2.54" visible="pin" length="middle" rot="R180"/>
<pin name="D7" x="17.78" y="-5.08" visible="pin" length="middle" rot="R180"/>
<pin name="D8" x="17.78" y="-7.62" visible="pin" length="middle" rot="R180"/>
<pin name="RX" x="17.78" y="-10.16" visible="pin" length="middle" rot="R180"/>
<pin name="TX" x="17.78" y="-12.7" visible="pin" length="middle" rot="R180"/>
<pin name="GND@3" x="17.78" y="-15.24" visible="pin" length="middle" rot="R180"/>
<pin name="3V3@2" x="17.78" y="-17.78" visible="pin" length="middle" rot="R180"/>
<wire x1="-12.7" y1="20.32" x2="-12.7" y2="-20.32" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-20.32" x2="12.7" y2="-20.32" width="0.254" layer="94"/>
<wire x1="12.7" y1="-20.32" x2="12.7" y2="20.32" width="0.254" layer="94"/>
<wire x1="12.7" y1="20.32" x2="-12.7" y2="20.32" width="0.254" layer="94"/>
<text x="-12.7" y="20.32" size="1.778" layer="95">&gt;NAME</text>
<text x="-12.7" y="-22.86" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="ESP12E_DEVKIT">
<description>ESP12E DEVKIT BOARD</description>
<gates>
<gate name="G$1" symbol="ESP12E_DEVKIT" x="0" y="0"/>
</gates>
<devices>
<device name="" package="ESP12E_DEVKIT">
<connects>
<connect gate="G$1" pin="3V3@0" pad="3V3@0"/>
<connect gate="G$1" pin="3V3@1" pad="3V3@1"/>
<connect gate="G$1" pin="3V3@2" pad="3V3@2"/>
<connect gate="G$1" pin="AD0" pad="AD0"/>
<connect gate="G$1" pin="CLK" pad="CLK"/>
<connect gate="G$1" pin="CMD" pad="CMD"/>
<connect gate="G$1" pin="D0" pad="D0"/>
<connect gate="G$1" pin="D1" pad="D1"/>
<connect gate="G$1" pin="D2" pad="D2"/>
<connect gate="G$1" pin="D3" pad="D3"/>
<connect gate="G$1" pin="D4" pad="D4"/>
<connect gate="G$1" pin="D5" pad="D5"/>
<connect gate="G$1" pin="D6" pad="D6"/>
<connect gate="G$1" pin="D7" pad="D7"/>
<connect gate="G$1" pin="D8" pad="D8"/>
<connect gate="G$1" pin="EN" pad="EN"/>
<connect gate="G$1" pin="GND@0" pad="GND@0"/>
<connect gate="G$1" pin="GND@1" pad="GND@1"/>
<connect gate="G$1" pin="GND@2" pad="GND@2"/>
<connect gate="G$1" pin="GND@3" pad="GND@3"/>
<connect gate="G$1" pin="RST" pad="RST"/>
<connect gate="G$1" pin="RSV@0" pad="RSV@0"/>
<connect gate="G$1" pin="RSV@1" pad="RSV@1"/>
<connect gate="G$1" pin="RX" pad="RX"/>
<connect gate="G$1" pin="SD0" pad="SD0"/>
<connect gate="G$1" pin="SD1" pad="SD1"/>
<connect gate="G$1" pin="SD2" pad="SD2"/>
<connect gate="G$1" pin="SD3" pad="SD3"/>
<connect gate="G$1" pin="TX" pad="TX"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="microchip" urn="urn:adsk.eagle:library:294">
<description>&lt;b&gt;Microchip PIC Microcontrollers and other Devices&lt;/b&gt;&lt;p&gt;
Based on the following sources :
&lt;ul&gt;
&lt;li&gt;Microchip Data Book, 1993
&lt;li&gt;THE EMERGING WORLD STANDARD, 1995/1996
&lt;li&gt;Microchip, Technical Library CD-ROM, June 1998
&lt;li&gt;www.microchip.com
&lt;/ul&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="DIL28-3" urn="urn:adsk.eagle:footprint:20609/1" library_version="2">
<description>&lt;B&gt;Dual In Line&lt;/B&gt;&lt;p&gt;
package type P</description>
<wire x1="-17.78" y1="-1.27" x2="-17.78" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-17.78" y1="1.27" x2="-17.78" y2="-1.27" width="0.1524" layer="21" curve="-180"/>
<wire x1="17.78" y1="-2.54" x2="17.78" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-17.78" y1="2.54" x2="-17.78" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-17.78" y1="2.54" x2="17.78" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-17.653" y1="-2.54" x2="17.78" y2="-2.54" width="0.1524" layer="21"/>
<pad name="1" x="-16.51" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="2" x="-13.97" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="3" x="-11.43" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="4" x="-8.89" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="5" x="-6.35" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="6" x="-3.81" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="7" x="-1.27" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="8" x="1.27" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="9" x="3.81" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="10" x="6.35" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="11" x="8.89" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="12" x="11.43" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="13" x="13.97" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="14" x="16.51" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="15" x="16.51" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="16" x="13.97" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="17" x="11.43" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="18" x="8.89" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="19" x="6.35" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="20" x="3.81" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="21" x="1.27" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="22" x="-1.27" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="23" x="-3.81" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="24" x="-6.35" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="25" x="-8.89" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="26" x="-11.43" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="27" x="-13.97" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="28" x="-16.51" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<text x="-17.907" y="-2.54" size="1.778" layer="25" ratio="10" rot="R90">&gt;NAME</text>
<text x="-15.748" y="-0.9398" size="1.778" layer="27" ratio="10">&gt;VALUE</text>
</package>
<package name="SO28W" urn="urn:adsk.eagle:footprint:20598/1" library_version="2">
<description>&lt;B&gt;28-Lead Plastic Small Outline (SO) &lt;/B&gt; Wide, 300 mil Body (SOIC)&lt;/B&gt;&lt;p&gt;
Source: http://ww1.microchip.com/downloads/en/devicedoc/39632c.pdf</description>
<wire x1="-8.1788" y1="-3.7132" x2="9.4742" y2="-3.7132" width="0.1524" layer="21"/>
<wire x1="9.4742" y1="-3.7132" x2="9.4742" y2="3.7132" width="0.1524" layer="21"/>
<wire x1="9.4742" y1="3.7132" x2="-8.1788" y2="3.7132" width="0.1524" layer="21"/>
<wire x1="-8.1788" y1="3.7132" x2="-8.1788" y2="-3.7132" width="0.1524" layer="21"/>
<circle x="-7.239" y="-3.1496" radius="0.5334" width="0.1524" layer="21"/>
<smd name="1" x="-7.62" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="2" x="-6.35" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="3" x="-5.08" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="4" x="-3.81" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="5" x="-2.54" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="6" x="-1.27" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="7" x="0" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="8" x="1.27" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="9" x="2.54" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="10" x="3.81" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="20" x="2.54" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="19" x="3.81" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="18" x="5.08" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="17" x="6.35" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="16" x="7.62" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="15" x="8.89" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="14" x="8.89" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="13" x="7.62" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="12" x="6.35" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="11" x="5.08" y="-4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="21" x="1.27" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="22" x="0" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="23" x="-1.27" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="24" x="-2.54" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="25" x="-3.81" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="26" x="-5.08" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="27" x="-6.35" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<smd name="28" x="-7.62" y="4.78" dx="0.65" dy="1.7" layer="1"/>
<text x="-8.509" y="-4.064" size="1.778" layer="25" ratio="10" rot="R90">&gt;NAME</text>
<text x="11.557" y="-4.064" size="1.778" layer="27" ratio="10" rot="R90">&gt;VALUE</text>
<rectangle x1="-7.874" y1="-5.2626" x2="-7.366" y2="-3.7386" layer="51"/>
<rectangle x1="-6.604" y1="-5.2626" x2="-6.096" y2="-3.7386" layer="51"/>
<rectangle x1="-5.334" y1="-5.2626" x2="-4.826" y2="-3.7386" layer="51"/>
<rectangle x1="-4.064" y1="-5.2626" x2="-3.556" y2="-3.7386" layer="51"/>
<rectangle x1="-2.794" y1="-5.2626" x2="-2.286" y2="-3.7386" layer="51"/>
<rectangle x1="-1.524" y1="-5.2626" x2="-1.016" y2="-3.7386" layer="51"/>
<rectangle x1="-0.254" y1="-5.2626" x2="0.254" y2="-3.7386" layer="51"/>
<rectangle x1="1.016" y1="-5.2626" x2="1.524" y2="-3.7386" layer="51"/>
<rectangle x1="2.286" y1="-5.2626" x2="2.794" y2="-3.7386" layer="51"/>
<rectangle x1="3.556" y1="-5.2626" x2="4.064" y2="-3.7386" layer="51"/>
<rectangle x1="4.826" y1="-5.2626" x2="5.334" y2="-3.7386" layer="51"/>
<rectangle x1="6.096" y1="-5.2626" x2="6.604" y2="-3.7386" layer="51"/>
<rectangle x1="7.366" y1="-5.2626" x2="7.874" y2="-3.7386" layer="51"/>
<rectangle x1="8.636" y1="-5.2626" x2="9.144" y2="-3.7386" layer="51"/>
<rectangle x1="8.636" y1="3.7386" x2="9.144" y2="5.2626" layer="51"/>
<rectangle x1="7.366" y1="3.7386" x2="7.874" y2="5.2626" layer="51"/>
<rectangle x1="6.096" y1="3.7386" x2="6.604" y2="5.2626" layer="51"/>
<rectangle x1="4.826" y1="3.7386" x2="5.334" y2="5.2626" layer="51"/>
<rectangle x1="3.556" y1="3.7386" x2="4.064" y2="5.2626" layer="51"/>
<rectangle x1="2.286" y1="3.7386" x2="2.794" y2="5.2626" layer="51"/>
<rectangle x1="1.016" y1="3.7386" x2="1.524" y2="5.2626" layer="51"/>
<rectangle x1="-0.254" y1="3.7386" x2="0.254" y2="5.2626" layer="51"/>
<rectangle x1="-1.524" y1="3.7386" x2="-1.016" y2="5.2626" layer="51"/>
<rectangle x1="-2.794" y1="3.7386" x2="-2.286" y2="5.2626" layer="51"/>
<rectangle x1="-4.064" y1="3.7386" x2="-3.556" y2="5.2626" layer="51"/>
<rectangle x1="-5.334" y1="3.7386" x2="-4.826" y2="5.2626" layer="51"/>
<rectangle x1="-6.604" y1="3.7386" x2="-6.096" y2="5.2626" layer="51"/>
<rectangle x1="-7.874" y1="3.7386" x2="-7.366" y2="5.2626" layer="51"/>
</package>
<package name="SSOP28" urn="urn:adsk.eagle:footprint:20599/1" library_version="2">
<description>&lt;b&gt;Shrink Small Outline Package&lt;/b&gt;&lt;p&gt;
package type SS</description>
<wire x1="-5.1" y1="-2.6" x2="5.1" y2="-2.6" width="0.2032" layer="21"/>
<wire x1="5.1" y1="-2.6" x2="5.1" y2="2.6" width="0.2032" layer="21"/>
<wire x1="5.1" y1="2.6" x2="-5.1" y2="2.6" width="0.2032" layer="21"/>
<smd name="1" x="-4.225" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="2" x="-3.575" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="3" x="-2.925" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="4" x="-2.275" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="5" x="-1.625" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="6" x="-0.975" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="7" x="-0.325" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="8" x="0.325" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="9" x="0.975" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="10" x="1.625" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="11" x="2.275" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="12" x="2.925" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="13" x="3.575" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="14" x="4.225" y="-3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="15" x="4.225" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="16" x="3.575" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="17" x="2.925" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="18" x="2.275" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="19" x="1.625" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="20" x="0.975" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="21" x="0.325" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="22" x="-0.325" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="23" x="-0.975" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="24" x="-1.625" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="25" x="-2.275" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="26" x="-2.925" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="27" x="-3.575" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<smd name="28" x="-4.225" y="3.625" dx="0.4" dy="1.5" layer="1"/>
<text x="-5.476" y="-2.6299" size="1.27" layer="25" rot="R90">&gt;NAME</text>
<text x="-3.8999" y="-0.68" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-4.4028" y1="-3.937" x2="-4.0472" y2="-2.6416" layer="51"/>
<rectangle x1="-3.7529" y1="-3.937" x2="-3.3973" y2="-2.6416" layer="51"/>
<rectangle x1="-3.1029" y1="-3.937" x2="-2.7473" y2="-2.6416" layer="51"/>
<rectangle x1="-2.4529" y1="-3.937" x2="-2.0973" y2="-2.6416" layer="51"/>
<rectangle x1="-1.8029" y1="-3.937" x2="-1.4473" y2="-2.6416" layer="51"/>
<rectangle x1="-1.1529" y1="-3.937" x2="-0.7973" y2="-2.6416" layer="51"/>
<rectangle x1="-0.5029" y1="-3.937" x2="-0.1473" y2="-2.6416" layer="51"/>
<rectangle x1="0.1473" y1="-3.937" x2="0.5029" y2="-2.6416" layer="51"/>
<rectangle x1="0.7973" y1="-3.937" x2="1.1529" y2="-2.6416" layer="51"/>
<rectangle x1="1.4473" y1="-3.937" x2="1.8029" y2="-2.6416" layer="51"/>
<rectangle x1="2.0973" y1="-3.937" x2="2.4529" y2="-2.6416" layer="51"/>
<rectangle x1="2.7473" y1="-3.937" x2="3.1029" y2="-2.6416" layer="51"/>
<rectangle x1="3.3973" y1="-3.937" x2="3.7529" y2="-2.6416" layer="51"/>
<rectangle x1="4.0472" y1="-3.937" x2="4.4028" y2="-2.6416" layer="51"/>
<rectangle x1="4.0472" y1="2.6416" x2="4.4028" y2="3.937" layer="51"/>
<rectangle x1="3.3973" y1="2.6416" x2="3.7529" y2="3.937" layer="51"/>
<rectangle x1="2.7473" y1="2.6416" x2="3.1029" y2="3.937" layer="51"/>
<rectangle x1="2.0973" y1="2.6416" x2="2.4529" y2="3.937" layer="51"/>
<rectangle x1="1.4473" y1="2.6416" x2="1.8029" y2="3.937" layer="51"/>
<rectangle x1="0.7973" y1="2.6416" x2="1.1529" y2="3.937" layer="51"/>
<rectangle x1="0.1473" y1="2.6416" x2="0.5029" y2="3.937" layer="51"/>
<rectangle x1="-0.5029" y1="2.6416" x2="-0.1473" y2="3.937" layer="51"/>
<rectangle x1="-1.1529" y1="2.6416" x2="-0.7973" y2="3.937" layer="51"/>
<rectangle x1="-1.8029" y1="2.6416" x2="-1.4473" y2="3.937" layer="51"/>
<rectangle x1="-2.4529" y1="2.6416" x2="-2.0973" y2="3.937" layer="51"/>
<rectangle x1="-3.1029" y1="2.6416" x2="-2.7473" y2="3.937" layer="51"/>
<rectangle x1="-3.7529" y1="2.6416" x2="-3.3973" y2="3.937" layer="51"/>
<rectangle x1="-4.4028" y1="2.6416" x2="-4.0472" y2="3.937" layer="51"/>
<rectangle x1="-5.1999" y1="-2.5999" x2="-4.225" y2="2.5999" layer="27"/>
</package>
<package name="QFN28-ML_6X6MM" urn="urn:adsk.eagle:footprint:20673/1" library_version="2">
<description>&lt;b&gt;QFN28-ML_6X6MM&lt;/b&gt;&lt;p&gt;
Source: http://www.microchip.com .. 39637a.pdf</description>
<wire x1="-2.8984" y1="-2.8984" x2="2.8984" y2="-2.8984" width="0.2032" layer="51"/>
<wire x1="2.8984" y1="-2.8984" x2="2.8984" y2="2.8984" width="0.2032" layer="51"/>
<wire x1="2.8984" y1="2.8984" x2="-2.22" y2="2.8984" width="0.2032" layer="51"/>
<wire x1="-2.22" y1="2.8984" x2="-2.22" y2="2.9" width="0.2032" layer="21"/>
<wire x1="-2.8984" y1="2.8984" x2="-2.22" y2="2.8984" width="0.2032" layer="21"/>
<wire x1="-2.22" y1="2.9" x2="-2.8984" y2="2.2216" width="0.2032" layer="21"/>
<wire x1="-2.8984" y1="2.2216" x2="-2.8984" y2="-2.8984" width="0.2032" layer="51"/>
<wire x1="-2.8984" y1="2.2216" x2="-2.8984" y2="2.8984" width="0.2032" layer="21"/>
<smd name="1" x="-2.7" y="1.95" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="2" x="-2.7" y="1.3" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="3" x="-2.7" y="0.65" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="4" x="-2.7" y="0" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="5" x="-2.7" y="-0.65" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="6" x="-2.7" y="-1.3" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="7" x="-2.7" y="-1.95" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="8" x="-1.95" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="9" x="-1.3" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="10" x="-0.65" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="11" x="0" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="12" x="0.65" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="13" x="1.3" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="14" x="1.95" y="-2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="15" x="2.7" y="-1.95" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="16" x="2.7" y="-1.3" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="17" x="2.7" y="-0.65" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="18" x="2.7" y="0" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="19" x="2.7" y="0.65" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="20" x="2.7" y="1.3" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="21" x="2.7" y="1.95" dx="0.7" dy="0.35" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="22" x="1.95" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="23" x="1.3" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="24" x="0.65" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="25" x="0" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="26" x="-0.65" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="27" x="-1.3" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="28" x="-1.95" y="2.7" dx="0.35" dy="0.7" layer="1" roundness="20" stop="no" cream="no"/>
<smd name="EXP" x="0" y="0" dx="3.7" dy="3.7" layer="1" roundness="20" stop="no" cream="no"/>
<text x="-3.175" y="3.175" size="1.27" layer="25">&gt;NAME</text>
<text x="-3.175" y="-4.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.055" y1="1.768" x2="-2.3465" y2="2.132" layer="29"/>
<rectangle x1="-3.042" y1="1.7875" x2="-2.3595" y2="2.1125" layer="31"/>
<rectangle x1="-3.055" y1="1.118" x2="-2.3465" y2="1.482" layer="29"/>
<rectangle x1="-3.042" y1="1.1375" x2="-2.3595" y2="1.4625" layer="31"/>
<rectangle x1="-3.055" y1="0.468" x2="-2.3465" y2="0.832" layer="29"/>
<rectangle x1="-3.042" y1="0.4875" x2="-2.3595" y2="0.8125" layer="31"/>
<rectangle x1="-3.055" y1="-0.182" x2="-2.3465" y2="0.182" layer="29"/>
<rectangle x1="-3.042" y1="-0.1625" x2="-2.3595" y2="0.1625" layer="31"/>
<rectangle x1="-3.055" y1="-0.832" x2="-2.3465" y2="-0.468" layer="29"/>
<rectangle x1="-3.042" y1="-0.8125" x2="-2.3595" y2="-0.4875" layer="31"/>
<rectangle x1="-3.055" y1="-1.482" x2="-2.3465" y2="-1.118" layer="29"/>
<rectangle x1="-3.042" y1="-1.4625" x2="-2.3595" y2="-1.1375" layer="31"/>
<rectangle x1="-3.055" y1="-2.132" x2="-2.3465" y2="-1.768" layer="29"/>
<rectangle x1="-3.042" y1="-2.1125" x2="-2.3595" y2="-1.7875" layer="31"/>
<rectangle x1="-2.3042" y1="-2.8827" x2="-1.5958" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="-2.2912" y1="-2.8632" x2="-1.6088" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="-1.6542" y1="-2.8827" x2="-0.9458" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="-1.6412" y1="-2.8632" x2="-0.9588" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="-1.0042" y1="-2.8827" x2="-0.2958" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="-0.9912" y1="-2.8632" x2="-0.3088" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="-0.3542" y1="-2.8827" x2="0.3542" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="-0.3412" y1="-2.8632" x2="0.3412" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="0.2958" y1="-2.8827" x2="1.0042" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="0.3088" y1="-2.8632" x2="0.9912" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="0.9458" y1="-2.8827" x2="1.6542" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="0.9588" y1="-2.8632" x2="1.6412" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="1.5958" y1="-2.8827" x2="2.3042" y2="-2.5187" layer="29" rot="R90"/>
<rectangle x1="1.6088" y1="-2.8632" x2="2.2912" y2="-2.5382" layer="31" rot="R90"/>
<rectangle x1="2.3465" y1="-2.132" x2="3.0549" y2="-1.768" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="-2.1125" x2="3.0419" y2="-1.7875" layer="31" rot="R180"/>
<rectangle x1="2.3465" y1="-1.482" x2="3.0549" y2="-1.118" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="-1.4625" x2="3.0419" y2="-1.1375" layer="31" rot="R180"/>
<rectangle x1="2.3465" y1="-0.832" x2="3.0549" y2="-0.468" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="-0.8125" x2="3.0419" y2="-0.4875" layer="31" rot="R180"/>
<rectangle x1="2.3465" y1="-0.182" x2="3.0549" y2="0.182" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="-0.1625" x2="3.0419" y2="0.1625" layer="31" rot="R180"/>
<rectangle x1="2.3465" y1="0.468" x2="3.0549" y2="0.832" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="0.4875" x2="3.0419" y2="0.8125" layer="31" rot="R180"/>
<rectangle x1="2.3465" y1="1.118" x2="3.0549" y2="1.482" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="1.1375" x2="3.0419" y2="1.4625" layer="31" rot="R180"/>
<rectangle x1="2.3465" y1="1.768" x2="3.0549" y2="2.132" layer="29" rot="R180"/>
<rectangle x1="2.3595" y1="1.7875" x2="3.0419" y2="2.1125" layer="31" rot="R180"/>
<rectangle x1="1.5958" y1="2.5187" x2="2.3042" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="1.6088" y1="2.5382" x2="2.2912" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="0.9458" y1="2.5187" x2="1.6542" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="0.9588" y1="2.5382" x2="1.6412" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="0.2958" y1="2.5187" x2="1.0042" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="0.3088" y1="2.5382" x2="0.9912" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="-0.3542" y1="2.5187" x2="0.3542" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="-0.3412" y1="2.5382" x2="0.3412" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="-1.0042" y1="2.5187" x2="-0.2958" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="-0.9912" y1="2.5382" x2="-0.3088" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="-1.6542" y1="2.5187" x2="-0.9458" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="-1.6412" y1="2.5382" x2="-0.9588" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="-2.3042" y1="2.5187" x2="-1.5958" y2="2.8827" layer="29" rot="R270"/>
<rectangle x1="-2.2912" y1="2.5382" x2="-1.6088" y2="2.8632" layer="31" rot="R270"/>
<rectangle x1="-1.859" y1="-1.859" x2="1.859" y2="1.859" layer="29"/>
<rectangle x1="-1.7355" y1="-1.7355" x2="1.7355" y2="1.7355" layer="31"/>
</package>
</packages>
<packages3d>
<package3d name="DIL28-3" urn="urn:adsk.eagle:package:20799/2" type="model" library_version="2">
<description>Dual In Line
package type P</description>
</package3d>
<package3d name="SO28W" urn="urn:adsk.eagle:package:20803/2" type="model" library_version="2">
<description>28-Lead Plastic Small Outline (SO)  Wide, 300 mil Body (SOIC)
Source: http://ww1.microchip.com/downloads/en/devicedoc/39632c.pdf</description>
</package3d>
<package3d name="SSOP28" urn="urn:adsk.eagle:package:20800/1" type="box" library_version="2">
<description>Shrink Small Outline Package
package type SS</description>
</package3d>
<package3d name="QFN28-ML_6X6MM" urn="urn:adsk.eagle:package:20818/2" type="model" library_version="2">
<description>QFN28-ML_6X6MM
Source: http://www.microchip.com .. 39637a.pdf</description>
</package3d>
</packages3d>
<symbols>
<symbol name="MCP23017" urn="urn:adsk.eagle:symbol:20682/1" library_version="2">
<wire x1="-10.16" y1="22.86" x2="10.16" y2="22.86" width="0.254" layer="94"/>
<wire x1="10.16" y1="22.86" x2="10.16" y2="-22.86" width="0.254" layer="94"/>
<wire x1="10.16" y1="-22.86" x2="-10.16" y2="-22.86" width="0.254" layer="94"/>
<wire x1="-10.16" y1="-22.86" x2="-10.16" y2="22.86" width="0.254" layer="94"/>
<text x="-10.16" y="24.13" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-25.4" size="1.778" layer="96">&gt;VALUE</text>
<pin name="SCL" x="-12.7" y="-2.54" length="short" direction="in"/>
<pin name="SDA" x="-12.7" y="-5.08" length="short"/>
<pin name="A0" x="-12.7" y="-10.16" length="short" direction="in"/>
<pin name="A1" x="-12.7" y="-12.7" length="short" direction="in"/>
<pin name="A2" x="-12.7" y="-15.24" length="short" direction="in"/>
<pin name="!RESET" x="-12.7" y="15.24" length="short" direction="in"/>
<pin name="INTA" x="-12.7" y="10.16" length="short" direction="out"/>
<pin name="INTB" x="-12.7" y="7.62" length="short" direction="out"/>
<pin name="GPB0" x="12.7" y="-2.54" length="short" rot="R180"/>
<pin name="GPB1" x="12.7" y="-5.08" length="short" rot="R180"/>
<pin name="GPB2" x="12.7" y="-7.62" length="short" rot="R180"/>
<pin name="GPB3" x="12.7" y="-10.16" length="short" rot="R180"/>
<pin name="GPB4" x="12.7" y="-12.7" length="short" rot="R180"/>
<pin name="GPB5" x="12.7" y="-15.24" length="short" rot="R180"/>
<pin name="GPB6" x="12.7" y="-17.78" length="short" rot="R180"/>
<pin name="GPB7" x="12.7" y="-20.32" length="short" rot="R180"/>
<pin name="GPA0" x="12.7" y="20.32" length="short" rot="R180"/>
<pin name="GPA1" x="12.7" y="17.78" length="short" rot="R180"/>
<pin name="GPA2" x="12.7" y="15.24" length="short" rot="R180"/>
<pin name="GPA3" x="12.7" y="12.7" length="short" rot="R180"/>
<pin name="GPA4" x="12.7" y="10.16" length="short" rot="R180"/>
<pin name="GPA5" x="12.7" y="7.62" length="short" rot="R180"/>
<pin name="GPA6" x="12.7" y="5.08" length="short" rot="R180"/>
<pin name="GPA7" x="12.7" y="2.54" length="short" rot="R180"/>
<pin name="VDD" x="-12.7" y="20.32" length="short" direction="pwr"/>
<pin name="VSS" x="-12.7" y="-20.32" length="short" direction="pwr"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="MCP23017" urn="urn:adsk.eagle:component:20914/4" prefix="IC" library_version="2">
<description>&lt;b&gt;http://ww1.microchip.com/downloads/en/DeviceDoc/21952a.pdf&lt;/b&gt;&lt;p&gt;
Source: http://ww1.microchip.com/downloads/en/DeviceDoc/21952a.pdf</description>
<gates>
<gate name="G$1" symbol="MCP23017" x="0" y="0"/>
</gates>
<devices>
<device name="SP" package="DIL28-3">
<connects>
<connect gate="G$1" pin="!RESET" pad="18"/>
<connect gate="G$1" pin="A0" pad="15"/>
<connect gate="G$1" pin="A1" pad="16"/>
<connect gate="G$1" pin="A2" pad="17"/>
<connect gate="G$1" pin="GPA0" pad="21"/>
<connect gate="G$1" pin="GPA1" pad="22"/>
<connect gate="G$1" pin="GPA2" pad="23"/>
<connect gate="G$1" pin="GPA3" pad="24"/>
<connect gate="G$1" pin="GPA4" pad="25"/>
<connect gate="G$1" pin="GPA5" pad="26"/>
<connect gate="G$1" pin="GPA6" pad="27"/>
<connect gate="G$1" pin="GPA7" pad="28"/>
<connect gate="G$1" pin="GPB0" pad="1"/>
<connect gate="G$1" pin="GPB1" pad="2"/>
<connect gate="G$1" pin="GPB2" pad="3"/>
<connect gate="G$1" pin="GPB3" pad="4"/>
<connect gate="G$1" pin="GPB4" pad="5"/>
<connect gate="G$1" pin="GPB5" pad="6"/>
<connect gate="G$1" pin="GPB6" pad="7"/>
<connect gate="G$1" pin="GPB7" pad="8"/>
<connect gate="G$1" pin="INTA" pad="20"/>
<connect gate="G$1" pin="INTB" pad="19"/>
<connect gate="G$1" pin="SCL" pad="12"/>
<connect gate="G$1" pin="SDA" pad="13"/>
<connect gate="G$1" pin="VDD" pad="9"/>
<connect gate="G$1" pin="VSS" pad="10"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:20799/2"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="MICROCHIP" constant="no"/>
<attribute name="MPN" value="MCP23017-E/SP" constant="no"/>
<attribute name="OC_FARNELL" value="1332088" constant="no"/>
<attribute name="OC_NEWARK" value="31K2959" constant="no"/>
</technology>
</technologies>
</device>
<device name="SO" package="SO28W">
<connects>
<connect gate="G$1" pin="!RESET" pad="18"/>
<connect gate="G$1" pin="A0" pad="15"/>
<connect gate="G$1" pin="A1" pad="16"/>
<connect gate="G$1" pin="A2" pad="17"/>
<connect gate="G$1" pin="GPA0" pad="21"/>
<connect gate="G$1" pin="GPA1" pad="22"/>
<connect gate="G$1" pin="GPA2" pad="23"/>
<connect gate="G$1" pin="GPA3" pad="24"/>
<connect gate="G$1" pin="GPA4" pad="25"/>
<connect gate="G$1" pin="GPA5" pad="26"/>
<connect gate="G$1" pin="GPA6" pad="27"/>
<connect gate="G$1" pin="GPA7" pad="28"/>
<connect gate="G$1" pin="GPB0" pad="1"/>
<connect gate="G$1" pin="GPB1" pad="2"/>
<connect gate="G$1" pin="GPB2" pad="3"/>
<connect gate="G$1" pin="GPB3" pad="4"/>
<connect gate="G$1" pin="GPB4" pad="5"/>
<connect gate="G$1" pin="GPB5" pad="6"/>
<connect gate="G$1" pin="GPB6" pad="7"/>
<connect gate="G$1" pin="GPB7" pad="8"/>
<connect gate="G$1" pin="INTA" pad="20"/>
<connect gate="G$1" pin="INTB" pad="19"/>
<connect gate="G$1" pin="SCL" pad="12"/>
<connect gate="G$1" pin="SDA" pad="13"/>
<connect gate="G$1" pin="VDD" pad="9"/>
<connect gate="G$1" pin="VSS" pad="10"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:20803/2"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="MICROCHIP" constant="no"/>
<attribute name="MPN" value="MCP23017-E/SO" constant="no"/>
<attribute name="OC_FARNELL" value="1332087" constant="no"/>
<attribute name="OC_NEWARK" value="31K2958" constant="no"/>
</technology>
</technologies>
</device>
<device name="SS" package="SSOP28">
<connects>
<connect gate="G$1" pin="!RESET" pad="18"/>
<connect gate="G$1" pin="A0" pad="15"/>
<connect gate="G$1" pin="A1" pad="16"/>
<connect gate="G$1" pin="A2" pad="17"/>
<connect gate="G$1" pin="GPA0" pad="21"/>
<connect gate="G$1" pin="GPA1" pad="22"/>
<connect gate="G$1" pin="GPA2" pad="23"/>
<connect gate="G$1" pin="GPA3" pad="24"/>
<connect gate="G$1" pin="GPA4" pad="25"/>
<connect gate="G$1" pin="GPA5" pad="26"/>
<connect gate="G$1" pin="GPA6" pad="27"/>
<connect gate="G$1" pin="GPA7" pad="28"/>
<connect gate="G$1" pin="GPB0" pad="1"/>
<connect gate="G$1" pin="GPB1" pad="2"/>
<connect gate="G$1" pin="GPB2" pad="3"/>
<connect gate="G$1" pin="GPB3" pad="4"/>
<connect gate="G$1" pin="GPB4" pad="5"/>
<connect gate="G$1" pin="GPB5" pad="6"/>
<connect gate="G$1" pin="GPB6" pad="7"/>
<connect gate="G$1" pin="GPB7" pad="8"/>
<connect gate="G$1" pin="INTA" pad="20"/>
<connect gate="G$1" pin="INTB" pad="19"/>
<connect gate="G$1" pin="SCL" pad="12"/>
<connect gate="G$1" pin="SDA" pad="13"/>
<connect gate="G$1" pin="VDD" pad="9"/>
<connect gate="G$1" pin="VSS" pad="10"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:20800/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="MICROCHIP" constant="no"/>
<attribute name="MPN" value="MCP23017-E/SS" constant="no"/>
<attribute name="OC_FARNELL" value="1467674" constant="no"/>
<attribute name="OC_NEWARK" value="31K2960" constant="no"/>
</technology>
</technologies>
</device>
<device name="ML" package="QFN28-ML_6X6MM">
<connects>
<connect gate="G$1" pin="!RESET" pad="14"/>
<connect gate="G$1" pin="A0" pad="11"/>
<connect gate="G$1" pin="A1" pad="12"/>
<connect gate="G$1" pin="A2" pad="13"/>
<connect gate="G$1" pin="GPA0" pad="17"/>
<connect gate="G$1" pin="GPA1" pad="18"/>
<connect gate="G$1" pin="GPA2" pad="19"/>
<connect gate="G$1" pin="GPA3" pad="20"/>
<connect gate="G$1" pin="GPA4" pad="21"/>
<connect gate="G$1" pin="GPA5" pad="22"/>
<connect gate="G$1" pin="GPA6" pad="23"/>
<connect gate="G$1" pin="GPA7" pad="24"/>
<connect gate="G$1" pin="GPB0" pad="25"/>
<connect gate="G$1" pin="GPB1" pad="26"/>
<connect gate="G$1" pin="GPB2" pad="27"/>
<connect gate="G$1" pin="GPB3" pad="28"/>
<connect gate="G$1" pin="GPB4" pad="1"/>
<connect gate="G$1" pin="GPB5" pad="2"/>
<connect gate="G$1" pin="GPB6" pad="3"/>
<connect gate="G$1" pin="GPB7" pad="4"/>
<connect gate="G$1" pin="INTA" pad="16"/>
<connect gate="G$1" pin="INTB" pad="15"/>
<connect gate="G$1" pin="SCL" pad="8"/>
<connect gate="G$1" pin="SDA" pad="9"/>
<connect gate="G$1" pin="VDD" pad="5"/>
<connect gate="G$1" pin="VSS" pad="6"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:20818/2"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="MICROCHIP" constant="no"/>
<attribute name="MPN" value="MCP23017-E/ML" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="31K2957" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="transistor-npn" urn="urn:adsk.eagle:library:398">
<description>&lt;b&gt;NPN Transistors&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="TO92-EBC" urn="urn:adsk.eagle:footprint:28938/1" library_version="2">
<description>&lt;b&gt;TO-92&lt;/b&gt;&lt;p&gt;
 grid 5.08 mm</description>
<wire x1="-2.095" y1="-2.921" x2="2.095" y2="-2.921" width="0.127" layer="21"/>
<wire x1="-2.413" y1="-0.1341" x2="2.413" y2="-0.1341" width="0.127" layer="21" curve="-129.583"/>
<wire x1="1.136" y1="-1.397" x2="-1.136" y2="-1.397" width="0.127" layer="51"/>
<wire x1="-2.413" y1="-0.1341" x2="-2.664" y2="-1.397" width="0.127" layer="51" curve="27.9376"/>
<wire x1="-2.664" y1="-1.397" x2="-2.413" y2="-2.4059" width="0.127" layer="51" curve="22.4788"/>
<wire x1="-1.404" y1="-1.397" x2="-2.664" y2="-1.397" width="0.127" layer="51"/>
<wire x1="-2.4135" y1="-2.4059" x2="-2.095" y2="-2.921" width="0.127" layer="21" curve="13.0385"/>
<wire x1="-1.136" y1="-1.397" x2="-1.404" y2="-1.397" width="0.127" layer="21"/>
<wire x1="2.413" y1="-2.4059" x2="2.664" y2="-1.397" width="0.127" layer="51" curve="22.4788"/>
<wire x1="2.664" y1="-1.397" x2="2.413" y2="-0.1341" width="0.127" layer="51" curve="27.9376"/>
<wire x1="2.664" y1="-1.397" x2="1.404" y2="-1.397" width="0.127" layer="51"/>
<wire x1="1.404" y1="-1.397" x2="1.136" y2="-1.397" width="0.127" layer="21"/>
<wire x1="2.095" y1="-2.921" x2="2.4247" y2="-2.3818" width="0.127" layer="21" curve="13.6094"/>
<pad name="C" x="-2.54" y="-1.27" drill="0.8128" shape="long" rot="R90"/>
<pad name="E" x="2.54" y="-1.27" drill="0.8128" shape="long" rot="R90"/>
<pad name="B" x="0" y="-1.27" drill="0.8128" shape="long" rot="R90"/>
<text x="-2.54" y="1.778" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-2.54" y="-4.572" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
</package>
</packages>
<packages3d>
<package3d name="TO92-EBC" urn="urn:adsk.eagle:package:28989/2" type="model" library_version="2">
<description>TO-92
 grid 5.08 mm</description>
</package3d>
</packages3d>
<symbols>
<symbol name="NPN" urn="urn:adsk.eagle:symbol:28915/1" library_version="2">
<wire x1="2.54" y1="2.54" x2="0.508" y2="1.524" width="0.1524" layer="94"/>
<wire x1="1.778" y1="-1.524" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="1.27" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="1.27" y1="-2.54" x2="1.778" y2="-1.524" width="0.1524" layer="94"/>
<wire x1="1.54" y1="-2.04" x2="0.308" y2="-1.424" width="0.1524" layer="94"/>
<wire x1="1.524" y1="-2.413" x2="2.286" y2="-2.413" width="0.254" layer="94"/>
<wire x1="2.286" y1="-2.413" x2="1.778" y2="-1.778" width="0.254" layer="94"/>
<wire x1="1.778" y1="-1.778" x2="1.524" y2="-2.286" width="0.254" layer="94"/>
<wire x1="1.524" y1="-2.286" x2="1.905" y2="-2.286" width="0.254" layer="94"/>
<wire x1="1.905" y1="-2.286" x2="1.778" y2="-2.032" width="0.254" layer="94"/>
<text x="-10.16" y="7.62" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="5.08" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-0.254" y1="-2.54" x2="0.508" y2="2.54" layer="94"/>
<pin name="B" x="-2.54" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
<pin name="E" x="2.54" y="-5.08" visible="off" length="short" direction="pas" swaplevel="3" rot="R90"/>
<pin name="C" x="2.54" y="5.08" visible="off" length="short" direction="pas" swaplevel="2" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="BC548*" urn="urn:adsk.eagle:component:29116/2" prefix="Q" library_version="2">
<description>&lt;b&gt;NPN Transistor&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="NPN" x="0" y="0"/>
</gates>
<devices>
<device name="" package="TO92-EBC">
<connects>
<connect gate="G$1" pin="B" pad="B"/>
<connect gate="G$1" pin="C" pad="C"/>
<connect gate="G$1" pin="E" pad="E"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:28989/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
<technology name="A"/>
<technology name="B"/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="eagle-ltspice" urn="urn:adsk.eagle:library:217">
<description>Default symbols for import LTspice schematics&lt;p&gt;
2012-10-29 alf@cadsoft.de&lt;br&gt;</description>
<packages>
<package name="0204/7" urn="urn:adsk.eagle:footprint:13215/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;&lt;p&gt;
type 0204, grid 7.5 mm</description>
<wire x1="3.81" y1="0" x2="2.921" y2="0" width="0.508" layer="51"/>
<wire x1="-3.81" y1="0" x2="-2.921" y2="0" width="0.508" layer="51"/>
<wire x1="-2.54" y1="0.762" x2="-2.286" y2="1.016" width="0.1524" layer="21" curve="-90"/>
<wire x1="-2.54" y1="-0.762" x2="-2.286" y2="-1.016" width="0.1524" layer="21" curve="90"/>
<wire x1="2.286" y1="-1.016" x2="2.54" y2="-0.762" width="0.1524" layer="21" curve="90"/>
<wire x1="2.286" y1="1.016" x2="2.54" y2="0.762" width="0.1524" layer="21" curve="-90"/>
<wire x1="-2.54" y1="-0.762" x2="-2.54" y2="0.762" width="0.1524" layer="21"/>
<wire x1="-2.286" y1="1.016" x2="-1.905" y2="1.016" width="0.1524" layer="21"/>
<wire x1="-1.778" y1="0.889" x2="-1.905" y2="1.016" width="0.1524" layer="21"/>
<wire x1="-2.286" y1="-1.016" x2="-1.905" y2="-1.016" width="0.1524" layer="21"/>
<wire x1="-1.778" y1="-0.889" x2="-1.905" y2="-1.016" width="0.1524" layer="21"/>
<wire x1="1.778" y1="0.889" x2="1.905" y2="1.016" width="0.1524" layer="21"/>
<wire x1="1.778" y1="0.889" x2="-1.778" y2="0.889" width="0.1524" layer="21"/>
<wire x1="1.778" y1="-0.889" x2="1.905" y2="-1.016" width="0.1524" layer="21"/>
<wire x1="1.778" y1="-0.889" x2="-1.778" y2="-0.889" width="0.1524" layer="21"/>
<wire x1="2.286" y1="1.016" x2="1.905" y2="1.016" width="0.1524" layer="21"/>
<wire x1="2.286" y1="-1.016" x2="1.905" y2="-1.016" width="0.1524" layer="21"/>
<wire x1="2.54" y1="-0.762" x2="2.54" y2="0.762" width="0.1524" layer="21"/>
<rectangle x1="2.54" y1="-0.254" x2="2.921" y2="0.254" layer="21"/>
<rectangle x1="-2.921" y1="-0.254" x2="-2.54" y2="0.254" layer="21"/>
<pad name="1" x="-3.81" y="0" drill="0.8128" shape="octagon"/>
<pad name="2" x="3.81" y="0" drill="0.8128" shape="octagon"/>
<text x="-2.54" y="1.2954" size="0.9906" layer="25" ratio="10">&gt;NAME</text>
<text x="-1.6256" y="-0.4826" size="0.9906" layer="27" ratio="10">&gt;VALUE</text>
</package>
<package name="0207/10" urn="urn:adsk.eagle:footprint:13216/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;&lt;p&gt;
type 0207, grid 10 mm</description>
<wire x1="5.08" y1="0" x2="4.064" y2="0" width="0.6096" layer="51"/>
<wire x1="-5.08" y1="0" x2="-4.064" y2="0" width="0.6096" layer="51"/>
<wire x1="-3.175" y1="0.889" x2="-2.921" y2="1.143" width="0.1524" layer="21" curve="-90"/>
<wire x1="-3.175" y1="-0.889" x2="-2.921" y2="-1.143" width="0.1524" layer="21" curve="90"/>
<wire x1="2.921" y1="-1.143" x2="3.175" y2="-0.889" width="0.1524" layer="21" curve="90"/>
<wire x1="2.921" y1="1.143" x2="3.175" y2="0.889" width="0.1524" layer="21" curve="-90"/>
<wire x1="-3.175" y1="-0.889" x2="-3.175" y2="0.889" width="0.1524" layer="21"/>
<wire x1="-2.921" y1="1.143" x2="-2.54" y2="1.143" width="0.1524" layer="21"/>
<wire x1="-2.413" y1="1.016" x2="-2.54" y2="1.143" width="0.1524" layer="21"/>
<wire x1="-2.921" y1="-1.143" x2="-2.54" y2="-1.143" width="0.1524" layer="21"/>
<wire x1="-2.413" y1="-1.016" x2="-2.54" y2="-1.143" width="0.1524" layer="21"/>
<wire x1="2.413" y1="1.016" x2="2.54" y2="1.143" width="0.1524" layer="21"/>
<wire x1="2.413" y1="1.016" x2="-2.413" y2="1.016" width="0.1524" layer="21"/>
<wire x1="2.413" y1="-1.016" x2="2.54" y2="-1.143" width="0.1524" layer="21"/>
<wire x1="2.413" y1="-1.016" x2="-2.413" y2="-1.016" width="0.1524" layer="21"/>
<wire x1="2.921" y1="1.143" x2="2.54" y2="1.143" width="0.1524" layer="21"/>
<wire x1="2.921" y1="-1.143" x2="2.54" y2="-1.143" width="0.1524" layer="21"/>
<wire x1="3.175" y1="-0.889" x2="3.175" y2="0.889" width="0.1524" layer="21"/>
<rectangle x1="3.175" y1="-0.3048" x2="4.0386" y2="0.3048" layer="21"/>
<rectangle x1="-4.0386" y1="-0.3048" x2="-3.175" y2="0.3048" layer="21"/>
<pad name="1" x="-5.08" y="0" drill="0.8128" shape="octagon"/>
<pad name="2" x="5.08" y="0" drill="0.8128" shape="octagon"/>
<text x="-3.048" y="1.524" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-2.2606" y="-0.635" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
</package>
<package name="R0201" urn="urn:adsk.eagle:footprint:13233/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt; chip&lt;p&gt;
Source: http://www.vishay.com/docs/20008/dcrcw.pdf</description>
<rectangle x1="-0.3" y1="-0.15" x2="-0.15" y2="0.15" layer="51"/>
<rectangle x1="0.15" y1="-0.15" x2="0.3" y2="0.15" layer="51"/>
<rectangle x1="-0.15" y1="-0.15" x2="0.15" y2="0.15" layer="21"/>
<smd name="1" x="-0.255" y="0" dx="0.28" dy="0.43" layer="1"/>
<smd name="2" x="0.255" y="0" dx="0.28" dy="0.43" layer="1"/>
<text x="-0.635" y="0.635" size="1.27" layer="25">&gt;NAME</text>
<text x="-0.635" y="-1.905" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R0402" urn="urn:adsk.eagle:footprint:13234/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.245" y1="0.224" x2="0.245" y2="0.224" width="0.1524" layer="51"/>
<wire x1="0.245" y1="-0.224" x2="-0.245" y2="-0.224" width="0.1524" layer="51"/>
<wire x1="-1.473" y1="0.483" x2="1.473" y2="0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.483" x2="1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.483" x2="-1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.483" x2="-1.473" y2="0.483" width="0.0508" layer="39"/>
<rectangle x1="-0.554" y1="-0.3048" x2="-0.254" y2="0.2951" layer="51"/>
<rectangle x1="0.2588" y1="-0.3048" x2="0.5588" y2="0.2951" layer="51"/>
<rectangle x1="-0.1999" y1="-0.4001" x2="0.1999" y2="0.4001" layer="35"/>
<smd name="1" x="-0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<text x="-0.635" y="0.635" size="1.27" layer="25">&gt;NAME</text>
<text x="-0.635" y="-1.905" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R0603" urn="urn:adsk.eagle:footprint:13235/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.432" y1="-0.356" x2="0.432" y2="-0.356" width="0.1524" layer="51"/>
<wire x1="0.432" y1="0.356" x2="-0.432" y2="0.356" width="0.1524" layer="51"/>
<wire x1="-1.473" y1="0.983" x2="1.473" y2="0.983" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.983" x2="1.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.983" x2="-1.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.983" x2="-1.473" y2="0.983" width="0.0508" layer="39"/>
<rectangle x1="0.4318" y1="-0.4318" x2="0.8382" y2="0.4318" layer="51"/>
<rectangle x1="-0.8382" y1="-0.4318" x2="-0.4318" y2="0.4318" layer="51"/>
<rectangle x1="-0.1999" y1="-0.4001" x2="0.1999" y2="0.4001" layer="35"/>
<smd name="1" x="-0.85" y="0" dx="1" dy="1.1" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1" dy="1.1" layer="1"/>
<text x="-0.635" y="0.635" size="1.27" layer="25">&gt;NAME</text>
<text x="-0.635" y="-1.905" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R0805" urn="urn:adsk.eagle:footprint:13236/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;&lt;p&gt;</description>
<wire x1="-0.41" y1="0.635" x2="0.41" y2="0.635" width="0.1524" layer="51"/>
<wire x1="-0.41" y1="-0.635" x2="0.41" y2="-0.635" width="0.1524" layer="51"/>
<wire x1="-1.973" y1="0.983" x2="1.973" y2="0.983" width="0.0508" layer="39"/>
<wire x1="1.973" y1="0.983" x2="1.973" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="1.973" y1="-0.983" x2="-1.973" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="-1.973" y1="-0.983" x2="-1.973" y2="0.983" width="0.0508" layer="39"/>
<rectangle x1="0.4064" y1="-0.6985" x2="1.0564" y2="0.7015" layer="51"/>
<rectangle x1="-1.0668" y1="-0.6985" x2="-0.4168" y2="0.7015" layer="51"/>
<rectangle x1="-0.1999" y1="-0.5001" x2="0.1999" y2="0.5001" layer="35"/>
<smd name="1" x="-0.95" y="0" dx="1.3" dy="1.5" layer="1"/>
<smd name="2" x="0.95" y="0" dx="1.3" dy="1.5" layer="1"/>
<text x="-0.635" y="1.27" size="1.27" layer="25">&gt;NAME</text>
<text x="-0.635" y="-2.54" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R1005" urn="urn:adsk.eagle:footprint:13237/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.245" y1="0.224" x2="0.245" y2="0.224" width="0.1524" layer="51"/>
<wire x1="0.245" y1="-0.224" x2="-0.245" y2="-0.224" width="0.1524" layer="51"/>
<wire x1="-1.473" y1="0.483" x2="1.473" y2="0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.483" x2="1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.483" x2="-1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.483" x2="-1.473" y2="0.483" width="0.0508" layer="39"/>
<rectangle x1="-0.554" y1="-0.3048" x2="-0.254" y2="0.2951" layer="51"/>
<rectangle x1="0.2588" y1="-0.3048" x2="0.5588" y2="0.2951" layer="51"/>
<rectangle x1="-0.1999" y1="-0.3" x2="0.1999" y2="0.3" layer="35"/>
<smd name="1" x="-0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<text x="-0.635" y="0.635" size="1.27" layer="25">&gt;NAME</text>
<text x="-0.635" y="-1.905" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R1206" urn="urn:adsk.eagle:footprint:13238/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="0.9525" y1="-0.8128" x2="-0.9652" y2="-0.8128" width="0.1524" layer="51"/>
<wire x1="0.9525" y1="0.8128" x2="-0.9652" y2="0.8128" width="0.1524" layer="51"/>
<wire x1="-2.473" y1="0.983" x2="2.473" y2="0.983" width="0.0508" layer="39"/>
<wire x1="2.473" y1="0.983" x2="2.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="2.473" y1="-0.983" x2="-2.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="-2.473" y1="-0.983" x2="-2.473" y2="0.983" width="0.0508" layer="39"/>
<rectangle x1="-1.6891" y1="-0.8763" x2="-0.9525" y2="0.8763" layer="51"/>
<rectangle x1="0.9525" y1="-0.8763" x2="1.6891" y2="0.8763" layer="51"/>
<rectangle x1="-0.3" y1="-0.7" x2="0.3" y2="0.7" layer="35"/>
<smd name="1" x="-1.422" y="0" dx="1.6" dy="1.803" layer="1"/>
<smd name="2" x="1.422" y="0" dx="1.6" dy="1.803" layer="1"/>
<text x="-1.27" y="1.27" size="1.27" layer="25">&gt;NAME</text>
<text x="-1.27" y="-2.54" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R1210" urn="urn:adsk.eagle:footprint:13239/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.913" y1="1.219" x2="0.939" y2="1.219" width="0.1524" layer="51"/>
<wire x1="-0.913" y1="-1.219" x2="0.939" y2="-1.219" width="0.1524" layer="51"/>
<wire x1="-2.473" y1="1.483" x2="2.473" y2="1.483" width="0.0508" layer="39"/>
<wire x1="2.473" y1="1.483" x2="2.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="2.473" y1="-1.483" x2="-2.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="-2.473" y1="-1.483" x2="-2.473" y2="1.483" width="0.0508" layer="39"/>
<rectangle x1="-1.651" y1="-1.3081" x2="-0.9009" y2="1.2918" layer="51"/>
<rectangle x1="0.9144" y1="-1.3081" x2="1.6645" y2="1.2918" layer="51"/>
<rectangle x1="-0.3" y1="-0.8999" x2="0.3" y2="0.8999" layer="35"/>
<smd name="1" x="-1.4" y="0" dx="1.6" dy="2.7" layer="1"/>
<smd name="2" x="1.4" y="0" dx="1.6" dy="2.7" layer="1"/>
<text x="-2.54" y="1.905" size="1.27" layer="25">&gt;NAME</text>
<text x="-2.54" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R1218" urn="urn:adsk.eagle:footprint:13240/1" library_version="1">
<description>&lt;b&gt;CRCW1218 Thick Film, Rectangular Chip Resistors&lt;/b&gt;&lt;p&gt;
Source: http://www.vishay.com .. dcrcw.pdf</description>
<wire x1="-0.913" y1="-2.219" x2="0.939" y2="-2.219" width="0.1524" layer="51"/>
<wire x1="0.913" y1="2.219" x2="-0.939" y2="2.219" width="0.1524" layer="51"/>
<rectangle x1="-1.651" y1="-2.3" x2="-0.9009" y2="2.3" layer="51"/>
<rectangle x1="0.9144" y1="-2.3" x2="1.6645" y2="2.3" layer="51"/>
<smd name="1" x="-1.475" y="0" dx="1.05" dy="4.9" layer="1"/>
<smd name="2" x="1.475" y="0" dx="1.05" dy="4.9" layer="1"/>
<text x="-2.54" y="2.54" size="1.27" layer="25">&gt;NAME</text>
<text x="-2.54" y="-3.81" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R2010" urn="urn:adsk.eagle:footprint:13241/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-1.662" y1="1.245" x2="1.662" y2="1.245" width="0.1524" layer="51"/>
<wire x1="-1.637" y1="-1.245" x2="1.687" y2="-1.245" width="0.1524" layer="51"/>
<wire x1="-3.473" y1="1.483" x2="3.473" y2="1.483" width="0.0508" layer="39"/>
<wire x1="3.473" y1="1.483" x2="3.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="3.473" y1="-1.483" x2="-3.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="-3.473" y1="-1.483" x2="-3.473" y2="1.483" width="0.0508" layer="39"/>
<rectangle x1="-2.4892" y1="-1.3208" x2="-1.6393" y2="1.3292" layer="51"/>
<rectangle x1="1.651" y1="-1.3208" x2="2.5009" y2="1.3292" layer="51"/>
<smd name="1" x="-2.2" y="0" dx="1.8" dy="2.7" layer="1"/>
<smd name="2" x="2.2" y="0" dx="1.8" dy="2.7" layer="1"/>
<text x="-3.175" y="1.905" size="1.27" layer="25">&gt;NAME</text>
<text x="-3.175" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R2012" urn="urn:adsk.eagle:footprint:13242/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.41" y1="0.635" x2="0.41" y2="0.635" width="0.1524" layer="51"/>
<wire x1="-0.41" y1="-0.635" x2="0.41" y2="-0.635" width="0.1524" layer="51"/>
<wire x1="-1.973" y1="0.983" x2="1.973" y2="0.983" width="0.0508" layer="39"/>
<wire x1="1.973" y1="0.983" x2="1.973" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="1.973" y1="-0.983" x2="-1.973" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="-1.973" y1="-0.983" x2="-1.973" y2="0.983" width="0.0508" layer="39"/>
<rectangle x1="0.4064" y1="-0.6985" x2="1.0564" y2="0.7015" layer="51"/>
<rectangle x1="-1.0668" y1="-0.6985" x2="-0.4168" y2="0.7015" layer="51"/>
<rectangle x1="-0.1001" y1="-0.5999" x2="0.1001" y2="0.5999" layer="35"/>
<smd name="1" x="-0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<text x="-0.635" y="1.27" size="1.27" layer="25">&gt;NAME</text>
<text x="-0.635" y="-2.54" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R2512" urn="urn:adsk.eagle:footprint:13243/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-2.362" y1="1.473" x2="2.387" y2="1.473" width="0.1524" layer="51"/>
<wire x1="-2.362" y1="-1.473" x2="2.387" y2="-1.473" width="0.1524" layer="51"/>
<wire x1="-3.973" y1="1.983" x2="3.973" y2="1.983" width="0.0508" layer="39"/>
<wire x1="3.973" y1="1.983" x2="3.973" y2="-1.983" width="0.0508" layer="39"/>
<wire x1="3.973" y1="-1.983" x2="-3.973" y2="-1.983" width="0.0508" layer="39"/>
<wire x1="-3.973" y1="-1.983" x2="-3.973" y2="1.983" width="0.0508" layer="39"/>
<rectangle x1="-3.2004" y1="-1.5494" x2="-2.3505" y2="1.5507" layer="51"/>
<rectangle x1="2.3622" y1="-1.5494" x2="3.2121" y2="1.5507" layer="51"/>
<rectangle x1="-0.5001" y1="-1" x2="0.5001" y2="1" layer="35"/>
<smd name="1" x="-2.8" y="0" dx="1.8" dy="3.2" layer="1"/>
<smd name="2" x="2.8" y="0" dx="1.8" dy="3.2" layer="1"/>
<text x="-2.54" y="1.905" size="1.27" layer="25">&gt;NAME</text>
<text x="-2.54" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R3216" urn="urn:adsk.eagle:footprint:13244/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.913" y1="0.8" x2="0.888" y2="0.8" width="0.1524" layer="51"/>
<wire x1="-0.913" y1="-0.8" x2="0.888" y2="-0.8" width="0.1524" layer="51"/>
<wire x1="-2.473" y1="0.983" x2="2.473" y2="0.983" width="0.0508" layer="39"/>
<wire x1="2.473" y1="0.983" x2="2.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="2.473" y1="-0.983" x2="-2.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="-2.473" y1="-0.983" x2="-2.473" y2="0.983" width="0.0508" layer="39"/>
<rectangle x1="-1.651" y1="-0.8763" x2="-0.9009" y2="0.8738" layer="51"/>
<rectangle x1="0.889" y1="-0.8763" x2="1.6391" y2="0.8738" layer="51"/>
<rectangle x1="-0.3" y1="-0.7" x2="0.3" y2="0.7" layer="35"/>
<smd name="1" x="-1.4" y="0" dx="1.6" dy="1.8" layer="1"/>
<smd name="2" x="1.4" y="0" dx="1.6" dy="1.8" layer="1"/>
<text x="-1.905" y="1.27" size="1.27" layer="25">&gt;NAME</text>
<text x="-1.905" y="-2.54" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R3225" urn="urn:adsk.eagle:footprint:13245/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-0.913" y1="1.219" x2="0.939" y2="1.219" width="0.1524" layer="51"/>
<wire x1="-0.913" y1="-1.219" x2="0.939" y2="-1.219" width="0.1524" layer="51"/>
<wire x1="-2.473" y1="1.483" x2="2.473" y2="1.483" width="0.0508" layer="39"/>
<wire x1="2.473" y1="1.483" x2="2.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="2.473" y1="-1.483" x2="-2.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="-2.473" y1="-1.483" x2="-2.473" y2="1.483" width="0.0508" layer="39"/>
<rectangle x1="-1.651" y1="-1.3081" x2="-0.9009" y2="1.2918" layer="51"/>
<rectangle x1="0.9144" y1="-1.3081" x2="1.6645" y2="1.2918" layer="51"/>
<rectangle x1="-0.3" y1="-1" x2="0.3" y2="1" layer="35"/>
<smd name="1" x="-1.4" y="0" dx="1.6" dy="2.7" layer="1"/>
<smd name="2" x="1.4" y="0" dx="1.6" dy="2.7" layer="1"/>
<text x="-2.54" y="1.905" size="1.27" layer="25">&gt;NAME</text>
<text x="-2.54" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R4527" urn="urn:adsk.eagle:footprint:13246/1" library_version="1">
<description>&lt;b&gt;Package 4527&lt;/b&gt;&lt;p&gt;
Source: http://www.vishay.com/docs/31059/wsrhigh.pdf</description>
<wire x1="-5.675" y1="-3.375" x2="5.65" y2="-3.375" width="0.2032" layer="21"/>
<wire x1="5.65" y1="-3.375" x2="5.65" y2="3.375" width="0.2032" layer="51"/>
<wire x1="5.65" y1="3.375" x2="-5.675" y2="3.375" width="0.2032" layer="21"/>
<wire x1="-5.675" y1="3.375" x2="-5.675" y2="-3.375" width="0.2032" layer="51"/>
<smd name="1" x="-4.575" y="0" dx="3.94" dy="5.84" layer="1"/>
<smd name="2" x="4.575" y="0" dx="3.94" dy="5.84" layer="1"/>
<text x="-5.715" y="3.81" size="1.27" layer="25">&gt;NAME</text>
<text x="-5.715" y="-5.08" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R5025" urn="urn:adsk.eagle:footprint:13247/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;</description>
<wire x1="-1.662" y1="1.245" x2="1.662" y2="1.245" width="0.1524" layer="51"/>
<wire x1="-1.637" y1="-1.245" x2="1.687" y2="-1.245" width="0.1524" layer="51"/>
<wire x1="-3.473" y1="1.483" x2="3.473" y2="1.483" width="0.0508" layer="39"/>
<wire x1="3.473" y1="1.483" x2="3.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="3.473" y1="-1.483" x2="-3.473" y2="-1.483" width="0.0508" layer="39"/>
<wire x1="-3.473" y1="-1.483" x2="-3.473" y2="1.483" width="0.0508" layer="39"/>
<rectangle x1="-2.4892" y1="-1.3208" x2="-1.6393" y2="1.3292" layer="51"/>
<rectangle x1="1.651" y1="-1.3208" x2="2.5009" y2="1.3292" layer="51"/>
<rectangle x1="-0.5001" y1="-1" x2="0.5001" y2="1" layer="35"/>
<smd name="1" x="-2.2" y="0" dx="1.8" dy="2.7" layer="1"/>
<smd name="2" x="2.2" y="0" dx="1.8" dy="2.7" layer="1"/>
<text x="-3.175" y="1.905" size="1.27" layer="25">&gt;NAME</text>
<text x="-3.175" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="R6332" urn="urn:adsk.eagle:footprint:13248/1" library_version="1">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;&lt;p&gt;
Source: http://download.siliconexpert.com/pdfs/2005/02/24/Semi_Ap/2/VSH/Resistor/dcrcwfre.pdf</description>
<wire x1="-2.362" y1="1.473" x2="2.387" y2="1.473" width="0.1524" layer="51"/>
<wire x1="-2.362" y1="-1.473" x2="2.387" y2="-1.473" width="0.1524" layer="51"/>
<wire x1="-3.973" y1="1.983" x2="3.973" y2="1.983" width="0.0508" layer="39"/>
<wire x1="3.973" y1="1.983" x2="3.973" y2="-1.983" width="0.0508" layer="39"/>
<wire x1="3.973" y1="-1.983" x2="-3.973" y2="-1.983" width="0.0508" layer="39"/>
<wire x1="-3.973" y1="-1.983" x2="-3.973" y2="1.983" width="0.0508" layer="39"/>
<rectangle x1="-3.2004" y1="-1.5494" x2="-2.3505" y2="1.5507" layer="51"/>
<rectangle x1="2.3622" y1="-1.5494" x2="3.2121" y2="1.5507" layer="51"/>
<rectangle x1="-0.5001" y1="-1" x2="0.5001" y2="1" layer="35"/>
<smd name="1" x="-3.1" y="0" dx="1" dy="3.2" layer="1"/>
<smd name="2" x="3.1" y="0" dx="1" dy="3.2" layer="1"/>
<text x="-2.54" y="1.905" size="1.27" layer="25">&gt;NAME</text>
<text x="-2.54" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
</package>
</packages>
<packages3d>
<package3d name="0204/7" urn="urn:adsk.eagle:package:13274/1" type="box" library_version="1">
<description>RESISTOR
type 0204, grid 7.5 mm</description>
</package3d>
<package3d name="0207/10" urn="urn:adsk.eagle:package:13275/1" type="box" library_version="1">
<description>RESISTOR
type 0207, grid 10 mm</description>
</package3d>
<package3d name="R0201" urn="urn:adsk.eagle:package:13294/1" type="box" library_version="1">
<description>RESISTOR chip
Source: http://www.vishay.com/docs/20008/dcrcw.pdf</description>
</package3d>
<package3d name="R0402" urn="urn:adsk.eagle:package:13296/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R0603" urn="urn:adsk.eagle:package:13302/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R0805" urn="urn:adsk.eagle:package:13300/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R1005" urn="urn:adsk.eagle:package:13297/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R1206" urn="urn:adsk.eagle:package:13301/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R1210" urn="urn:adsk.eagle:package:13299/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R1218" urn="urn:adsk.eagle:package:13303/1" type="box" library_version="1">
<description>CRCW1218 Thick Film, Rectangular Chip Resistors
Source: http://www.vishay.com .. dcrcw.pdf</description>
</package3d>
<package3d name="R2010" urn="urn:adsk.eagle:package:13309/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R2012" urn="urn:adsk.eagle:package:13306/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R2512" urn="urn:adsk.eagle:package:13304/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R3216" urn="urn:adsk.eagle:package:13305/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R3225" urn="urn:adsk.eagle:package:13311/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R4527" urn="urn:adsk.eagle:package:13310/1" type="box" library_version="1">
<description>Package 4527
Source: http://www.vishay.com/docs/31059/wsrhigh.pdf</description>
</package3d>
<package3d name="R5025" urn="urn:adsk.eagle:package:13308/1" type="box" library_version="1">
<description>RESISTOR</description>
</package3d>
<package3d name="R6332" urn="urn:adsk.eagle:package:13307/1" type="box" library_version="1">
<description>RESISTOR
Source: http://download.siliconexpert.com/pdfs/2005/02/24/Semi_Ap/2/VSH/Resistor/dcrcwfre.pdf</description>
</package3d>
</packages3d>
<symbols>
<symbol name="R" urn="urn:adsk.eagle:symbol:13232/1" library_version="1">
<wire x1="-2.54" y1="-0.889" x2="2.54" y2="-0.889" width="0.254" layer="94"/>
<wire x1="2.54" y1="0.889" x2="-2.54" y2="0.889" width="0.254" layer="94"/>
<wire x1="2.54" y1="-0.889" x2="2.54" y2="0.889" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-0.889" x2="-2.54" y2="0.889" width="0.254" layer="94"/>
<pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
<pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<text x="-3.81" y="1.4986" size="1.778" layer="95">&gt;NAME</text>
<text x="-3.81" y="-3.302" size="1.778" layer="96">&gt;VALUE</text>
<text x="-5.08" y="0" size="0.4064" layer="99" align="center">SpiceOrder 1</text>
<text x="5.08" y="0" size="0.4064" layer="99" align="center">SpiceOrder 2</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="R" urn="urn:adsk.eagle:component:13322/1" prefix="R" uservalue="yes" library_version="1">
<description>&lt;B&gt;RESISTOR&lt;/B&gt;, European symbol</description>
<gates>
<gate name="G$1" symbol="R" x="0" y="0"/>
</gates>
<devices>
<device name="0204/7" package="0204/7">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13274/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="0207/10" package="0207/10">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13275/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R0201" package="R0201">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13294/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R0402" package="R0402">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13296/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R0603" package="R0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13302/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="" package="R0805">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13300/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R1005" package="R1005">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13297/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R1206" package="R1206">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13301/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R1210" package="R1210">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13299/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R1218" package="R1218">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13303/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R2010" package="R2010">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13309/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R2012" package="R2012">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13306/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R2512" package="R2512">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13304/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R3216" package="R3216">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13305/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R3225" package="R3225">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13311/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R4527" package="R4527">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13310/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R5025" package="R5025">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13308/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
<device name="R6332" package="R6332">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:13307/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="SPICEMODEL" value="NONE" constant="no"/>
<attribute name="SPICEPREFIX" value="R" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="pinhead" urn="urn:adsk.eagle:library:325">
<description>&lt;b&gt;Pin Header Connectors&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="1X08" urn="urn:adsk.eagle:footprint:22262/1" library_version="2">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt;</description>
<wire x1="5.715" y1="1.27" x2="6.985" y2="1.27" width="0.1524" layer="21"/>
<wire x1="6.985" y1="1.27" x2="7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="7.62" y1="0.635" x2="7.62" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="7.62" y1="-0.635" x2="6.985" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="2.54" y1="0.635" x2="3.175" y2="1.27" width="0.1524" layer="21"/>
<wire x1="3.175" y1="1.27" x2="4.445" y2="1.27" width="0.1524" layer="21"/>
<wire x1="4.445" y1="1.27" x2="5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="5.08" y1="0.635" x2="5.08" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="5.08" y1="-0.635" x2="4.445" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="4.445" y1="-1.27" x2="3.175" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="3.175" y1="-1.27" x2="2.54" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="5.715" y1="1.27" x2="5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="5.08" y1="-0.635" x2="5.715" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="6.985" y1="-1.27" x2="5.715" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-1.905" y1="1.27" x2="-0.635" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-0.635" y1="1.27" x2="0" y2="0.635" width="0.1524" layer="21"/>
<wire x1="0" y1="0.635" x2="0" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="0" y1="-0.635" x2="-0.635" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="0" y1="0.635" x2="0.635" y2="1.27" width="0.1524" layer="21"/>
<wire x1="0.635" y1="1.27" x2="1.905" y2="1.27" width="0.1524" layer="21"/>
<wire x1="1.905" y1="1.27" x2="2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="2.54" y1="0.635" x2="2.54" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="2.54" y1="-0.635" x2="1.905" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="1.905" y1="-1.27" x2="0.635" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="0.635" y1="-1.27" x2="0" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="0.635" x2="-4.445" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-4.445" y1="1.27" x2="-3.175" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-3.175" y1="1.27" x2="-2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-2.54" y1="0.635" x2="-2.54" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-2.54" y1="-0.635" x2="-3.175" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-3.175" y1="-1.27" x2="-4.445" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-4.445" y1="-1.27" x2="-5.08" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-1.905" y1="1.27" x2="-2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-2.54" y1="-0.635" x2="-1.905" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-0.635" y1="-1.27" x2="-1.905" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-9.525" y1="1.27" x2="-8.255" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-8.255" y1="1.27" x2="-7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-7.62" y1="0.635" x2="-7.62" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-7.62" y1="-0.635" x2="-8.255" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-7.62" y1="0.635" x2="-6.985" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-6.985" y1="1.27" x2="-5.715" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-5.715" y1="1.27" x2="-5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="0.635" x2="-5.08" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-0.635" x2="-5.715" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-5.715" y1="-1.27" x2="-6.985" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-6.985" y1="-1.27" x2="-7.62" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-10.16" y1="0.635" x2="-10.16" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-9.525" y1="1.27" x2="-10.16" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-10.16" y1="-0.635" x2="-9.525" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="-8.255" y1="-1.27" x2="-9.525" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="8.255" y1="1.27" x2="9.525" y2="1.27" width="0.1524" layer="21"/>
<wire x1="9.525" y1="1.27" x2="10.16" y2="0.635" width="0.1524" layer="21"/>
<wire x1="10.16" y1="0.635" x2="10.16" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="10.16" y1="-0.635" x2="9.525" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="8.255" y1="1.27" x2="7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="7.62" y1="-0.635" x2="8.255" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="9.525" y1="-1.27" x2="8.255" y2="-1.27" width="0.1524" layer="21"/>
<pad name="1" x="-8.89" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="2" x="-6.35" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="3" x="-3.81" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="4" x="-1.27" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="5" x="1.27" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="6" x="3.81" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="7" x="6.35" y="0" drill="1.016" shape="long" rot="R90"/>
<pad name="8" x="8.89" y="0" drill="1.016" shape="long" rot="R90"/>
<text x="-10.2362" y="1.8288" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-10.16" y="-3.175" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="6.096" y1="-0.254" x2="6.604" y2="0.254" layer="51"/>
<rectangle x1="3.556" y1="-0.254" x2="4.064" y2="0.254" layer="51"/>
<rectangle x1="1.016" y1="-0.254" x2="1.524" y2="0.254" layer="51"/>
<rectangle x1="-1.524" y1="-0.254" x2="-1.016" y2="0.254" layer="51"/>
<rectangle x1="-4.064" y1="-0.254" x2="-3.556" y2="0.254" layer="51"/>
<rectangle x1="-6.604" y1="-0.254" x2="-6.096" y2="0.254" layer="51"/>
<rectangle x1="-9.144" y1="-0.254" x2="-8.636" y2="0.254" layer="51"/>
<rectangle x1="8.636" y1="-0.254" x2="9.144" y2="0.254" layer="51"/>
</package>
<package name="1X08/90" urn="urn:adsk.eagle:footprint:22261/1" library_version="2">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt;</description>
<wire x1="-10.16" y1="-1.905" x2="-7.62" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-7.62" y1="-1.905" x2="-7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-7.62" y1="0.635" x2="-10.16" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-10.16" y1="0.635" x2="-10.16" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-8.89" y1="6.985" x2="-8.89" y2="1.27" width="0.762" layer="21"/>
<wire x1="-7.62" y1="-1.905" x2="-5.08" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-1.905" x2="-5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="0.635" x2="-7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-6.35" y1="6.985" x2="-6.35" y2="1.27" width="0.762" layer="21"/>
<wire x1="-5.08" y1="-1.905" x2="-2.54" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-2.54" y1="-1.905" x2="-2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-2.54" y1="0.635" x2="-5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-3.81" y1="6.985" x2="-3.81" y2="1.27" width="0.762" layer="21"/>
<wire x1="-2.54" y1="-1.905" x2="0" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="0" y1="-1.905" x2="0" y2="0.635" width="0.1524" layer="21"/>
<wire x1="0" y1="0.635" x2="-2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="6.985" x2="-1.27" y2="1.27" width="0.762" layer="21"/>
<wire x1="0" y1="-1.905" x2="2.54" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="2.54" y1="-1.905" x2="2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="2.54" y1="0.635" x2="0" y2="0.635" width="0.1524" layer="21"/>
<wire x1="1.27" y1="6.985" x2="1.27" y2="1.27" width="0.762" layer="21"/>
<wire x1="2.54" y1="-1.905" x2="5.08" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="5.08" y1="-1.905" x2="5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="5.08" y1="0.635" x2="2.54" y2="0.635" width="0.1524" layer="21"/>
<wire x1="3.81" y1="6.985" x2="3.81" y2="1.27" width="0.762" layer="21"/>
<wire x1="5.08" y1="-1.905" x2="7.62" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="7.62" y1="-1.905" x2="7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="7.62" y1="0.635" x2="5.08" y2="0.635" width="0.1524" layer="21"/>
<wire x1="6.35" y1="6.985" x2="6.35" y2="1.27" width="0.762" layer="21"/>
<wire x1="7.62" y1="-1.905" x2="10.16" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="10.16" y1="-1.905" x2="10.16" y2="0.635" width="0.1524" layer="21"/>
<wire x1="10.16" y1="0.635" x2="7.62" y2="0.635" width="0.1524" layer="21"/>
<wire x1="8.89" y1="6.985" x2="8.89" y2="1.27" width="0.762" layer="21"/>
<pad name="1" x="-8.89" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="2" x="-6.35" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="3" x="-3.81" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="4" x="-1.27" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="5" x="1.27" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="6" x="3.81" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="7" x="6.35" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<pad name="8" x="8.89" y="-3.81" drill="1.016" shape="long" rot="R90"/>
<text x="-10.795" y="-3.81" size="1.27" layer="25" ratio="10" rot="R90">&gt;NAME</text>
<text x="12.065" y="-3.81" size="1.27" layer="27" rot="R90">&gt;VALUE</text>
<rectangle x1="-9.271" y1="0.635" x2="-8.509" y2="1.143" layer="21"/>
<rectangle x1="-6.731" y1="0.635" x2="-5.969" y2="1.143" layer="21"/>
<rectangle x1="-4.191" y1="0.635" x2="-3.429" y2="1.143" layer="21"/>
<rectangle x1="-1.651" y1="0.635" x2="-0.889" y2="1.143" layer="21"/>
<rectangle x1="0.889" y1="0.635" x2="1.651" y2="1.143" layer="21"/>
<rectangle x1="3.429" y1="0.635" x2="4.191" y2="1.143" layer="21"/>
<rectangle x1="5.969" y1="0.635" x2="6.731" y2="1.143" layer="21"/>
<rectangle x1="8.509" y1="0.635" x2="9.271" y2="1.143" layer="21"/>
<rectangle x1="-9.271" y1="-2.921" x2="-8.509" y2="-1.905" layer="21"/>
<rectangle x1="-6.731" y1="-2.921" x2="-5.969" y2="-1.905" layer="21"/>
<rectangle x1="-4.191" y1="-2.921" x2="-3.429" y2="-1.905" layer="21"/>
<rectangle x1="-1.651" y1="-2.921" x2="-0.889" y2="-1.905" layer="21"/>
<rectangle x1="0.889" y1="-2.921" x2="1.651" y2="-1.905" layer="21"/>
<rectangle x1="3.429" y1="-2.921" x2="4.191" y2="-1.905" layer="21"/>
<rectangle x1="5.969" y1="-2.921" x2="6.731" y2="-1.905" layer="21"/>
<rectangle x1="8.509" y1="-2.921" x2="9.271" y2="-1.905" layer="21"/>
</package>
</packages>
<packages3d>
<package3d name="1X08" urn="urn:adsk.eagle:package:22409/2" type="model" library_version="2">
<description>PIN HEADER</description>
</package3d>
<package3d name="1X08/90" urn="urn:adsk.eagle:package:22413/1" type="box" library_version="2">
<description>PIN HEADER</description>
</package3d>
</packages3d>
<symbols>
<symbol name="PINHD8" urn="urn:adsk.eagle:symbol:22260/1" library_version="2">
<wire x1="-6.35" y1="-10.16" x2="1.27" y2="-10.16" width="0.4064" layer="94"/>
<wire x1="1.27" y1="-10.16" x2="1.27" y2="12.7" width="0.4064" layer="94"/>
<wire x1="1.27" y1="12.7" x2="-6.35" y2="12.7" width="0.4064" layer="94"/>
<wire x1="-6.35" y1="12.7" x2="-6.35" y2="-10.16" width="0.4064" layer="94"/>
<text x="-6.35" y="13.335" size="1.778" layer="95">&gt;NAME</text>
<text x="-6.35" y="-12.7" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-2.54" y="10.16" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="2" x="-2.54" y="7.62" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="3" x="-2.54" y="5.08" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="4" x="-2.54" y="2.54" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="5" x="-2.54" y="0" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="6" x="-2.54" y="-2.54" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="7" x="-2.54" y="-5.08" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="8" x="-2.54" y="-7.62" visible="pad" length="short" direction="pas" function="dot"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="PINHD-1X8" urn="urn:adsk.eagle:component:22503/2" prefix="JP" uservalue="yes" library_version="2">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt;</description>
<gates>
<gate name="A" symbol="PINHD8" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1X08">
<connects>
<connect gate="A" pin="1" pad="1"/>
<connect gate="A" pin="2" pad="2"/>
<connect gate="A" pin="3" pad="3"/>
<connect gate="A" pin="4" pad="4"/>
<connect gate="A" pin="5" pad="5"/>
<connect gate="A" pin="6" pad="6"/>
<connect gate="A" pin="7" pad="7"/>
<connect gate="A" pin="8" pad="8"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:22409/2"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="/90" package="1X08/90">
<connects>
<connect gate="A" pin="1" pad="1"/>
<connect gate="A" pin="2" pad="2"/>
<connect gate="A" pin="3" pad="3"/>
<connect gate="A" pin="4" pad="4"/>
<connect gate="A" pin="5" pad="5"/>
<connect gate="A" pin="6" pad="6"/>
<connect gate="A" pin="7" pad="7"/>
<connect gate="A" pin="8" pad="8"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:22413/1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="jumper" urn="urn:adsk.eagle:library:252">
<description>&lt;b&gt;Jumpers&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="JP1" urn="urn:adsk.eagle:footprint:15398/1" library_version="1">
<description>&lt;b&gt;JUMPER&lt;/b&gt;</description>
<wire x1="-1.016" y1="0" x2="-1.27" y2="0.254" width="0.1524" layer="21"/>
<wire x1="-1.016" y1="0" x2="-1.27" y2="-0.254" width="0.1524" layer="21"/>
<wire x1="1.016" y1="0" x2="1.27" y2="0.254" width="0.1524" layer="21"/>
<wire x1="1.016" y1="0" x2="1.27" y2="-0.254" width="0.1524" layer="21"/>
<wire x1="1.27" y1="-0.254" x2="1.27" y2="-2.286" width="0.1524" layer="21"/>
<wire x1="1.016" y1="-2.54" x2="1.27" y2="-2.286" width="0.1524" layer="21"/>
<wire x1="1.27" y1="2.286" x2="1.016" y2="2.54" width="0.1524" layer="21"/>
<wire x1="1.27" y1="2.286" x2="1.27" y2="0.254" width="0.1524" layer="21"/>
<wire x1="1.016" y1="2.54" x2="-1.016" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="2.286" x2="-1.016" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="2.286" x2="-1.27" y2="0.254" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="-0.254" x2="-1.27" y2="-2.286" width="0.1524" layer="21"/>
<wire x1="-1.016" y1="-2.54" x2="-1.27" y2="-2.286" width="0.1524" layer="21"/>
<wire x1="-1.016" y1="-2.54" x2="1.016" y2="-2.54" width="0.1524" layer="21"/>
<pad name="1" x="0" y="-1.27" drill="0.9144" shape="long"/>
<pad name="2" x="0" y="1.27" drill="0.9144" shape="long"/>
<text x="-1.651" y="-2.54" size="1.27" layer="25" ratio="10" rot="R90">&gt;NAME</text>
<text x="2.921" y="-2.54" size="1.27" layer="27" ratio="10" rot="R90">&gt;VALUE</text>
<rectangle x1="-0.3048" y1="0.9652" x2="0.3048" y2="1.5748" layer="51"/>
<rectangle x1="-0.3048" y1="-1.5748" x2="0.3048" y2="-0.9652" layer="51"/>
</package>
</packages>
<packages3d>
<package3d name="JP1" urn="urn:adsk.eagle:package:15455/1" type="box" library_version="1">
<description>JUMPER</description>
</package3d>
</packages3d>
<symbols>
<symbol name="JP2E" urn="urn:adsk.eagle:symbol:15391/1" library_version="1">
<wire x1="0" y1="0" x2="0" y2="1.27" width="0.1524" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="1.27" width="0.4064" layer="94"/>
<wire x1="2.54" y1="0" x2="2.54" y2="1.27" width="0.1524" layer="94"/>
<wire x1="2.54" y1="2.54" x2="2.54" y2="1.27" width="0.4064" layer="94"/>
<wire x1="-0.635" y1="0" x2="3.175" y2="0" width="0.4064" layer="94"/>
<wire x1="3.175" y1="0" x2="3.175" y2="0.635" width="0.4064" layer="94"/>
<wire x1="3.175" y1="0.635" x2="-0.635" y2="0.635" width="0.4064" layer="94"/>
<wire x1="-0.635" y1="0.635" x2="-0.635" y2="0" width="0.4064" layer="94"/>
<text x="-1.27" y="0" size="1.778" layer="95" rot="R90">&gt;NAME</text>
<text x="5.715" y="0" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="1" x="0" y="-2.54" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="2" x="2.54" y="-2.54" visible="pad" length="short" direction="pas" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="JP1E" urn="urn:adsk.eagle:component:15487/1" prefix="JP" uservalue="yes" library_version="1">
<description>&lt;b&gt;JUMPER&lt;/b&gt;</description>
<gates>
<gate name="A" symbol="JP2E" x="2.54" y="0"/>
</gates>
<devices>
<device name="" package="JP1">
<connects>
<connect gate="A" pin="1" pad="1"/>
<connect gate="A" pin="2" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:15455/1"/>
</package3dinstances>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="con-wago-500" urn="urn:adsk.eagle:library:195">
<description>&lt;b&gt;Wago Screw Clamps&lt;/b&gt;&lt;p&gt;
Grid 5.00 mm&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="W237-102" urn="urn:adsk.eagle:footprint:10676/1" library_version="1">
<description>&lt;b&gt;WAGO SCREW CLAMP&lt;/b&gt;</description>
<wire x1="-3.491" y1="-2.286" x2="-1.484" y2="-0.279" width="0.254" layer="51"/>
<wire x1="1.488" y1="-2.261" x2="3.469" y2="-0.254" width="0.254" layer="51"/>
<wire x1="-4.989" y1="-5.461" x2="4.993" y2="-5.461" width="0.1524" layer="21"/>
<wire x1="4.993" y1="3.734" x2="4.993" y2="3.531" width="0.1524" layer="21"/>
<wire x1="4.993" y1="3.734" x2="-4.989" y2="3.734" width="0.1524" layer="21"/>
<wire x1="-4.989" y1="-5.461" x2="-4.989" y2="-3.073" width="0.1524" layer="21"/>
<wire x1="-4.989" y1="-3.073" x2="-3.389" y2="-3.073" width="0.1524" layer="21"/>
<wire x1="-3.389" y1="-3.073" x2="-1.611" y2="-3.073" width="0.1524" layer="51"/>
<wire x1="-1.611" y1="-3.073" x2="1.615" y2="-3.073" width="0.1524" layer="21"/>
<wire x1="3.393" y1="-3.073" x2="4.993" y2="-3.073" width="0.1524" layer="21"/>
<wire x1="-4.989" y1="-3.073" x2="-4.989" y2="3.531" width="0.1524" layer="21"/>
<wire x1="4.993" y1="-3.073" x2="4.993" y2="-5.461" width="0.1524" layer="21"/>
<wire x1="-4.989" y1="3.531" x2="4.993" y2="3.531" width="0.1524" layer="21"/>
<wire x1="-4.989" y1="3.531" x2="-4.989" y2="3.734" width="0.1524" layer="21"/>
<wire x1="4.993" y1="3.531" x2="4.993" y2="-3.073" width="0.1524" layer="21"/>
<wire x1="1.615" y1="-3.073" x2="3.393" y2="-3.073" width="0.1524" layer="51"/>
<circle x="-2.5" y="-1.27" radius="1.4986" width="0.1524" layer="51"/>
<circle x="-2.5" y="2.2098" radius="0.508" width="0.1524" layer="21"/>
<circle x="2.5038" y="-1.27" radius="1.4986" width="0.1524" layer="51"/>
<circle x="2.5038" y="2.2098" radius="0.508" width="0.1524" layer="21"/>
<pad name="1" x="-2.5" y="-1.27" drill="1.1938" shape="long" rot="R90"/>
<pad name="2" x="2.5" y="-1.27" drill="1.1938" shape="long" rot="R90"/>
<text x="-5.04" y="-7.62" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
<text x="-3.8462" y="-5.0038" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-4.532" y="0.635" size="1.27" layer="21" ratio="10">1</text>
<text x="0.421" y="0.635" size="1.27" layer="21" ratio="10">2</text>
</package>
</packages>
<packages3d>
<package3d name="W237-102" urn="urn:adsk.eagle:package:10688/1" type="box" library_version="1">
<description>WAGO SCREW CLAMP</description>
</package3d>
</packages3d>
<symbols>
<symbol name="KL" urn="urn:adsk.eagle:symbol:10675/1" library_version="1">
<circle x="1.27" y="0" radius="1.27" width="0.254" layer="94"/>
<text x="0" y="0.889" size="1.778" layer="95" rot="R180">&gt;NAME</text>
<pin name="KL" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
<symbol name="KL+V" urn="urn:adsk.eagle:symbol:10673/1" library_version="1">
<circle x="1.27" y="0" radius="1.27" width="0.254" layer="94"/>
<text x="-2.54" y="-3.683" size="1.778" layer="96">&gt;VALUE</text>
<text x="0" y="0.889" size="1.778" layer="95" rot="R180">&gt;NAME</text>
<pin name="KL" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="W237-102" urn="urn:adsk.eagle:component:10702/1" prefix="X" uservalue="yes" library_version="1">
<description>&lt;b&gt;WAGO SCREW CLAMP&lt;/b&gt;</description>
<gates>
<gate name="-1" symbol="KL" x="0" y="5.08" addlevel="always"/>
<gate name="-2" symbol="KL+V" x="0" y="0" addlevel="always"/>
</gates>
<devices>
<device name="" package="W237-102">
<connects>
<connect gate="-1" pin="KL" pad="1"/>
<connect gate="-2" pin="KL" pad="2"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:10688/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="237-102" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="70K9898" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="U$1" library="ESP12E_DEVKIT" deviceset="ESP12E_DEVKIT" device=""/>
<part name="IC1" library="microchip" library_urn="urn:adsk.eagle:library:294" deviceset="MCP23017" device="SP" package3d_urn="urn:adsk.eagle:package:20799/2"/>
<part name="Q1" library="transistor-npn" library_urn="urn:adsk.eagle:library:398" deviceset="BC548*" device="" package3d_urn="urn:adsk.eagle:package:28989/2"/>
<part name="R1" library="eagle-ltspice" library_urn="urn:adsk.eagle:library:217" deviceset="R" device="0207/10" package3d_urn="urn:adsk.eagle:package:13275/1" value="4K7"/>
<part name="R2" library="eagle-ltspice" library_urn="urn:adsk.eagle:library:217" deviceset="R" device="0207/10" package3d_urn="urn:adsk.eagle:package:13275/1" value="4K7"/>
<part name="J1" library="pinhead" library_urn="urn:adsk.eagle:library:325" deviceset="PINHD-1X8" device="" package3d_urn="urn:adsk.eagle:package:22409/2" value="RELAYBRD"/>
<part name="JP1" library="jumper" library_urn="urn:adsk.eagle:library:252" deviceset="JP1E" device="" package3d_urn="urn:adsk.eagle:package:15455/1" value="5v"/>
<part name="X1" library="con-wago-500" library_urn="urn:adsk.eagle:library:195" deviceset="W237-102" device="" package3d_urn="urn:adsk.eagle:package:10688/1" value="Sw1"/>
<part name="X2" library="con-wago-500" library_urn="urn:adsk.eagle:library:195" deviceset="W237-102" device="" package3d_urn="urn:adsk.eagle:package:10688/1" value="Sw7"/>
<part name="X3" library="con-wago-500" library_urn="urn:adsk.eagle:library:195" deviceset="W237-102" device="" package3d_urn="urn:adsk.eagle:package:10688/1" value="Sw5"/>
<part name="X4" library="con-wago-500" library_urn="urn:adsk.eagle:library:195" deviceset="W237-102" device="" package3d_urn="urn:adsk.eagle:package:10688/1" value="Sw4"/>
<part name="X5" library="con-wago-500" library_urn="urn:adsk.eagle:library:195" deviceset="W237-102" device="" package3d_urn="urn:adsk.eagle:package:10688/1" value="Sw3"/>
<part name="X6" library="con-wago-500" library_urn="urn:adsk.eagle:library:195" deviceset="W237-102" device="" package3d_urn="urn:adsk.eagle:package:10688/1" value="Sw2"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="U$1" gate="G$1" x="68.58" y="76.2"/>
<instance part="IC1" gate="G$1" x="68.58" y="15.24"/>
<instance part="Q1" gate="G$1" x="30.48" y="45.72" rot="MR0"/>
<instance part="R1" gate="G$1" x="30.48" y="10.16"/>
<instance part="R2" gate="G$1" x="12.7" y="12.7"/>
<instance part="J1" gate="A" x="96.52" y="30.48"/>
<instance part="JP1" gate="A" x="129.54" y="109.22" rot="R270"/>
<instance part="X1" gate="-1" x="142.24" y="43.18" rot="R180"/>
<instance part="X1" gate="-2" x="142.24" y="38.1" rot="R180"/>
<instance part="X2" gate="-1" x="142.24" y="-7.62" rot="R180"/>
<instance part="X2" gate="-2" x="142.24" y="-12.7" rot="R180"/>
<instance part="X3" gate="-1" x="142.24" y="2.54" rot="R180"/>
<instance part="X3" gate="-2" x="142.24" y="-2.54" rot="R180"/>
<instance part="X4" gate="-1" x="142.24" y="12.7" rot="R180"/>
<instance part="X4" gate="-2" x="142.24" y="7.62" rot="R180"/>
<instance part="X5" gate="-1" x="142.24" y="22.86" rot="R180"/>
<instance part="X5" gate="-2" x="142.24" y="17.78" rot="R180"/>
<instance part="X6" gate="-1" x="142.24" y="33.02" rot="R180"/>
<instance part="X6" gate="-2" x="142.24" y="27.94" rot="R180"/>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="D0"/>
<wire x1="86.36" y1="93.98" x2="86.36" y2="104.14" width="0.1524" layer="91"/>
<wire x1="86.36" y1="104.14" x2="33.02" y2="104.14" width="0.1524" layer="91"/>
<pinref part="Q1" gate="G$1" pin="B"/>
<wire x1="33.02" y1="104.14" x2="33.02" y2="45.72" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="Q1" gate="G$1" pin="E"/>
<wire x1="27.94" y1="40.64" x2="27.94" y2="30.48" width="0.1524" layer="91"/>
<pinref part="IC1" gate="G$1" pin="!RESET"/>
<wire x1="27.94" y1="30.48" x2="55.88" y2="30.48" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="SCL"/>
<pinref part="R2" gate="G$1" pin="2"/>
<wire x1="55.88" y1="12.7" x2="43.18" y2="12.7" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="D1"/>
<wire x1="43.18" y1="12.7" x2="17.78" y2="12.7" width="0.1524" layer="91"/>
<wire x1="86.36" y1="91.44" x2="106.68" y2="91.44" width="0.1524" layer="91"/>
<wire x1="106.68" y1="91.44" x2="106.68" y2="-17.78" width="0.1524" layer="91"/>
<wire x1="106.68" y1="-17.78" x2="43.18" y2="-17.78" width="0.1524" layer="91"/>
<wire x1="43.18" y1="-17.78" x2="43.18" y2="12.7" width="0.1524" layer="91"/>
<junction x="43.18" y="12.7"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="SDA"/>
<pinref part="R1" gate="G$1" pin="2"/>
<wire x1="55.88" y1="10.16" x2="45.72" y2="10.16" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="D2"/>
<wire x1="45.72" y1="10.16" x2="35.56" y2="10.16" width="0.1524" layer="91"/>
<wire x1="86.36" y1="88.9" x2="104.14" y2="88.9" width="0.1524" layer="91"/>
<wire x1="104.14" y1="88.9" x2="104.14" y2="-15.24" width="0.1524" layer="91"/>
<wire x1="104.14" y1="-15.24" x2="45.72" y2="-15.24" width="0.1524" layer="91"/>
<wire x1="45.72" y1="-15.24" x2="45.72" y2="10.16" width="0.1524" layer="91"/>
<junction x="45.72" y="10.16"/>
</segment>
</net>
<net name="N$7" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="D5"/>
<wire x1="86.36" y1="76.2" x2="96.52" y2="76.2" width="0.1524" layer="91"/>
<wire x1="96.52" y1="76.2" x2="96.52" y2="50.8" width="0.1524" layer="91"/>
<wire x1="96.52" y1="50.8" x2="48.26" y2="50.8" width="0.1524" layer="91"/>
<wire x1="48.26" y1="50.8" x2="48.26" y2="25.4" width="0.1524" layer="91"/>
<pinref part="IC1" gate="G$1" pin="INTA"/>
<wire x1="48.26" y1="25.4" x2="55.88" y2="25.4" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPA0"/>
<pinref part="J1" gate="A" pin="3"/>
<wire x1="81.28" y1="35.56" x2="93.98" y2="35.56" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$11" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPA1"/>
<pinref part="J1" gate="A" pin="4"/>
<wire x1="81.28" y1="33.02" x2="93.98" y2="33.02" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$12" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPA2"/>
<pinref part="J1" gate="A" pin="5"/>
<wire x1="81.28" y1="30.48" x2="93.98" y2="30.48" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPA3"/>
<pinref part="J1" gate="A" pin="6"/>
<wire x1="81.28" y1="27.94" x2="93.98" y2="27.94" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$14" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPA4"/>
<pinref part="J1" gate="A" pin="7"/>
<wire x1="81.28" y1="25.4" x2="93.98" y2="25.4" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$15" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPA5"/>
<pinref part="J1" gate="A" pin="8"/>
<wire x1="81.28" y1="22.86" x2="93.98" y2="22.86" width="0.1524" layer="91"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="GND@3"/>
<wire x1="127" y1="10.16" x2="127" y2="12.7" width="0.1524" layer="91"/>
<wire x1="127" y1="12.7" x2="127" y2="22.86" width="0.1524" layer="91"/>
<wire x1="127" y1="22.86" x2="127" y2="33.02" width="0.1524" layer="91"/>
<wire x1="127" y1="33.02" x2="127" y2="38.1" width="0.1524" layer="91"/>
<wire x1="127" y1="38.1" x2="127" y2="43.18" width="0.1524" layer="91"/>
<wire x1="127" y1="43.18" x2="127" y2="60.96" width="0.1524" layer="91"/>
<wire x1="127" y1="60.96" x2="127" y2="106.68" width="0.1524" layer="91"/>
<wire x1="86.36" y1="60.96" x2="127" y2="60.96" width="0.1524" layer="91"/>
<junction x="127" y="60.96"/>
<pinref part="IC1" gate="G$1" pin="VSS"/>
<wire x1="55.88" y1="-5.08" x2="50.8" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="50.8" y1="-5.08" x2="50.8" y2="-12.7" width="0.1524" layer="91"/>
<wire x1="50.8" y1="-12.7" x2="99.06" y2="-12.7" width="0.1524" layer="91"/>
<wire x1="99.06" y1="-12.7" x2="99.06" y2="10.16" width="0.1524" layer="91"/>
<wire x1="99.06" y1="10.16" x2="127" y2="10.16" width="0.1524" layer="91"/>
<pinref part="IC1" gate="G$1" pin="A0"/>
<wire x1="55.88" y1="5.08" x2="50.8" y2="5.08" width="0.1524" layer="91"/>
<wire x1="50.8" y1="5.08" x2="50.8" y2="2.54" width="0.1524" layer="91"/>
<junction x="50.8" y="-5.08"/>
<pinref part="IC1" gate="G$1" pin="A1"/>
<wire x1="50.8" y1="2.54" x2="50.8" y2="0" width="0.1524" layer="91"/>
<wire x1="50.8" y1="0" x2="50.8" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="55.88" y1="2.54" x2="50.8" y2="2.54" width="0.1524" layer="91"/>
<junction x="50.8" y="2.54"/>
<pinref part="IC1" gate="G$1" pin="A2"/>
<wire x1="55.88" y1="0" x2="50.8" y2="0" width="0.1524" layer="91"/>
<junction x="50.8" y="0"/>
<pinref part="J1" gate="A" pin="2"/>
<wire x1="93.98" y1="38.1" x2="127" y2="38.1" width="0.1524" layer="91"/>
<junction x="127" y="38.1"/>
<pinref part="JP1" gate="A" pin="2"/>
<pinref part="X1" gate="-1" pin="KL"/>
<wire x1="137.16" y1="43.18" x2="127" y2="43.18" width="0.1524" layer="91"/>
<junction x="127" y="43.18"/>
<pinref part="X6" gate="-1" pin="KL"/>
<wire x1="137.16" y1="33.02" x2="127" y2="33.02" width="0.1524" layer="91"/>
<junction x="127" y="33.02"/>
<pinref part="X5" gate="-1" pin="KL"/>
<wire x1="137.16" y1="22.86" x2="127" y2="22.86" width="0.1524" layer="91"/>
<junction x="127" y="22.86"/>
<pinref part="X4" gate="-1" pin="KL"/>
<wire x1="137.16" y1="12.7" x2="127" y2="12.7" width="0.1524" layer="91"/>
<junction x="127" y="12.7"/>
<pinref part="X3" gate="-1" pin="KL"/>
<wire x1="137.16" y1="2.54" x2="127" y2="2.54" width="0.1524" layer="91"/>
<wire x1="127" y1="2.54" x2="127" y2="10.16" width="0.1524" layer="91"/>
<junction x="127" y="10.16"/>
<pinref part="X2" gate="-1" pin="KL"/>
<wire x1="137.16" y1="-7.62" x2="127" y2="-7.62" width="0.1524" layer="91"/>
<wire x1="127" y1="-7.62" x2="127" y2="2.54" width="0.1524" layer="91"/>
<junction x="127" y="2.54"/>
</segment>
</net>
<net name="VCC" class="0">
<segment>
<wire x1="129.54" y1="109.22" x2="127" y2="109.22" width="0.1524" layer="91"/>
<wire x1="127" y1="109.22" x2="0" y2="109.22" width="0.1524" layer="91"/>
<wire x1="0" y1="109.22" x2="0" y2="58.42" width="0.1524" layer="91"/>
<pinref part="Q1" gate="G$1" pin="C"/>
<wire x1="0" y1="58.42" x2="0" y2="50.8" width="0.1524" layer="91"/>
<wire x1="0" y1="50.8" x2="0" y2="35.56" width="0.1524" layer="91"/>
<wire x1="0" y1="35.56" x2="0" y2="12.7" width="0.1524" layer="91"/>
<wire x1="0" y1="12.7" x2="0" y2="10.16" width="0.1524" layer="91"/>
<wire x1="27.94" y1="50.8" x2="0" y2="50.8" width="0.1524" layer="91"/>
<junction x="0" y="50.8"/>
<pinref part="R2" gate="G$1" pin="1"/>
<wire x1="7.62" y1="12.7" x2="0" y2="12.7" width="0.1524" layer="91"/>
<junction x="0" y="12.7"/>
<pinref part="R1" gate="G$1" pin="1"/>
<wire x1="25.4" y1="10.16" x2="0" y2="10.16" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="VIN"/>
<wire x1="50.8" y1="58.42" x2="0" y2="58.42" width="0.1524" layer="91"/>
<junction x="0" y="58.42"/>
<pinref part="IC1" gate="G$1" pin="VDD"/>
<wire x1="0" y1="35.56" x2="53.34" y2="35.56" width="0.1524" layer="91"/>
<junction x="0" y="35.56"/>
<pinref part="J1" gate="A" pin="1"/>
<wire x1="53.34" y1="35.56" x2="55.88" y2="35.56" width="0.1524" layer="91"/>
<wire x1="93.98" y1="40.64" x2="53.34" y2="40.64" width="0.1524" layer="91"/>
<wire x1="53.34" y1="40.64" x2="53.34" y2="35.56" width="0.1524" layer="91"/>
<junction x="53.34" y="35.56"/>
<pinref part="JP1" gate="A" pin="1"/>
<junction x="127" y="109.22"/>
</segment>
</net>
<net name="N$5" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPB0"/>
<wire x1="81.28" y1="12.7" x2="119.38" y2="12.7" width="0.1524" layer="91"/>
<wire x1="119.38" y1="12.7" x2="119.38" y2="35.56" width="0.1524" layer="91"/>
<pinref part="X1" gate="-2" pin="KL"/>
<wire x1="119.38" y1="35.56" x2="137.16" y2="35.56" width="0.1524" layer="91"/>
<wire x1="137.16" y1="35.56" x2="137.16" y2="38.1" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPB1"/>
<wire x1="81.28" y1="10.16" x2="86.36" y2="10.16" width="0.1524" layer="91"/>
<wire x1="86.36" y1="10.16" x2="86.36" y2="15.24" width="0.1524" layer="91"/>
<wire x1="86.36" y1="15.24" x2="116.84" y2="15.24" width="0.1524" layer="91"/>
<wire x1="116.84" y1="15.24" x2="116.84" y2="27.94" width="0.1524" layer="91"/>
<pinref part="X6" gate="-2" pin="KL"/>
<wire x1="116.84" y1="27.94" x2="137.16" y2="27.94" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPB2"/>
<wire x1="81.28" y1="7.62" x2="121.92" y2="7.62" width="0.1524" layer="91"/>
<wire x1="121.92" y1="7.62" x2="121.92" y2="17.78" width="0.1524" layer="91"/>
<pinref part="X5" gate="-2" pin="KL"/>
<wire x1="121.92" y1="17.78" x2="137.16" y2="17.78" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$9" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPB3"/>
<pinref part="X4" gate="-2" pin="KL"/>
<wire x1="81.28" y1="5.08" x2="137.16" y2="5.08" width="0.1524" layer="91"/>
<wire x1="137.16" y1="5.08" x2="137.16" y2="7.62" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$16" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPB4"/>
<wire x1="81.28" y1="2.54" x2="121.92" y2="2.54" width="0.1524" layer="91"/>
<wire x1="121.92" y1="2.54" x2="121.92" y2="-2.54" width="0.1524" layer="91"/>
<pinref part="X3" gate="-2" pin="KL"/>
<wire x1="121.92" y1="-2.54" x2="137.16" y2="-2.54" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$17" class="0">
<segment>
<pinref part="IC1" gate="G$1" pin="GPB5"/>
<wire x1="81.28" y1="0" x2="114.3" y2="0" width="0.1524" layer="91"/>
<wire x1="114.3" y1="0" x2="114.3" y2="-12.7" width="0.1524" layer="91"/>
<pinref part="X2" gate="-2" pin="KL"/>
<wire x1="114.3" y1="-12.7" x2="137.16" y2="-12.7" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
<errors>
<approved hash="104,1,55.88,35.56,IC1,VDD,VCC,,,"/>
<approved hash="104,1,55.88,-5.08,IC1,VSS,GND,,,"/>
</errors>
</schematic>
</drawing>
<compatibility>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports URNs for individual library
assets (packages, symbols, and devices). The URNs of those assets
will not be understood (or retained) with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports the association of 3D packages
with devices in libraries, schematics, and board files. Those 3D
packages will not be understood (or retained) with this version.
</note>
</compatibility>
</eagle>

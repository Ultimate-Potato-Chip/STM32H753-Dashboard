Sa e f Te c h n o l o g y Li m i t e d
信创天源科技有限公司
SF-RD-001
REV：A PAGE：1/16
DATE：2022-05-14
SPEC TITLE
DOCUMENT CONTROL SPECIFICATION
SPECIFICATION
FOR
4.0” 720X720 TFT LCD MODULE
MODEL NO.: SF-TO400XC-8996A2-N
Customer Approval:
□Accept □ Reject
SAEF TECHNOLOGY LTD SIGNATURE DATE
PREPARED BY Z.K. CHEN 2022-05-14
CHECKED BY G.Y. CHEN 2022-05-14
APPROVED BY Q.Q. YE 2022-05-14
Factory Address:
25/B,Yuanyiyuan Industrial Zone, 2nd QianJin Road, Xixiang BaoAn, ShenZhen China 518126
Tel:+86 0755-2370 6380
Cell/wechat: +86 135 0298 3321
Http://www.saefdisplay.com
Email: sales2@saef.com.cn


---

2 / 16
Table of Contents
1.Record of Revision............................................................................................................3
2.General Specifications..................................................................................................... 4
3.Pin Assignment.................................................................................................................5
4.Absolute Maximum Ratings.............................................................................................6
5.Electrical Characteristics..................................................................................................6
6.Timing Chart.....................................................................................................................8
7.Mechanical Drawing........................................................................................................11
8.Optical Characteristics.....................................................................................................12
9.Environmental / Reliability Test.......................................................................................15
10.Precautions for Use of LCD Modules.............................................................................16
Model:SF-TO400XC-8996A2-N

---

3 / 16
1.Record of Revision
Rev. Issued Date Description Editor
1.0 2022-05-14 First release HOU
Model:SF-TO400XC-8996A2-N

---

4 / 16
2.General Specifications
Note 1: Viewing direction for best image quality is different from TFT definition. There is a 180°shift.
Note 2: ROHS compliant.
Item Specification Unit
LCD size 4.0 inch
Display Mode Normally Black --
Resolution 720(RGB)x720 Pixel
Pixel pitch 0.047*RGB*0.141 mm
Pixel Arrangement RGB Vertical Stripe
Viewing direction IPS -
Module outline dimension 105.3(H)*109.6(V)*2.3(D) mm
LCD AA 101.52*101.52 mm
TP VA - mm
Colors 262K -
Driver IC NV3052CGRB -
Driver IC RAM Size - -
Interface 3SPI+18RGB --
Backlight White LED --
Touch IC - --
Surface hardness - --
Touch structure - --
Cover lens - --
Colors - --
Operating Temperature -20℃~ +70℃ --
Storage Temperature -30℃~ +80℃ --
Model:SF-TO400XC-8996A2-N

---

5 / 16
3.Pin Assignment
PIN Symbol Description Remark
1 LEDA LED ANODE
2 LEDK LED CATHODE
3 LEDK LED CATHODE
4 GND Ground
5 VCI LCD analog power supply (3.3V)
6 RESET Reset Signal
7 NC Not connect
8 NC Not connect
9 SDA SPI Data signal
10 SCK SPI Clock signal
11 CS SPI Chip select signal
12 PCLK RGB dot clock signal
13 DE RGB data enable signal
14 VSYNC RGB frame synchronizing signal
15 HSYNC RGB line synchronizing signal
16-33 DB0-DB17 RGB data(B2-B7,G2-G7,R2-R7)
34 GND Ground
35 TP_INT Touch Interrupt
36 TP_SDA Touch IIC Data signal
37 TP_SCL Touch IIC Clock signal
38 TP_RESET Touch Reset Signal
39 TP_VCI Touch Power supply
40 TP_GND Touch Ground
Model:SF-TO400XC-8996A2-N

---

6 / 16
4.Absolute Maximum Ratings
5.Electrical Characteristics
5.1Recommended Operating Condition
VCI=3.3V，GND=0V，Ta = 25℃
Item Symbol Min. Max. Unit Remark
Power Voltage
VCI -0.30 +3.3 V
IOVCC - - V
TP_VCI / / V
TP_IOVCC / / V
Operating Temperature Top -20.0 70.0 ℃
Storage Temperature Tst -30.0 80.0 ℃
Operating and Storage
Humidity Hstg 10% 90% %(RH)
Item Symbol Min. TYP. Max. Unit Remark
Digital supply
Voltage IOVCC - - - V
Analog supply
Voltage VCI 2.5 2.8 3.3 V
TP Power TP_VCI / / / V
TP Power TP_IOVCC / / / V NOTES
Input
Signal
Voltage
Low
Level VIL 0 - 0.3 x
IOVCC V
High
Level VIH
0.7 x
IOVCC - IOVCC V
Current of digital
supply voltage IIOVCC - / / Ma VCI=3.3V, color bar
pattern
Current of analog
supply voltage IVCI - / / Ma
Model:SF-TO400XC-8996A2-N

---

7 / 16
5.2 Backlight Unit Driving Condition
Note1: The LED driving condition is defined for each module.
Note2: When LCM is operated, the stable forward current should be inputted. And forward voltage is
for reference only.
Note3: Optical performance should be evaluated at Ta=25℃ When LED is driven at high current, high
ambient temperature & humidity condition. The life time of LED will be reduced. Operating life means
brightness goes down to 50% initial brightness. Typical operating life time is estimated data.
Note4: The LED driving condition is defined for each LED module.
Ite
m
Symbol Min. TYP. Max. Unit Remark
Forward Current IF - 40 -
Forward Current Voltag
mA
e VF - 30 - V
Backlight Power
Consumption WBL - 1200 -
Operating Life Tim
mW
e -- 20000 -- -- hrs Note 2, Note 3
Model:SF-TO400XC-8996A2-N

---

8 / 16
6.Timing Characteristics
6.1 RGB Interface
Model:SF-TO400XC-8996A2-N

---

9 / 16
6.2 Parallel 18-bit RGB Input Timing Table
Parallel 18-bit RGB Input Timing (VDDI= 1.8V, PVDD=VDD= 3.3V, AGND= 0V, TA=25。C)
720RGB X 720 Resolution Timing Table
Item Symbol Min. Typ. Max. Unit Remark
DCLK Frequency Fclk 33 49 - MHz
HSYNC
Period Time Th 730 1080 - DCLK
Display Period Thdisp 720 DCLK
Back Porch Thbp 4 120 255 DCLK
Front Porch Thfp 4 120 255 DCLK
Pulse Width Thw 2 120 255 DCLK
VSYNC
Period Time Tv 738 756 776 HSYNC
Display Period Tvdisp 720 HSYNC
Back Porch Tvbp 8 16 24 HSYNC
Front Porch Tvfp 8 16 24 HSYNC
Pulse Width Tvw 2 4 8 HSYNC
Note: 1. The minimum blanking time depends on the GIP timing of the panel specification
2. To ensure the compatibility of different panels, it is recommended to use the typical setting.
Model:SF-TO400XC-8996A2-N

---

10 / 16
6.3 Power On/Off Timing
6.4 Reset timing characteristics
Model:SF-TO400XC-8996A2-N

---

7.Mechanical Drawing
 更改内容 日 期 标示签 暑
请  签 回  此  图 承认 承认日期 客户名称
 SF-TO400XC-8996A2-NEDITION:(版本号 SF'S CODE:(信创料号))
CUSTOMER'S CODE:(客户型号)EDITION:(版本号)
SPECIFICATION                          规  格 APPROVED:(批准) CHECKED:(检查) DESIGN:(设计)
图纸视角: 1/1 Page:(页数) 2022.04.27 Date:(日期)
LED CIRCUIT DIAGRAM:
正视图侧视图背视图
      
A
K1
K2
TP_RESET
TP_SCL
TP_SDA
TP_INT
GND
DB0~DB17
HSYNC
CS
SCK
SDA
NC
NC
RESET
VCI
GND
LEDK2
LEDK1
LEDA
8
7
6
5
4
3
2
1
SYMBLE PIN:
38
37
36
35
34
16~33
15
14
13
12
11
10
9
40
39
GND
TP_VCI
VSYNC
DE
PCLK
接口说明
1、VCI供电3.3V
2、所有IO口电压也要３．３V
101.52<AA>
105<CF/TFT>
101.52<AA>
106.55<CF>
109.265<TFT>
109.6±0.15<BL>
105.3±0.15<BL>
PI 补强
元件区
AGAG
钢片补强T=0.2MM接地
20.5±0.1
19.5±0.05
0.3
0.5
3
4.5
26±0.5
1.89
1.892
0.3±0.03
2.3±0.15
17
单层区
140401
K A
XXXX/XX/XX
AG
散热石墨片
T=0.1±0.02 MM
Saef Technology Limited
Model:SF-TO400XC-8996A2-N11 / 16

---

12 / 16
8.Optical Characteristics
Test Conditions:
1.IF= 40 Ma, VF=30V and the ambient temperature is 25±2℃.humidity is 65±7%
2.The test systems refer to Note 1 and Note 2.
Item Symbol Condition Min. TYP. Max. Unit Remark
View Angles
θT
CR≧10
80 85 -
Degree Note 2
θB 80 85 -
θL 80 85 -
θR 80 85 -
Contrast Ratio CR θ=0° 1000 1200 -
Note1
Note3
Response Time
TON
25℃ 30 35 - ms
Note1
Note4TOFF
Chromaticity White
x
Backlight is
on
- - - Note1
Note5y - - -
Uniformity U 80 85 - %
Note1
Note6
NTSC 45 50 - % Note 5
Luminance L - 800 - CD/m2
Note1
Note7
Model:SF-TO400XC-8996A2-N

---

13 / 16
Note 1: Definition of optical measurement system.
Properties are measured at the center point of the LCD screen. All input terminals LCD panel must be
ground when measuring the center area of the panel.
Photo detector
Field
TFT LCD Module 500mm
LCD Panel
The center of the screen
Note 2: Definition of viewing angle range and measurement system.
Viewing angle is measured at the center point of the LCD by CONOSCOPE(ergo-80)。
Note 3: Definition of contrast ratio
Model:SF-TO400XC-8996A2-N

---

14 / 16
Note 4: Definition of response time
The response time is defined as the LCD optical switching time interval between “White” state and
“Black” state. Rise time (TON) is the time between photo detector output intensity changed from 90%
to 10%. And fall time (TOFF) is the time between photo detector output intensity changed from 10% to
90%.
Note 5: Definition of color chromaticity (CIE1931)
Color coordinates measured at center point ofLCD.
Note 6: Definition of luminance uniformity
Active area is divided into 9 measuring areas (Refer Fig. 2). Every measuring point is placed at the center
of each measuring area.
Luminance Uniformity (U) = Luminance min / Luminance max
L-------Active area length W-----Active area width
Luminance max: The measured Maximum luminance of all measurement position. Luminance min: The measured
Minimum luminance of all measurement position.
Note 7: Definition of luminance:
Measure the luminance of white state at center point
Model:SF-TO400XC-8996A2-N

---

15 / 16
9.Environmental / Reliability Test
Note1: Ts is the temperature of panel’s surface.
Note2: Ta is the ambient temperature of samples
No. Items Condition Inspection after test
1 High Temperature
Storage T = 80℃ for 96 hr
Inspection after
4 hours storage at room
temperature, the sample
shall be free from defects:
1.Air bubble in the LCD
2.Seal leak;
3.Non-display; 4.missing
segments; 5.Glass crack;
6.Current IDD is twice
higher than initial value.
2 Low Temperature
Storage T = -30℃ for 96 hr
3 High Temperature
Operating
T = 70℃ for 96 hr
4 Low Temperature
Operating
T = -20℃ for 96 hr
(But no condensation of dew)
5
High Temp. and
High Humidity
Operating
T = 60℃ /90% for 96 hr (But no
condensation dew)
6 Thermal Shock -20℃~25~70℃×10cycles (30min.)
(5min.) (30min.)
7 ESD Voltage:±2KV R: 330Ω C:
150pF Air discharge, 10time
Model:SF-TO400XC-8996A2-N

---

16 / 16
10.Precautions for Use of LCD Modules Handling Precautions
10.1Handling Precautions
10.1.1 The display panel is made of glass. Do not subject it to a mechanical shock by dropping it
from a high place, etc.
10.1.2 If the display panel is damaged and the liquid crystal substance inside it leaks out, be sure
not to get any in your mouth, if the substance comes into contact with your skin or clothes,
promptly wash it off using soap and water.
10.1.3 Do not apply excessive force to the display surface or the adjoining areas since this may
cause the color tone to vary.
10.1.4 The Polarizer covering the display surface of the LCD module is soft and easily scratched.
Handle this Polarizer carefully.
10.1.5 If the display surface is contaminated, breathe on the surface and gently wipe it with a soft
dry cloth. If still not completely clear, moisten cloth with one of the following solvents:
－ isopropyl alcohol
－ Ethyl alcohol
Solvents other than those mentioned above may damage the Polarizer. Especially, do not use the
following:
－ Water
－ Ketone
－ Aromatic solvents
10.1.6 Do not attempt to disassemble the LCD Module.
10.1.7 If the logic circuit power is off, do not apply the input signals.
10.1.8 To prevent destruction of the elements by static electricity, be careful to maintain an
optimum work environment.
10.1.8.1 Be sure to ground the body when handling the LCD Modules.
Tools required for assembly, such as soldering irons, must be properly ground.
10.1.8.2 To reduce the amount of static electricity generated, do not conduct assembly and
other work under dry conditions.
10.1.8.3 The LCD Module is coated with a film to protect the display surface. Be care when
peeling off this protective film since static electricity may be generated.
10.2 Storage Precautions
10.2.1When storing the LCD modules, avoid exposure to direct sunlight or to the light of
fluorescent lamps.
10.2.2The LCD modules should be stored under the storage temperature range. If the LCD modules
will be stored for a long time, the recommend condition is: Temperature : 0℃ ～ 40℃ Relatively
humidity: ≤80%
10.2.3The LCD modules should be stored in the room without acid, alkali and harmful gas.
10.3Transportation Precautions
The LCD modules should be no falling and violent shocking during transportation, and also should
avoid excessive press, water, damp and sunshine.
Model:SF-TO400XC-8996A2-N
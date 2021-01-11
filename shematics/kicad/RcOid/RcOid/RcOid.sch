EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "RcOid Ir Schaltung"
Date "2021-01-11"
Rev "2"
Comp "smart-devices.cf"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:+3.3V #PWR0105
U 1 1 5F9C8826
P 3950 1700
F 0 "#PWR0105" H 3950 1550 50  0001 C CNN
F 1 "+3.3V" H 3965 1873 50  0000 C CNN
F 2 "" H 3950 1700 50  0001 C CNN
F 3 "" H 3950 1700 50  0001 C CNN
	1    3950 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5F9CDAD9
P 3950 1900
F 0 "R1" H 4020 1946 50  0000 L CNN
F 1 "2.4" H 4020 1855 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0411_L9.9mm_D3.6mm_P15.24mm_Horizontal" V 3880 1900 50  0001 C CNN
F 3 "~" H 3950 1900 50  0001 C CNN
	1    3950 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 1700 3950 1750
$Comp
L power:GND #PWR0106
U 1 1 5F9D178F
P 3950 3400
F 0 "#PWR0106" H 3950 3150 50  0001 C CNN
F 1 "GND" H 3955 3227 50  0000 C CNN
F 2 "" H 3950 3400 50  0001 C CNN
F 3 "" H 3950 3400 50  0001 C CNN
	1    3950 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 3300 3950 3400
$Comp
L Interface_Optical:TSOP531xx U1
U 1 1 5F9C2861
P 2600 2650
F 0 "U1" H 2588 3075 50  0000 C CNN
F 1 "VS1838B" H 2588 2984 50  0000 C CNN
F 2 "OptoDevice:Vishay_MINIMOLD-3Pin" H 2550 2275 50  0001 C CNN
F 3 "http://www.vishay.com/docs/82745/tsop531.pdf" H 3250 2950 50  0001 C CNN
	1    2600 2650
	1    0    0    -1  
$EndComp
$Comp
L LED:LD271 D1
U 1 1 5F9C3280
P 3950 2600
F 0 "D1" V 3946 2520 50  0000 R CNN
F 1 "TSAL6400" V 3855 2520 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm_Horizontal_O1.27mm_Z3.0mm_IRGrey" H 3950 2775 50  0001 C CNN
F 3 "http://www.alliedelec.com/m/d/40788c34903a719969df15f1fbea1056.pdf" H 3900 2600 50  0001 C CNN
	1    3950 2600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3950 2800 3950 2900
Wire Wire Line
	3950 1700 3000 1700
Wire Wire Line
	3000 1700 3000 2450
Connection ~ 3950 1700
Wire Wire Line
	3000 2850 3000 3400
Wire Wire Line
	3000 3400 3950 3400
Connection ~ 3950 3400
$Comp
L Connector:Conn_01x08_Male J1
U 1 1 5F9C5E95
P 2250 4350
F 0 "J1" H 2358 4831 50  0000 C CNN
F 1 "Wemos Left (5V)" H 2358 4740 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 2250 4350 50  0001 C CNN
F 3 "~" H 2250 4350 50  0001 C CNN
	1    2250 4350
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Male J2
U 1 1 5F9C8CB1
P 3350 4350
F 0 "J2" H 3458 4831 50  0000 C CNN
F 1 "Wemos Right (3.3V)" H 3458 4740 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 3350 4350 50  0001 C CNN
F 3 "~" H 3350 4350 50  0001 C CNN
	1    3350 4350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5F9C9BF7
P 2600 4150
F 0 "#PWR0101" H 2600 3900 50  0001 C CNN
F 1 "GND" H 2605 3977 50  0000 C CNN
F 2 "" H 2600 4150 50  0001 C CNN
F 3 "" H 2600 4150 50  0001 C CNN
	1    2600 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 4150 2600 4150
$Comp
L power:+3.3V #PWR0102
U 1 1 5F9CA6C7
P 4000 4050
F 0 "#PWR0102" H 4000 3900 50  0001 C CNN
F 1 "+3.3V" H 4015 4223 50  0000 C CNN
F 2 "" H 4000 4050 50  0001 C CNN
F 3 "" H 4000 4050 50  0001 C CNN
	1    4000 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 4050 4000 4050
Wire Wire Line
	3000 2650 3050 2650
Wire Wire Line
	3050 2650 3050 4250
Wire Wire Line
	3050 4250 2450 4250
$Comp
L Device:CP C1
U 1 1 5F9D204B
P 4650 3700
F 0 "C1" H 4532 3654 50  0000 R CNN
F 1 "2000µF" H 4532 3745 50  0000 R CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P2.50mm" H 4688 3550 50  0001 C CNN
F 3 "~" H 4650 3700 50  0001 C CNN
	1    4650 3700
	-1   0    0    1   
$EndComp
Wire Wire Line
	3950 3400 4650 3400
Wire Wire Line
	4650 3400 4650 3550
Wire Wire Line
	4650 3850 4650 4050
Wire Wire Line
	4650 4050 4000 4050
Connection ~ 4000 4050
Wire Wire Line
	3650 3100 3100 3100
Wire Wire Line
	3100 3100 3100 4350
Wire Wire Line
	3100 4350 2450 4350
$Comp
L Device:Q_NMOS_GDS Q1
U 1 1 5FA71831
P 3850 3100
F 0 "Q1" H 4054 3146 50  0000 L CNN
F 1 "IRLU024N" H 4054 3055 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-251-2-1EP_Horizontal_TabDown" H 4050 3200 50  0001 C CNN
F 3 "~" H 3850 3100 50  0001 C CNN
	1    3850 3100
	1    0    0    -1  
$EndComp
$Comp
L LED:LD274 D2
U 1 1 5FAC439D
P 3950 2200
F 0 "D2" V 3946 2121 50  0000 R CNN
F 1 "TSAL6400" V 3855 2121 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm_IRGrey" H 3950 2375 50  0001 C CNN
F 3 "http://pdf.datasheetcatalog.com/datasheet/siemens/LD274.pdf" H 3900 2200 50  0001 C CNN
	1    3950 2200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3950 2050 3950 2100
Wire Wire Line
	3950 2400 3950 2500
$Comp
L Jumper:SolderJumper_2_Open JP1
U 1 1 5FAC6616
P 3550 2250
F 0 "JP1" V 3504 2318 50  0000 L CNN
F 1 "SJ" V 3595 2318 50  0000 L CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm" H 3550 2250 50  0001 C CNN
F 3 "~" H 3550 2250 50  0001 C CNN
	1    3550 2250
	0    1    1    0   
$EndComp
Wire Wire Line
	3550 2100 3950 2100
Connection ~ 3950 2100
Wire Wire Line
	3550 2400 3950 2400
Connection ~ 3950 2400
$EndSCHEMATC

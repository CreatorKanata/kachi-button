# PCB Specification

Baseline: PCB v1 exported September 5, 2026. See [the release record](../../pcb/v1/README.md) for BOM, manufacturing files, and original verification results.

## Geometry and coordinate convention

The board has two copper layers, Top-side component placement, an intended FR-4 material and 1.6 mm thickness, and no corner mounting holes. Retention is by the enclosure's snap fits. Switch and connector mounting holes remain functional features.

The exported outline is 55 mm wide and approximately 41.22 mm high after R5 rounding. The original straight upper edge slopes slightly; do not replace it with an assumed exact 55 × 41 mm rounded rectangle. All four corners are tangent arcs of radius 5 mm. The Gerber outline and drill files define exact XY geometry; confirm board thickness in the fabrication order.

All coordinates below are millimeters in the export's XY frame: X increases to the right and Y toward the upper edge in a Top view. The straight left edge is X=0 and lower edge Y=0; their virtual intersection lies outside the rounded material. Bottom views are mirrored and must not be used to copy Top coordinates.

Placement coordinates are the export's **Mid X / Mid Y** values, with rotations as exported by EasyEDA. They are not automatically the center of a stem, USB mouth, or enclosure cutout. Derive those features from the footprint and 3D model. Use one documented transform when importing the STEP assembly into CAD.

## Exported component placement

| Reference | Function | X (mm) | Y (mm) | Rotation (degrees) |
| --- | --- | --- | --- | --- |
| SW1 | Upper key | 18.000 | 33.800 | 0 |
| SW2 | Lower-left key | 13.970 | 14.800 | 0 |
| SW3 | Lower-right key | 40.000 | 14.800 | 0 |
| J1 | USB-C, faces right | 50.000 | 31.000 | 90 |
| U1 | CH552E | 36.000 | 31.000 | 90 |
| U2 | SRV05-4 protection | 43.000 | 34.000 | 90 |
| D1 | Status LED | 50.927 | 22.733 | 0 |
| R3 | LED resistor | 48.006 | 22.733 | 0 |
| R1 | CC1 pull-down | 39.000 | 37.000 | 0 |
| R2 | CC2 pull-down | 39.000 | 35.000 | 0 |
| C1 | VBUS bypass | 32.500 | 31.500 | 90 |
| C2 | V33 bypass | 32.500 | 29.500 | 90 |
| C3 | VBUS bulk capacitor | 41.000 | 38.500 | 90 |

Source: [v1 pick-and-place workbook](../../pcb/v1/gerber/PickAndPlace_PCB1_2026-09-05.xlsx). All thirteen components are on Top. The three switches are through-hole; J1 combines surface contacts with mechanical anchors and is marked `SMD = No` in the export.

The user's placements are the baseline. Electronics are concentrated at upper right. J1 is in the upper half of the board, facing the right edge. D1 is below J1 and outside the keycap rectangles; R3 stays close to D1. Components may occupy a silk keycap rectangle only if their physical heights and the moving keycap remain compatible.

## Power and USB

The board is a **5 V USB-C sink without USB PD negotiation**. R1 and R2 independently connect CC1 and CC2 to GND through 5.1 kΩ. A compliant Type-C source detects the sink and supplies nominal 5 V; this circuit does not request 9, 12, 15, or 20 V. The resistors indicate attachment, not a negotiated power contract. [TI's USB Type-C guide](https://www.ti.com/lit/SLYY228) describes this 5 V sink arrangement.

VBUS feeds U1 VCC directly. There is no external regulator converting a higher input voltage to 5 V, and no PD controller. Power consumption and firmware USB power declarations must remain within the supported USB power budget; the CC resistors alone do not authorize arbitrary current draw.

J1's A6/B6 contacts join USB_DP, and A7/B7 join USB_DM. SBU contacts are unused. U2 protects USB and CC signals. C1 is 100 nF on VBUS, C2 is 100 nF on V33, and C3 is 1 µF on VBUS. V33 is the MCU's internal regulator bypass node, not an external power input.

## MCU and user I/O

| U1 pin | Net | Required function |
| --- | --- | --- |
| 1 / P1.4 | LED_K | LOW turns LED on |
| 2 / P1.5 | KEY_TOP | SW1 input, internal pull-up, pressed LOW |
| 3 / P1.6 | KEY_LEFT | SW2 input, internal pull-up, pressed LOW |
| 4 / P1.7 | KEY_RIGHT | SW3 input, internal pull-up, pressed LOW |
| 5 | RESET | Reset test access |
| 6 / P3.6 | USB_DP | USB D+ |
| 7 / P3.7 | USB_DM | USB D− |
| 8 | GND | Ground |
| 9 / VCC | VBUS | USB 5 V |
| 10 / V33 | V33 | Internal regulator bypass |

Each switch closes its GPIO to GND, without a key matrix or matrix diodes. Firmware provides debounce. The LED current path is VBUS → R3 (1 kΩ) → D1 → LED_K. No external crystal or USB series resistors are populated in v1. Refer to the [WCH CH552 datasheet](https://www.wch-ic.com/downloads/CH552DS1_PDF.html) when implementing reset, GPIO, and USB initialization; RESET must not be treated as a generic active-LOW reset or SWIO connection.

## Component information

The following table records all nine part types and thirteen components in the [v1 BOM](../../pcb/v1/gerber/BOM_Board1_Schematic1_2026-09-05.xlsx). Quantities are per board. Manufacturer part numbers identify the ordered devices; footprint names identify the PCB library geometry and may differ from the device name.

| References | Qty | Role / value | Manufacturer | Manufacturer part number | Package / mounting | LCSC part |
| --- | --- | --- | --- | --- | --- | --- |
| U1 | 1 | USB MCU; key scanning, HID, configuration, and LED control | WCH | CH552E | MSOP-10, SMD | [C967938](https://www.lcsc.com/product-detail/C967938.html) |
| SW1, SW2, SW3 | 3 | Mechanical key switches | HanElectricity | CPG151101D13 | MX-compatible, through-hole | [C49234235](https://www.lcsc.com/product-detail/C49234235.html) |
| J1 | 1 | USB-C power and USB 2.0 data connector | SHOU HAN | TYPE-C16PIN | 16-pin receptacle; SMD contacts and through-board anchors | [C393939](https://www.lcsc.com/product-detail/C393939.html) |
| D1 | 1 | Yellow-green status LED | YONGYUTAI | YLED0603YG | 0603, SMD | [C19171392](https://www.lcsc.com/product-detail/C19171392.html) |
| U2 | 1 | USB/CC ESD protection array | TECH PUBLIC | SRV05-4 | SOT-23-6, SMD | [C558418](https://www.lcsc.com/product-detail/C558418.html) |
| R1, R2 | 2 | Independent CC pull-downs, 5.1 kΩ | UNI-ROYAL | 0402WGF5101TCE | 0402, SMD | [C25905](https://www.lcsc.com/product-detail/C25905.html) |
| R3 | 1 | LED current-limiting resistor, 1 kΩ | UNI-ROYAL | 0402WGF1001TCE | 0402, SMD | [C11702](https://www.lcsc.com/product-detail/C11702.html) |
| C1, C2 | 2 | VBUS and V33 bypass capacitors, 100 nF | SAMSUNG | CL05B104KO5NNNC | 0402, SMD | [C1525](https://www.lcsc.com/product-detail/C1525.html) |
| C3 | 1 | VBUS bulk capacitor, 1 µF | SAMSUNG | CL05A105KA5NQNC | 0402, SMD | [C52923](https://www.lcsc.com/product-detail/C52923.html) |

### MCU: U1

The CH552E is the single controller for the three direct switch inputs, USB keyboard/configuration interfaces, and active-LOW status LED. It runs from VBUS in this design, with the V33 bypass capacitor and no external crystal. The exact pin-to-net mapping is recorded above. Firmware functionality is specified in [the firmware document](firmware-spec.md); its presence in the circuit is not evidence that the firmware has been implemented.

### Mechanical buttons: SW1–SW3

All three positions use the same CPG151101D13 switch: SW1 is the upper button, SW2 is lower left, and SW3 is lower right. They use MX-compatible cross stems and are soldered through the PCB. The BOM's footprint is named `KEY-TH_CPG151101D21X`; this is the library footprint identifier, not a substitute ordering part number. Check the actual CPG151101D13 against that footprint and the 3D model before assembly.

The keycaps are separate custom parts, not included in this electronic BOM. Their confirmed external sizes are 35 × 18 × 10 mm for the upper key and 25 × 18 × 10 mm for each lower key. A nominal 1.3 mm switch cross thickness was supplied by the user; the current printed keycap prototype uses a 1.5 mm cross-slot width for clearance. Verify fit with the actual switch and printing process rather than treating that clearance as a switch datasheet dimension.

### Connector, indicator, and protection

J1 is the only electrical USB port. Its shell anchors provide mechanical attachment, but the enclosure must still support cable handling loads. It receives standard USB-C 5 V power without a PD controller. Its mixed mounting explains why the placement export marks it `SMD = No`.

D1 sits below J1 with R3 nearby. Its anode is fed from VBUS through R3, and the MCU sinks current through LED_K to turn it on. U2 is the TECH PUBLIC SRV05-4 selected after the earlier protection candidate was unavailable; confirm the exact supplier part and pinout when considering any replacement.

All resistors and capacitors are 0402; D1 is 0603. LCSC availability was checked during the September 5 design session, not continuously. Recheck stock, order-quantity prices, assembly-service inventory, and mixed/through-hole assembly handling when ordering. The BOM remains the procurement source of truth for this version.

## Routing requirements

Use continuous schematic wires to show all component-to-MCU connections, functional grouping, and genuine power/GND symbols with correct net attributes. Preserve netlist connectivity when improving readability.

Both Top and Bottom have solid GND pours, with isolated islands disabled. Stitching vias connect both layers around the circuitry and perimeter; added stitching used 0.3 mm holes and 0.6 mm lands. The final count includes subsequent user changes and is not an EMC-optimized number.

Keep USB routes short with a continuous GND return, avoid unnecessary vias, and review the exported routes rather than assuming automatic routing achieves impedance control. Keep VBUS routing direct and suitably wide while respecting pad escapes and nearby GND clearance. R3–D1 routing should remain short. Rebuild pours after routing/outline changes and run DRC through the [EasyEDA development workflow](../pcb-development.md).

## Test access and silkscreen

Six exposed Top pads provide GND, VBUS, V33, USB_DP, USB_DM, and RESET. Their labels are GND, 5V, 3V3, D+, D−, and RST. Preserve the user's locations outside the Top logo and away from the buttons; a straight row is not required.

| Net | Label | X (mm) | Y (mm) |
| --- | --- | --- | --- |
| VBUS | 5V | 34.163 | 38.354 |
| V33 | 3V3 | 34.036 | 28.575 |
| GND | GND | 30.099 | 37.084 |
| USB_DP | D+ | 34.671 | 34.671 |
| USB_DM | D− | 43.815 | 26.162 |
| RESET | RST | 36.449 | 33.528 |

These pad positions are from the final design-session snapshot, not the component placement workbook. Nominal pad diameter is 1.016 mm. Check the actual manufactured openings before designing a pogo-pin fixture. Labels were placed with nominal 1 mm height and 0.15 mm strokes. Preserve artwork on both faces and avoid label overlap with exposed copper.

## Manufacturing and acceptance

Release directories are `pcb/v1/gerber/`, `pcb/v1/board-3d-data/`, and `pcb/v1/images/`. Native editable project data is not included in that release. The Gerber, placement, and STEP exports have differing timestamps; reconcile their revision before changing mechanical interfaces.

Design-session checks reported PCB DRC zero errors; schematic checks reported zero fatal/errors and three warnings. Those warnings and remaining prototype checks are documented in the v1 release record. New changes require netlist checks, DRC, regenerated pours, outline checks, and assembly preview review.

Bring-up must verify VBUS/GND isolation, nominal 5 V input, MCU operation, all three switches, LED polarity, programming access, and USB behavior with the purchased cable. DRC does not establish USB signal quality, ESD performance, case fit, or working firmware.

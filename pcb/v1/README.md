# Kachi Button PCB v1

Design record: September 5, 2026.

This directory contains the first hardware manufacturing export set. The product image and the user's final mechanical placement took priority over the preliminary AI-generated concept specification. In particular, v1 uses one upper large key and two lower small keys, rather than three keys in a horizontal row.

## Release files

| File | Purpose |
| --- | --- |
| [Gerber_PCB1_2026-09-05.zip](Gerber_PCB1_2026-09-05.zip) | Copper, silkscreen, solder mask, top paste, board outline, and drill data |
| [BOM_Board1_Schematic1_2026-09-05.xlsx](BOM_Board1_Schematic1_2026-09-05.xlsx) | Nine part types, thirteen components, with LCSC part numbers |
| [PickAndPlace_PCB1_2026-09-05.xlsx](PickAndPlace_PCB1_2026-09-05.xlsx) | Thirteen component positions in millimeters, rotations, and mounting-side information |
| [3D_PCB1_2026-09-05.step](3D_PCB1_2026-09-05.step) | STEP assembly for mechanical review |

The Gerber and placement exports are timestamped 19:38 on September 5; the STEP header is timestamped 21:15. These are the files supplied for this release. Their common origin in an identical saved EasyEDA revision has not been independently established. Native editable EasyEDA project files are not included in this directory.

## Circuit design

USB VBUS powers the CH552E directly. Three switches connect individual GPIO inputs to GND; firmware must enable internal pull-ups and debounce the inputs. A pressed key reads LOW. The status LED is driven by sinking current: VBUS → R3 (1 kΩ) → D1 → LED_K. A LOW output turns it on.

CC1 and CC2 each have an independent 5.1 kΩ pull-down to GND. The USB-C D+ contacts are joined together, as are the D− contacts. U2 provides protection for USB and CC lines. SBU contacts are unused.

C1 bypasses VBUS with 100 nF, C2 bypasses the MCU's V33 pin with 100 nF, and C3 adds 1 µF on VBUS. V33 is the internal regulator bypass node, not an external 3.3 V supply input. The design uses no external crystal or USB series resistors.

### MCU connections

| U1 pin | Signal | Connection |
| --- | --- | --- |
| 1 / P1.4 | LED_K | D1 cathode; active-LOW LED control |
| 2 / P1.5 | KEY_TOP | SW1, upper key |
| 3 / P1.6 | KEY_LEFT | SW2, lower-left key |
| 4 / P1.7 | KEY_RIGHT | SW3, lower-right key |
| 5 | RESET | Reset test pad |
| 6 / P3.6 | USB_DP | USB D+ and protection circuit |
| 7 / P3.7 | USB_DM | USB D− and protection circuit |
| 8 | GND | Ground planes and return connections |
| 9 / VCC | VBUS | USB 5 V supply |
| 10 / V33 | V33 | Internal regulator bypass capacitor C2 |

The schematic was organized by function, with continuous wires showing component-to-MCU connections and proper power and GND symbols. Network names supplement the wires. The power nets are VBUS and V33.

### Component selection

Parts were selected for low cost and available LCSC inventory during design on September 5, 2026. Availability and quantity pricing must be checked again when ordering; assembly-service inventory was not fully verified.

| References | Part / value | Package | LCSC |
| --- | --- | --- | --- |
| U1 | WCH CH552E | MSOP-10 | C967938 |
| U2 | TECH PUBLIC SRV05-4 | SOT-23-6 | C558418 |
| J1 | SHOU HAN TYPE-C16PIN | USB-C, mixed mounting | C393939 |
| SW1–SW3 | HanElectricity CPG151101D13 | MX-compatible, through-hole | C49234235 |
| D1 | YLED0603YG | 0603 | C19171392 |
| R1, R2 | 5.1 kΩ | 0402 | C25905 |
| R3 | 1 kΩ | 0402 | C11702 |
| C1, C2 | 100 nF | 0402 | C1525 |
| C3 | 1 µF | 0402 | C52923 |

U2 replaced an unavailable protection part. All resistors and capacitors are 0402; the LED is 0603. The BOM is the authoritative ordering list for this export.

## Mechanical layout and routing

- Two copper layers; all thirteen components are placed on Top. The intended board thickness is 1.6 mm, to be confirmed in fabrication settings.
- Approximately 55 × 41.24 mm overall. Four tangent R5 mm corners preserve the original edge positions, including the slight slope of the upper edge. Use the Gerber outline for exact geometry.
- No corner mounting holes: the enclosure is intended to retain the PCB with snap fits. Switch-related holes remain.
- Electronics occupy the upper-right area. Keycap rectangles on Top silkscreen are mechanical references; components may occupy their interior where mechanically compatible.
- J1 is at X50, Y31 mm, facing the right edge. D1 is below J1, outside the keycap rectangles, at X50.927, Y22.733 mm. R3 is nearby at X48.006, Y22.733 mm.
- R3 and D1 were routed together to shorten LED wiring. VBUS routing was adjusted around the connector and power loads, with pad escapes constrained by nearby GND clearance.
- Both sides use solid GND pours with isolated islands disabled. Stitching vias connect the layers around the perimeter and circuitry. Added stitching used 0.3 mm drills and 0.6 mm lands; subsequent user edits are retained. Via count was not optimized through EMC measurement.
- User-placed artwork and test pads were preserved. Test pads sit outside the Top logo and away from the buttons, with Top silkscreen labels GND, 5V, 3V3, D+, D−, and RST. They expose GND, VBUS, V33, USB_DP, USB_DM, and RESET respectively.

Coordinates above follow the placement export's board coordinate system. The placement file marks J1 and the three switches as `SMD = No`; J1 has both surface contacts and mechanical anchors. Confirm the intended assembly process and rotations in the assembler's preview.

## Verification and remaining work

During design, netlist checks covered MCU connections, USB polarity, CC resistors, power, switches, and passive footprints. The final saved PCB check after rounding the outline reported zero DRC errors. Geometry checks covered the four R5 arcs, outline closure, and preservation of existing placement and routing.

The schematic check reported zero fatal errors, zero errors, and three warnings concerning the RESET single-pin net, resistor/capacitor attribute standardization, and unused SBU NC marking. These were not represented as a warning-free schematic.

Release-file checks verified ZIP integrity, matching BOM and placement designators, thirteen Top-side components, 0402 passive footprints, four R5 outline arcs, and STEP exchange-file framing. These checks do not replace reviewing the exported layers or checking the assembled board.

Hardware bring-up, the programming/boot procedure, USB behavior, firmware, WebHID configuration, signal quality, ESD performance, and physical keycap/enclosure fit remain to be validated. Review test-pad mask/paste openings and assembly alignment before fabrication.

## Development references

See [PCB development with EasyEDA](../../docs/pcb-development.md) for the workflow used to edit and verify the design. Component decisions were informed by the [WCH CH552 datasheet](https://www.wch-ic.com/downloads/CH552DS1_PDF.html) and supplier data. Schematic organization followed functional grouping and power/ground placement principles from the [KiCad Library Convention](https://klc.kicad.org/).

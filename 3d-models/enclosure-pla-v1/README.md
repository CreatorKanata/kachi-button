# Kachi Button PLA Enclosure Prototype

Created September 6, 2026 from the user's current Fusion `asm` placement. This is a printable prototype, not a physically validated enclosure release.

## Editable source and exports

The Fusion file is `KachiButton-Enclosure-PLA-v1`, saved in HAPT Lab / `kachi-button` (file ID `urn:adsk.wipprod:dm.lineage:ralD4T5OSdiXAXDjQ-ULcw`). It contains the reference assembly plus separate `Enclosure Top` and `Enclosure Bottom` components. Named sketches, extrusions, lofts, and fillets remain editable in the timeline; the outline sketches are not fully dimension-constrained.

- `KachiButton-Enclosure-Top.step`: the box-shaped top and twelve separate logo solids.
- `KachiButton-Enclosure-Bottom.step`: the bottom plate, supports, and integral latches as one solid.
- `KachiButton-Enclosure-Top.3mf`: thirteen mesh objects with their relative placement preserved.
- `KachiButton-Enclosure-Bottom.3mf`: one mesh object.

Exports use millimeters and component-local coordinates, not an automatically arranged print plate. Keep all Top objects together when importing into a slicer.

## Geometry

| Feature | Prototype value |
| --- | --- |
| Footprint | 62.9 × 45.0 mm |
| Case height | 22.05 mm, excluding protruding keycaps |
| Overall height with current keys | 29.05 mm |
| Outer plan corners | R6 mm |
| Top outer edge | R2.5 mm |
| Bottom outer edge | R0.4 mm |
| Nominal side walls / roof / floor | 1.6 / 1.8 / 2.0 mm; the right side is extended for cable sockets |
| PCB side clearance | Approximately 0.3 mm along the ordinary straight walls |
| Top surface | 3 mm above the keycap lower edges |
| Right outer face | Approximately 6 mm beyond the PCB right edge |
| Bottom-to-Top seam clearance | 0.25 mm |
| PCB upper-stop clearance | 0.25 mm above the nominal PCB top |
| Key opening clearance | Nominally 0.5 mm around the full keycap footprint |

In component coordinates, X is board width, Y is board depth, and Z is height. The PCB bottom is Z=0 and its top is Z=1.6 mm. The keycap bottoms are Z=13.55 mm, the enclosure top is Z=16.55 mm, and the bottom outside is Z=-5.5 mm. This maps to the reference assembly as `(X, Z, -Y)`.

The key openings form one stepped opening. Thin inter-key bridges and small junction protrusions are removed; the inward step has an R1.4 blend.

## Screwless assembly

Top is the box with internal catch pockets. Bottom is a plate with four lateral flexure tabs cut into its perimeter region. Their retaining shoulders and insertion ramps stay below the PCB bottom; there are no hooks over the PCB. The beams are approximately 9 mm long and 1.0 mm wide in their bending direction, with 0.5 mm relief slots and R0.25 roots. The reinforced lower pocket lips are 1.2 mm thick and the remaining pocket back walls are approximately 1.1 mm thick. The nominal lateral hook engagement is 0.35 mm.

Bottom supports the PCB underside. Straight stops descending from Top limit upward PCB motion when the enclosure is closed. The latches fasten the enclosure; they do not clamp the PCB by hooking over its upper face.

Place the PCB on the Bottom supports, check that no lead or solder joint sits on a support, and lower Top over the keys. Engage the catches without excessive force. Inspect the narrow tabs after the first closure. Release access is through the relief slots in the bottom; release individual tabs rather than pulling the case apart by force. Verify this operation on the first print.

## USB and cable parking

The right wall has an enclosed, rounded live USB tunnel rather than a large rectangular window. It runs toward J1 and screens the surrounding PCB. The preliminary opening is 9.4 × 3.6 mm with R1.6 corners, centered at board depth 31 mm and height 3.15 mm.

The blind parking socket is to its left when looking directly at the right face, at the same height. Its preliminary opening is 9.2 × 3.6 mm with R1.6 corners, centered at board depth 14 mm. Its depth from the right outer face is 4.75 mm. It is entirely mechanical and has no electrical contacts.

**Cable fit is unresolved.** The purchased 10 cm cable's exposed metal length and white overmold dimensions have not been supplied. The live receptacle is about 6 mm behind the exterior face: a short metal plug may require the socket mouth to be recessed so its overmold can approach J1. Do not assume full insertion or parking retention from these nominal openings. Measure the cable and print a socket fit coupon before printing the complete case.

## Logo and colors

The Top uses `images/logo/kachi-button-text-2lines.svg`, at approximately 22 mm wide in the upper-right area. Twelve disconnected glyph solids fill matching 0.4 mm-deep pockets, flush with the top surface.

- Enclosure shells: white.
- `Logo Kachi ...` bodies: SVG gray `#3C3B3A`.
- `Logo Button ...` bodies: SVG orange `#FE6401`.

Assign the corresponding PLA filaments by object name in the slicer. Do not move or independently auto-arrange the logo objects.

## PLA printing and verification

Suggested initial setup: 0.4 mm nozzle, 0.2 mm layers, and four perimeters where geometry allows. Inspect the 1.0 mm flexures in the slicer: their width requires suitable variable-width extrusion or an equivalent reliable fill strategy. Tune temperatures and cooling to the selected PLA rather than using an unverified universal profile.

Orient Bottom with its flat outside on the bed. Orient Top with its outside top face toward the bed, keeping the inlays registered; the rounded perimeter may need localized support or a brim. Inspect bridges and supports inside both USB sockets and around latch ramps. Keep support contact off the small latch shoulders where possible. These are starting instructions, not a validated slicing profile.

CAD checks found no enclosure-to-PCB interference and no key-to-Top interference at 0, 2, and 4 mm downward key offsets. The 4 mm offset is a clearance check, not a measured switch travel specification. The imported PCB model contains its own internal overlaps; these were separated from enclosure interference. Top and Bottom have no overlapping volume in their assembled positions.

All enclosure features were healthy at export. The shell/plate are solids, all twelve logo inlays are 0.4 mm thick, and all fourteen exported mesh objects passed closed-edge checks. Physical PLA strength, repeated latch cycles, key operation, cable insertion, and parking retention remain untested.

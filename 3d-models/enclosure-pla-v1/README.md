# Kachi Button PLA Enclosure Prototype

Created September 6, 2026 from the user's current Fusion `asm` placement. This is a printable prototype, not a physically validated enclosure release.

## Current STL exports (September 16, 2026)

The current printable enclosure is provided as separate files:

- [Top STL](KachiButton-Enclosure-PLA-v1-Top.stl)
- [Bottom STL](KachiButton-Enclosure-PLA-v1-Bottom.stl)

These replace the combined `KachiButton-Enclosure-PLA-v1.3mf` export. The original September 6 descriptions and subsequent design notes below are historical snapshots; use the current STL geometry for this revision.

The STL files retain the assembly orientation: Y is the height axis. Their bounding dimensions in file X/Y/Z are 71.2921 × 12.6 × 47.2 mm for Top, including the keyring ear, and 63.3 × 6.7 × 47.2 mm for Bottom. STL does not encode units; import as millimeters and choose the print orientation in the slicer.

Both binary STL files passed triangle-record, finite-coordinate, nondegenerate-triangle, and closed-edge checks. These checks establish mesh integrity, not physical fit or strength.

## Initial editable source and exports

The Fusion file is `KachiButton-Enclosure-PLA-v1`, saved in HAPT Lab / `kachi-button` (file ID `urn:adsk.wipprod:dm.lineage:ralD4T5OSdiXAXDjQ-ULcw`). It contains the reference assembly plus separate `Enclosure Top` and `Enclosure Bottom` components. Named sketches, extrusions, lofts, and fillets remain editable in the timeline; the outline sketches are not fully dimension-constrained.

- `KachiButton-Enclosure-Top.step`: the box-shaped top and twelve separate logo solids.
- `KachiButton-Enclosure-Bottom.step`: the bottom plate, supports, and integral latches as one solid.
- `KachiButton-Enclosure-Top.3mf`: thirteen mesh objects with their relative placement preserved.
- `KachiButton-Enclosure-Bottom.3mf`: one mesh object.

Exports use millimeters and component-local coordinates, not an automatically arranged print plate. Keep all Top objects together when importing into a slicer.

## Geometry

| Feature | Prototype value |
| --- | --- |
| Footprint | 63.3 × 47.2 mm |
| Case height | 23.25 mm, excluding protruding keycaps |
| Overall height with current keys | 30.25 mm |
| Outer plan corners | R6 mm |
| Top outer edge | R2.5 mm |
| Bottom outer edge | R0.4 mm |
| Nominal side walls / roof / floor | 1.5 ordinary / 2.4 / 2.0 mm; local snap receivers and USB regions remain reinforced |
| PCB side clearance | Approximately 0.3 mm along the ordinary straight walls |
| Top surface | 3 mm above the keycap lower edges |
| Right outer face | Approximately 6 mm beyond the PCB right edge |
| Bottom-to-Top seam clearance | 0.25 mm |
| PCB upper-stop clearance | 0.25 mm above the nominal PCB top |
| Key opening clearance | 0.5 mm at the outer full-skirt edges, recentered on the current key placements |

In component coordinates, X is board width, Y is board depth, and Z is height. The PCB bottom is Z=0 and its top is Z=1.6 mm. The keycap bottoms are Z=13.55 mm, the enclosure top is Z=16.55 mm, and the bottom outside is Z=-6.7 mm. This maps to the reference assembly as `(X, Z, -Y)`.

The key openings form one stepped opening. Thin inter-key bridges and small junction protrusions are removed; the inward step has an R1.4 blend.

## Screwless assembly

Top is the box with internal catch pockets. Bottom has two broad front latch wedges and two thick fixed rear tabs, all below the PCB. This replaces the earlier four fine hooks.

| Retention feature | Current geometry |
| --- | --- |
| Front wedge contact width | 4 mm |
| Front wedge solid shoulder height | 2.0 mm |
| Front entry ramp height | 0.55 mm |
| Front lateral spring | 10 mm free length, 2 mm wide in the bending direction, 2 mm high |
| Spring relief slots / root fillets | 0.6 mm / R0.4 |
| Front lateral engagement | 0.55 mm nominal |
| Rear fixed tabs | 8 mm wide, 2 mm thick; 2 mm-deep roots |
| Pocket lower lips | 1.5 mm front / 1.7 mm rear |

Bottom now has five PCB supports. The right upper support is a 4 × 5 mm pad directly beneath J1, centered at board X=51.5, Y=31 mm, to support connector insertion loads. Its top is at the PCB underside. The other three supports retain their established locations. Top uses five 3 mm-wide ribs that extend continuously from the side walls to their contact ends, stopping 0.25 mm above the nominal PCB top. They have no hooks above the board.

Assemble with the keycaps removed: place the PCB on Bottom, engage the rear fixed tabs in Top at a slight angle, then close the front latches. Install the keycaps after closure. Release the front latches through the Bottom relief slots before withdrawing the rear tabs. The complete angled assembly/release path still needs physical testing; do not force it if the USB receptacle or PCB catches.

The new wedge removes the former 0.5 mm-thick isolated shoulder. With Bottom flat on the bed, the spring bends in the layer plane, but the upright wedge still transmits force across layers. Larger geometry reduces reliance on fine printed details; it does not establish PLA strength or cycle life. Print a local latch/pocket trial before the full enclosure.

## USB and cable parking

The right wall has an enclosed, rounded live USB tunnel rather than a large rectangular window. It runs toward J1 and screens the surrounding PCB. The preliminary opening is 9.4 × 3.6 mm with R1.6 corners, centered at board depth 31 mm and height 3.15 mm.

The blind parking socket is to its left when looking directly at the right face, at the same height. Its preliminary opening is 9.2 × 3.6 mm with R1.6 corners, centered at board depth 14 mm. Its depth from the right outer face is 3.75 mm, leaving a 2 mm blind end wall. It is entirely mechanical and has no electrical contacts.

**Cable fit is unresolved.** The purchased 10 cm cable's exposed metal length and white overmold dimensions have not been supplied. The live receptacle is about 6 mm behind the exterior face: a short metal plug may require the socket mouth to be recessed so its overmold can approach J1. Do not assume full insertion or parking retention from these nominal openings. Measure the cable and print a socket fit coupon before printing the complete case.

## Logo and colors

The Top uses `images/logo/kachi-button-text-2lines.svg`, at approximately 22 mm wide in the upper-right area. Twelve disconnected glyph solids fill matching 0.4 mm-deep pockets, flush with the top surface.

- Enclosure shells: white.
- `Logo Kachi ...` bodies: SVG gray `#3C3B3A`.
- `Logo Button ...` bodies: SVG orange `#FE6401`.

Assign the corresponding PLA filaments by object name in the slicer. Do not move or independently auto-arrange the logo objects.

## PLA printing and verification

Suggested initial setup: 0.4 mm nozzle, 0.2 mm layers, and four perimeters where geometry allows. Inspect the 2 mm spring strips for continuous extrusion paths; actual line placement depends on the slicer. Tune temperatures and cooling to the selected PLA rather than using an unverified universal profile.

Orient Bottom with its flat outside on the bed. Orient Top with its outside top face toward the bed, keeping the inlays registered; the rounded perimeter may need localized support or a brim. Inspect bridges and supports inside both USB sockets and around latch ramps. Keep support contact off the small latch shoulders where possible. These are starting instructions, not a validated slicing profile.

CAD checks found no enclosure-to-PCB interference and no key-to-Top interference at 0, 2, and 4 mm downward key offsets. The 4 mm offset is a clearance check, not a measured switch travel specification. The imported PCB model contains its own internal overlaps; these were separated from enclosure interference. Top and Bottom have no overlapping volume in their assembled positions.

All enclosure features were healthy at export. The shell/plate are solids, all twelve logo inlays are 0.4 mm thick, and all fourteen exported mesh objects passed closed-edge checks. Physical PLA strength, repeated latch cycles, key operation, cable insertion, and parking retention remain untested.

## Assembly placement update

On September 6, 2026, keycap alignment was corrected against `asm` version 5. The first synchronization copied occurrence transforms but left the previous rigid joints unchanged; recomputation restored those joint-driven positions. That earlier transform-only verification was insufficient.

The enclosure's three existing rigid joints now have X and Y offsets of -0.1403226 mm in their own joint coordinate systems. These values differ from the source assembly's offsets because the source uses different joint reference geometry. After recomputation, saving, and switching documents, all three occurrence transforms and every keycap body's bounding box and volume match the source assembly within the CAD check tolerance (0.000001 mm for positions). Top clearance checks also pass at 0, 2, and 4 mm downward offsets, and all joints remain healthy.

This correction changes assembly constraints and reference key placements only. Top and Bottom solids and their standalone exports remain applicable. Physical key operation remains untested.

## Key opening clearance correction

After the joint alignment correction, the Top opening and its two junction cleanup sketches were translated by +0.215198 mm in board X and +0.215222 mm in board Y. The keycap placements remain unchanged. The measured straight outer footprint gaps are 0.499957–0.500000 mm; the shared inter-key opening and its rounded junction remain continuous. CAD interference checks passed at 0, 2, and 4 mm downward key offsets, and key positions remained stable after recomputation. Top features are healthy and the thirteen exported meshes have closed edges. The Top STEP and 3MF exports were regenerated for this opening correction. Physical printed clearance remains untested.

## Additional rear-right corner support

A paired support was added near the user-marked rear-right corner. Bottom has a 3 × 2 mm pad at board X48–51, Y38–40 mm, extending from the inside floor Z=-4.7 mm to the PCB underside Z=0. Top has a matching 3 mm-wide rib extending to the rear wall at Y41.6 mm; its contact face is at Z=1.85 mm, preserving the nominal 0.25 mm clearance above the PCB. Both features join their respective shells. The Bottom contact footprint was checked inside the imported PCB outline. CAD checks found no enclosure-to-electronics or Top-to-Bottom overlap and no key interference at 0, 2, and 4 mm downward offsets. Both exports were regenerated and passed closed-mesh checks. Actual solder clearance and printed retention still require prototype validation.

## FDM structural reinforcement and shorter springs

The user identified fragile rear tab roots and specified at least 1.5 mm structural sections, preferably 2 mm. The rear tab roots were widened from 0.7 to 2 mm and the tabs thickened to 2 mm. Front spring sections and latch shoulders are also 2 mm. The final spring free length is **10 mm**, per the user; the 0.6 mm end relief is separate from that length. Receiver pocket lower lips are 1.5 mm front and 1.7 mm rear, with approximately 2 mm outer backing. Left, front, and rear exterior walls were expanded, increasing the footprint by 0.4 mm in width and 2.2 mm in depth while retaining height and key placement. The three original narrow Bottom posts were widened from 1.6 to 2 mm.

The right-side Top PCB stops were retracted 0.7 mm toward the wall (contact ends now at board X54 mm), retaining their 3 mm width. USB tunnel surrounding walls and the parking socket ceiling/end wall are now 2 mm. The parking socket is consequently 1 mm shallower. The roof is 2.4 mm, leaving 2 mm beneath the 0.4 mm logo inlays.

CAD checks passed for enclosure-to-PCB, Top-to-Bottom, and key clearance at 0/2/4 mm downward offsets. Features are healthy and both exported parts pass closed-mesh checks. These checks establish geometry, not PLA flexural strength: the 10 mm × 2 mm spring will be stiffer than the previous long thin spring, and assembly force and repeated use must be evaluated on a printed latch/pocket sample. Do not equate interference-free CAD with a tested snap fit.

## Bottom PCB post thickness update

All five Bottom PCB supports now have a minimum nominal cross-section dimension of 2.4 mm or more along their full 4.7 mm rise from the floor. The three original posts are 2.4 × 3 mm, the rear-right post is 3 × 2.4 mm (X48–51, Y37.8–40.2 mm), and the USB support remains 4 × 5 mm. This supersedes the earlier 2 mm post dimensions; the latch geometry remains as specified separately. Feature and section-dimension checks passed, together with enclosure/PCB interference checks and key clearance at 0/2/4 mm. The Bottom STEP and 3MF were regenerated and the mesh is closed.

## Ordinary wall thickness update

Per the user, ordinary side walls are now 1.5 mm while the Bottom plate remains 2 mm. The Top cavity was expanded to a concentric R4.5 inner outline inside the R6 exterior. Four local reinforcement pads preserve the front and rear snap receivers and their previously specified pocket lips/backing. PCB stop roots were extended to remain joined to the enlarged cavity walls, without moving their contact ends. USB regions, the 2.4 mm roof, and Bottom support posts retain their reinforced dimensions. The exterior footprint is unchanged. Solid-boundary tests checked all four ordinary wall thicknesses; enclosure/PCB and key checks at 0/2/4 mm passed. The Top STEP and 3MF were regenerated and the mesh passed closed-edge checks.

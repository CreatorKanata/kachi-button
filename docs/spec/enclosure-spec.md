# Enclosure and 3D CAD Specification

Status: design requirements, September 6, 2026. The enclosure must be designed in **parametric 3D CAD** and validated with physical prototypes. No finished enclosure model or validated fit is established by the current PCB release.

## Appearance and fixed keycap dimensions

![Latest product and keycap reference](../../images/kachi-button-product2.png)

Use the latest reference for the rounded white body, visible key arrangement, logo beside the upper key, and a cable loop on the right. Use the redesigned logo assets; visual renderings do not define hidden construction details.

| Feature | Dimension / decision | Status |
| --- | --- | --- |
| Upper keycap | 33 × 18 mm in plan view | Confirmed by user |
| Lower-left keycap | 25 × 18 mm in plan view | Confirmed by user |
| Lower-right keycap | 25 × 18 mm in plan view | Confirmed by user |
| Pictured enclosure | 62 × 36 × 18 mm | Provisional visual target only |
| Purchased cable | Nominal 10 cm, USB-C male to male | Confirmed purchase; detailed dimensions TBD |
| PCB retention | Snap fits, no corner PCB screws | Confirmed |

The upper width is revised from 35 mm to 33 mm (25 + 4 × 2). All three keycaps are 10 mm high. The central underside stem boss must be 7 mm in outer diameter, increased from 6 mm, while retaining the 1.5 mm print-clearance cross slot. White SVG lettering remains a separate 0.4 mm-thick body set. Physical switch fit and print tolerances still require testing.

Two blank print samples are saved in the Fusion `kachi-button` folder: `KachiButton-Keycap-Sample-Blank-33x18x10` and `KachiButton-Keycap-Sample-Blank-25x18x10`. Each is a single solid with no lettering or lettering recess, retaining the 7 mm stem boss and existing cross-slot clearance. Both models passed dimension and feature-health checks; physical print fit remains untested.

Do not substitute standard “2U / 1U” dimensions for the confirmed keycap sizes. Record which physical keycap edges define the stated dimensions before machining close-fitting openings; skirt profile, stem location, underside cavity, and travel envelope still need measurement.

The two 18 mm keycap rows already require 36 mm before any gap or surrounding wall. The existing PCB is also approximately 41 mm high. Therefore, the pictured 36 mm external enclosure dimension cannot contain the current board and intended keys. Keep the keycap dimensions and PCB geometry fixed; derive a larger case envelope in CAD. The 62 mm width and 18 mm height also remain unverified, particularly with full-height switch bodies and protruding keycaps.

Specify case-body height and overall height including keycaps separately. Do not reduce switch travel, thin walls arbitrarily, or scale imported PCB geometry to match the rendering.

## CAD inputs and datums

Import the [v1 STEP assembly](../../pcb/v1/board-3d-data/3D_PCB1_2026-09-05.step) and align it with the [Gerber outline/drills](../../pcb/v1/gerber/Gerber_PCB1_2026-09-05.zip). Use the [PCB placement table](pcb-spec.md) as a cross-check. STEP export and manufacturing data have different timestamps; verify their agreement before using them as mating geometry.

Use millimeters and a documented board-to-case coordinate transform. Define the board top surface as the case assembly's Z datum, with the nominal 1.6 mm board extending below it. Validate that interpretation against the imported model rather than assuming the STEP origin or axis directions.

Measure actual switches, keycaps, plug bodies, strain relief, and assembled underside protrusions. A placement-file origin is not necessarily a stem center or USB aperture center. Model missing features as clearly identified measured reference bodies.

## Construction and retention

Proposed architecture: a lower tray with board support ledges and snap retention, plus an upper frame around the key openings. Part split and fastening between enclosure pieces are CAD decisions, but PCB retention must use snap fits rather than introducing four corner board holes.

Support the PCB where pressing keys and inserting the cable load it. Place supports on verified free regions and keep them away from exposed pads, solder joints, traces, switch pins, and connector anchor protrusions. A Bottom face without mounted components still needs clearance for through-hole leads and solder fillets.

Snap beams need clearance to flex, root fillets, a lead-in, controlled engagement, and an assembly/release path. Select dimensions and material together, then print coupons before committing to the full case. No untested latch geometry, pull force, or cycle life is a fixed specification.

The rendered underside circles may represent feet; they are not permission to add PCB screw holes. Feet are optional pending stability tests. If used, ensure they do not block disassembly or conceal required access.

## Key openings and motion

Locate openings from the actual switch stems and the confirmed keycaps. Verify the upper 33 × 18 mm cap's orientation and stem offset rather than copying the visual center. Preserve the v1 switch placement unless a PCB revision is explicitly approved.

Model each key at rest and at full depression, including skirt motion and tolerances. Allow clearance between adjacent keys and between keys and walls throughout travel. Check off-center presses on the wide cap. Determine whether the selected cap/switch combination requires stabilization; no stabilizer interface is assumed present in v1.

Keycap rectangles on PCB silkscreen are design references, not certified 3D clearance boundaries. Electronics inside them are allowed only when the real moving keycap envelope clears those components. Keep the frame removable enough for assembly and servicing.

## USB opening, LED, and test access

The only electrical USB port is J1, facing the right edge at exported placement X50, Y31 mm. Derive the aperture from its actual shell and the purchased cable's overmold. Allow full insertion without the overmold bottoming on the case; provide finger access for unplugging and account for molding/printing tolerances.

The LED is D1 at exported X50.927, Y22.733 mm below J1. Provide visibility with a suitable opening, translucent region, or light guide. Select the solution in CAD; do not relocate D1 or R3 just to match a concept rendering.

Keep access to the six Top test pads available during assembly or through an intentionally removable cover. Do not place permanent supports on those pads. A service opening in the final enclosure is optional; accessibility before closure is required for programming and bring-up.

## Cable loop and parking holder

Use the purchased white, nominal **10 cm** USB-C cable as the mechanical baseline. See [the cable reference and unknowns](product-spec.md#purchased-cable).

One plug stays inserted into J1; the free plug parks in an **unpowered mechanical holder** to make a loop. The latest image shows two plug bodies on the right side. This supersedes treating the preliminary rear-holder location as fixed. Exact holder position and orientation are determined by CAD and the actual cable.

Do not add a second live connector. The holder should grip an appropriate measured part of the plug body without damaging the metal shell or creating electrical contact. Provide a release motion and finger clearance. Record the usable cable length between strain-relief exits rather than treating “10 cm” as that measurement.

Route the cable with a natural bend, without forcing a kink or persistent side load on J1. Design strain support so normal handling of the loop does not transfer all force to the connector solder joints. Verify insertion, parking, unplugging, and carrying on a physical prototype. If the 10 cm cable cannot form a practical loop with this geometry, report the conflict before changing the purchased cable baseline.

The loop's load capability is not yet established. Do not describe it as a tested keychain attachment until retention and handling checks are completed.

## Parametric dimensions and deliverables

Keep these values in the CAD parameter table rather than scattering them through sketches:

| Parameter group | Inputs |
| --- | --- |
| Board | Exact outline, R5 arcs, thickness, assembly transform |
| Keys | Confirmed plan sizes; measured stem positions, heights, travel, and clearances |
| Case | Wall/base thickness, key gaps, rim height, overall envelope |
| Retention | Latch length/thickness, engagement, root radius, flex clearance |
| USB | Shell aperture, plug overmold clearance, insertion depth, strain support |
| Cable parking | Holder size, location, orientation, cable diameter, bend envelope |
| Manufacturing | Chosen process/material, shrinkage and clearance allowances, print orientation |

Values not yet measured are TBD. Choose a CAD application that preserves editable parametric source and can export STEP and printable meshes; no particular application has been selected. Prototype manufacturing is 3D printing. Material and process selection must support repeatable snap behavior and acceptable surface finish.

Deliver editable CAD source, a parameter table, STEP solids/assembly, STL or 3MF print files, and a dimensioned drawing. Include an exploded assembly view, section views through the switches/USB/latches, print settings, assembly instructions, and the compatible PCB revision. A rendering alone is not the enclosure deliverable.

## Acceptance checks

| Check | Pass condition |
| --- | --- |
| CAD interference | No collisions among board, components, keycaps through travel, case, and cable |
| PCB fit | Board seats without bending and snaps retain it without touching electrical features |
| Key operation | Each cap returns freely and does not contact adjacent caps or the frame |
| USB fit | Purchased plug fully seats and can be removed without case interference |
| Cable storage | Free plug parks/releases; bend and handling do not loosen or visibly stress J1 |
| Assembly/service | Switch leads clear the base; programming/testing is possible before closure |
| Print iteration | Measured clearance and latch performance recorded for the selected printer/material |
| Dimensional release | Final body and overall dimensions replace provisional image dimensions in drawings |

Record test conditions and results, not just “fits.” Repeated snap/parking cycles, drop/handling criteria, and acceptable forces must be defined before a production release; they are not validated by the v1 board preview.

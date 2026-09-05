# Kachi Button Specifications

Updated September 6, 2026. These English specifications consolidate the preliminary concepts, the design discussion, PCB v1 exports, and the latest product reference.

| Document | Scope |
| --- | --- |
| [Product](product-spec.md) | Purpose, experience, scope, purchased cable, and delivery milestones |
| [PCB](pcb-spec.md) | Electrical interfaces, dimensions, placement coordinates, manufacturing, and checks |
| [Firmware and configuration](firmware-spec.md) | Key behavior, USB HID, WebHID, storage, and acceptance criteria |
| [Enclosure](enclosure-spec.md) | Parametric 3D CAD, confirmed keycap sizes, board retention, and cable storage |
| [PCB development](pcb-development.md) | EasyEDA MCP / `easyeda-api` skill workflow |
| [PCB v1 release](../pcb/v1/README.md) | Version-specific design history, previews, and manufacturing files |

## Authority and status

**Confirmed** means explicitly decided by the user. **Exported** means represented in the supplied v1 design files. **Required** means behavior or a deliverable to implement; it does not mean implemented. **Proposed** identifies an engineering choice that still needs implementation review. **TBD** means information or a decision is missing.

Use the latest user decisions for product requirements and the actual versioned exports for existing PCB geometry. If these conflict, record the incompatibility and resolve it explicitly; do not silently rescale the PCB or keycaps. Product images guide appearance but do not establish unspecified dimensions or prove physical fit.

The ignored `docs/concept/` directory contains the original planning input. These specifications are self-contained and do not require those local files to be present.

## Current decisions

| Topic | Current baseline |
| --- | --- |
| Key arrangement | One upper wide key, two lower keys |
| Keycap size | Confirmed: upper 35 × 18 mm; lower two each 25 × 18 mm |
| Product reference | `images/kachi-button-product2.png` is the latest appearance reference |
| Enclosure size | 62 × 36 × 18 mm in the image is provisional, not a manufacturing requirement |
| PCB | Existing v1, approximately 55 × 41 mm, four R5 corners, two copper layers |
| Assembly | Components on Top; resistors and capacitors all 0402 |
| Retention | Snap-fit PCB retention; no corner screw holes |
| Power | USB-C 5 V sink, without USB PD negotiation |
| Cable | Purchased: nominal 10 cm, USB-C male to USB-C male, white |
| Configuration | Required: browser configuration over vendor HID; settings retained on device |
| Development language | Japanese chat; English documentation, code, comments, issues, and PRs |

Earlier 0603 passive suggestions, a horizontal three-key row, a 65 × 25–30 mm board, and a 15–20 cm cable are superseded. Earlier “2U / 1U” shorthand must not replace the confirmed millimeter dimensions. The v1 design record remains a historical account of PCB work.

## Open decisions and dependencies

| Item | Required resolution |
| --- | --- |
| Enclosure dimensions | CAD must contain the existing board and measured component stack; the pictured 36 mm case dimension is smaller than the board's approximately 41 mm dimension |
| Keycap fit | Verify MX stem geometry, offset from the cap center, height, travel, and underside clearances with actual parts |
| Cable | Confirm USB data support; measure length convention, overmolds, strain relief, and usable bend radius |
| Firmware | Select toolchain, validate boot entry, USB descriptors, memory budget, and report format |
| USB identification | Establish appropriate VID/PID values before release; do not assume example identifiers are assigned |
| Assembly | Recheck stock/pricing and verify switch and USB-C assembly handling |
| Prototype | Electrical bring-up, firmware operation, and physical fit are pending |

The delivered PCB, firmware, and enclosure should be versioned independently and identify their compatible counterparts. Do not treat a PCB manufacturing export as a completed product release.

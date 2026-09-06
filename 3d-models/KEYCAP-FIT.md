# Keycap Fit Revision — September 6, 2026

This revision addresses the user's physical feedback: the 7 mm stem boss interfered with the switch, and the previous 1.5 mm cross socket was loose. All three lettered Fusion files and both blank sample files have been updated and saved in HAPT Lab / `kachi-button`.

| Feature | Revised CAD dimension |
| --- | --- |
| Circular stem boss outside diameter | 6.00 mm |
| Cross socket arm width, both axes | 1.35 mm |
| Cross socket overall span, both axes | 4.25 mm |
| Blind socket depth | 4.50 mm |
| Entry mouth width / span | 1.65 / 4.55 mm |
| Entry transition height | 0.30 mm |
| External keycap sizes | 33 × 18 × 10 mm and 25 × 18 × 10 mm |
| White SVG inlays | Separate bodies, 0.40 mm thick, flush with the top |

## Basis for the fit

[KeyV2's source dimensions](https://github.com/rsheldiii/KeyV2/blob/master/src/functions.scad) use horizontal and vertical arms with different nominal widths. With the [default inner clearance of 0.2](https://github.com/rsheldiii/KeyV2/blob/master/src/settings.scad), the formulas give approximately 1.317 mm and 1.217 mm widths. Its split-stem construction differs from this closed cylindrical boss, so those dimensions are reference points rather than a direct fit guarantee.

A separate [OpenSCAD printable keycap example](https://gist.github.com/timmo001/bf831a57b441c036bef390cfc226dc51) uses a symmetric 1.25 × 4.25 mm cross and a 5.5 mm circular boss. These examples use tighter sockets than the previous 1.5 mm width.

The selected 1.35 mm width leaves 0.05 mm total nominal clearance over the user's approximately 1.3 mm shaft thickness, compared with 0.20 mm previously. The 4.25 mm span follows the symmetric example. This is a tighter prototype target, not a universally optimal printed fit. Print a blank sample with the intended PLA, orientation, and slicer settings before printing the full colored set. Record insertion force, rocking, retention, and removal; do not force a cap that requires excessive pressure.

## Files and verification

Each keycap has a STEP and a 3MF export. Preserve the relative placement of all glyph bodies when importing a lettered 3MF; assign the body and white inlay filaments separately. The blank samples have one body each.

Fusion solid-boundary checks tested the socket walls at three insertion depths, the cross endpoints, the 6 mm boss boundary, and the blind socket roof. All five models retained their external dimensions, expected body counts, healthy features, and 0.4 mm lettering thickness. Results are recorded in `keycap-fit-verification.json`. Mesh validation checks closed triangle edges and archive integrity; it does not establish physical fit.

The separately arranged `3d-models/keycap/KachiButton-Keycaps.3mf` in the repository is a user-supplied print project and was not overwritten. Replace its keycap geometry with these revised exports before using that plate for this fit revision.

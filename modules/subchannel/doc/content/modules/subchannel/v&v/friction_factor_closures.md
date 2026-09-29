# Friction Factor Closures Verification

## Test Description

&nbsp;

This verification case checks the axial friction factor $f$ that SCM computes over the laminar, transition, and turbulent regimes for the friction closures in SCM:

- [SCMFrictionMATRA.md], for bare pins in a quadrilateral lattice;
- [SCMFrictionChenTodreas.md], for bare pins in a quadrilateral lattice and for bare or wire-wrapped pins in a triangular lattice. The triangular-lattice parameterizations are the upgraded Chen-Todreas correlation (UCTD), `friction_model = Upgraded`, and the Pacio-Chen-Todreas correlation (PCTD), `friction_model = Pacio`.

The curves are the friction factor computed by SCM, the `ff` auxiliary variable, in assemblies with the geometry of existing SCM inputs. The inputs in `modules/subchannel/verification/friction_model_verification/friction_model` sweep the inlet mass flux up to a bulk Reynolds number of about $3 \times 10^5$ and report the friction factor and the local Reynolds number of one interior, edge, and corner subchannel. The script `modules/subchannel/doc/content/media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py` plots their CSV output. The friction factor is plotted versus the local subchannel Reynolds number, while SCMFrictionChenTodreas selects the laminar, transition, or turbulent regime from the bulk Reynolds number of the assembly. Solid, dashed, and dotted lines denote interior, edge, and corner subchannels. MATRA does not distinguish between subchannel types and is shown as a single line for the interior subchannel.

## Results

&nbsp;

### Quadrilateral lattice, bare pins

The geometry is the PSBT assembly, with pitch $P = 12.6$ mm, pin diameter $D = 9.5$ mm, and side gap $0.95$ mm.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_friction_quad_bare.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-friction-quad-bare
    caption=Friction factor of MATRA (black) and Chen-Todreas (red) for bare pins in a quadrilateral lattice.

### Triangular lattice, bare pins

The geometry is the 19-pin LBE assembly, with $P = 11.48$ mm, $D = 8.2$ mm, and duct flat-to-flat distance $53.2$ mm. Only UCTD is shown. `friction_model = Pacio` uses the same bare-pin coefficients but changes the transition Reynolds numbers and the interpolation between the laminar and turbulent regimes.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_friction_tri_bare.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-friction-tri-bare
    caption=Friction factor of UCTD for bare pins in a triangular lattice.

### Triangular lattice, wire-wrapped pins

The geometry is the 61-pin EBR-II XX09 assembly, with $P = 5.664$ mm, $D = 4.419$ mm, wire diameter $D_w = 1.244$ mm, wire lead length $H = 152.4$ mm, and duct flat-to-flat distance $46.4$ mm. For this geometry the laminar UCTD and PCTD curves nearly coincide.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_friction_tri_wire.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-friction-tri-wire
    caption=Friction factor of UCTD (black) and PCTD (red) for wire-wrapped pins in a triangular lattice.

## Input files

The three geometry inputs include a shared file with the mass flux sweep and postprocessors:

!listing /verification/friction_model_verification/friction_model/friction_factor_sampling.i language=moose

Quadrilateral lattice, bare pins:

!listing /verification/friction_model_verification/friction_model/quad_bare.i language=moose

Triangular lattice, bare pins:

!listing /verification/friction_model_verification/friction_model/tri_bare.i language=moose

Triangular lattice, wire-wrapped pins:

!listing /verification/friction_model_verification/friction_model/tri_wire.i language=moose

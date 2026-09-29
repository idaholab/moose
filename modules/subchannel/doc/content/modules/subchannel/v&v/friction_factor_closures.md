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

The SCM friction factors are compared with the UCTD implementation of the DASSH subchannel code [!cite](atz2021ducted). DASSH computes the interior, edge, and corner subchannel friction factors in its UCTD flow split and uses the bundle friction factor for the pressure drop. The script `dassh_XX09_SS17.py` evaluates the DASSH subchannel friction factors for the same geometry with the DASSH flow split and transition interpolation, over the bundle Reynolds number range of the SCM sweep. The DASSH curves overlay the SCM UCTD curves.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_friction_tri_wire.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-friction-tri-wire
    caption=Friction factor of UCTD (black) and PCTD (red) in SCM and UCTD in DASSH (green) for wire-wrapped pins in a triangular lattice. The DASSH curves hide the SCM UCTD curves.

[tab-friction-tri-wire] gives the friction factors at local subchannel Reynolds numbers of $10^2$, $10^3$, and $10^4$, interpolated in log-log from the curves of [scm-friction-tri-wire]. SCM UCTD and DASSH UCTD agree within 1.5%. PCTD gives up to 8.5% lower friction factors in the transition and turbulent regimes.

!table id=tab-friction-tri-wire caption=Friction factor of wire-wrapped pins in a triangular lattice at local subchannel Reynolds numbers of $10^2$, $10^3$, and $10^4$.
| Subchannel | Closure | $Re = 10^2$ | $Re = 10^3$ | $Re = 10^4$ |
| :- | :- | -: | -: | -: |
| interior | SCM UCTD | 0.8511 | 0.1126 | 0.0383 |
| interior | SCM PCTD | 0.8501 | 0.1083 | 0.0351 |
| interior | DASSH UCTD | 0.8511 | 0.1127 | 0.0382 |
| edge | SCM UCTD | 0.9949 | 0.1129 | 0.0322 |
| edge | SCM PCTD | 0.9924 | 0.1033 | 0.0306 |
| edge | DASSH UCTD | 0.9949 | 0.1112 | 0.0322 |
| corner | SCM UCTD | 0.9596 | 0.1152 | 0.0316 |
| corner | SCM PCTD | 0.9555 | 0.1128 | 0.0296 |
| corner | DASSH UCTD | 0.9596 | 0.1154 | 0.0313 |

### EBR-II XX09, SHRT-17 steady state

The effect of the friction closure on the flow and temperature distribution is shown for the SHRT-17 steady state of the EBR-II XX09 assembly in [EBR-II.md]. `XX09_SS17.i` runs `validation/EBR-II/XX09_SCM_SS17.i` with UCTD, or with PCTD via `SCMClosures/Chen/friction_model=Pacio`, and reports the mass flow rate and temperature of the subchannels of the TTC thermocouples at $z = 0.322$ m. `dassh_XX09_SS17.txt` is a DASSH model of the same case with UCTD friction, flow split, and mixing: the same geometry, inlet temperature, mass flow rate, uniform radial pin power, and axial power shape. Unlike the DASSH results in [EBR-II.md], the thimble region is not modeled, as in SCM. DASSH orients the hexagonal lattice 30 degrees apart from SCM, so each TTC subchannel of SCM is compared with the DASSH subchannel at the same position after rotation.

DASSH does not solve a lateral momentum equation, so its mass flow rate is uniform over each type of subchannel. The SCM mass flow rate of the interior subchannels is 4% to 12% higher than DASSH, most next to the edge subchannels, so the DASSH temperature is up to 16 K higher in the interior of the assembly. The SCM PCTD mass flow rate of the interior subchannels is about 1% higher than with UCTD, which lowers the temperature by up to 1.5 K.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_XX09_SS17_mdot.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-XX09-SS17-mdot
    caption=Subchannel mass flow rate along the TTC traverse of EBR-II XX09, SHRT-17 steady state.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_XX09_SS17_T.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-XX09-SS17-T
    caption=Subchannel temperature along the TTC traverse of EBR-II XX09, SHRT-17 steady state.

## Input files

The three geometry inputs include a shared file with the mass flux sweep and postprocessors:

!listing /verification/friction_model_verification/friction_model/friction_factor_sampling.i language=moose

Quadrilateral lattice, bare pins:

!listing /verification/friction_model_verification/friction_model/quad_bare.i language=moose

Triangular lattice, bare pins:

!listing /verification/friction_model_verification/friction_model/tri_bare.i language=moose

Triangular lattice, wire-wrapped pins:

!listing /verification/friction_model_verification/friction_model/tri_wire.i language=moose

EBR-II XX09, SHRT-17 steady state:

!listing /verification/friction_model_verification/friction_model/XX09_SS17.i language=moose

DASSH model of EBR-II XX09, SHRT-17 steady state:

!listing /verification/friction_model_verification/friction_model/dassh_XX09_SS17.txt language=text

The script that runs DASSH and writes the DASSH data:

!listing /verification/friction_model_verification/friction_model/dassh_XX09_SS17.py language=python

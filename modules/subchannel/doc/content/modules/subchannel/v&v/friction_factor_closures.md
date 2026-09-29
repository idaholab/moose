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

[tab-friction-tri-wire] gives the friction factors at local subchannel Reynolds numbers of $10^2$, $3 \times 10^3$, and $10^4$, interpolated in log-log from the curves of [scm-friction-tri-wire]. The Reynolds numbers avoid the onset of the transition regime near $10^3$, where the friction factor changes slope sharply and the interpolation between the sampled mass fluxes is not accurate. For both parameterizations and all subchannels, $Re = 10^2$ is in the laminar regime and $Re = 3 \times 10^3$ is inside the transition regime, with an intermittency factor $\psi$ of 0.37 to 0.63 based on the bulk Reynolds number; $Re = 10^4$ is near the transition-turbulent boundary. SCM UCTD and DASSH UCTD agree within 1%. PCTD gives up to 8% lower friction factors in the transition and turbulent regimes.

!table id=tab-friction-tri-wire caption=Friction factor of wire-wrapped pins in a triangular lattice at local subchannel Reynolds numbers of $10^2$, $3 \times 10^3$, and $10^4$.
| Subchannel | Closure | $Re = 10^2$ | $Re = 3 \times 10^3$ | $Re = 10^4$ |
| :- | :- | -: | -: | -: |
| interior | SCM UCTD | 0.8511 | 0.0597 | 0.0383 |
| interior | SCM PCTD | 0.8501 | 0.0559 | 0.0351 |
| interior | DASSH UCTD | 0.8511 | 0.0597 | 0.0382 |
| edge | SCM UCTD | 0.9949 | 0.0548 | 0.0322 |
| edge | SCM PCTD | 0.9924 | 0.0537 | 0.0306 |
| edge | DASSH UCTD | 0.9949 | 0.0546 | 0.0322 |
| corner | SCM UCTD | 0.9596 | 0.0545 | 0.0316 |
| corner | SCM PCTD | 0.9555 | 0.0528 | 0.0296 |
| corner | DASSH UCTD | 0.9596 | 0.0544 | 0.0313 |

### EBR-II XX09, SHRT-17 steady state

The effect of the closures on the flow and temperature distribution is shown for the SHRT-17 steady state of the EBR-II XX09 assembly in [EBR-II.md]. `XX09_SS17.i` runs `validation/EBR-II/XX09_SCM_SS17.i`, with $C_T = 1.0$ instead of 2.6 for the turbulent exchange of axial momentum, with UCTD friction and Chen-Todreas (1986) mixing, `mixing_model = 1986` of [SCMMixingChenTodreas.md], or with PCTD friction and mixing via `SCMClosures/Chen/friction_model=Pacio` and `SCMClosures/Chen_Todreas/mixing_model=Pacio`, and reports the mass flow rate and temperature of the subchannels of the TTC thermocouples at $z = 0.322$ m. `dassh_XX09_SS17.txt` is a DASSH model of the same case with UCTD friction, flow split, and mixing: the same geometry, inlet temperature, bundle mass flow rate, uniform radial pin power, and axial power shape. As in the DASSH results in [EBR-II.md], DASSH also models the thimble, the bypass annulus between the inner duct and the guide thimble, which takes 9% of the XX09 flow rate and cools the inner duct. SCM does not model the thimble and treats the inner duct as adiabatic. The two codes define the same subchannels with the same flow areas, and DASSH orients the hexagonal lattice 30 degrees apart from SCM. Each TTC subchannel of SCM is therefore compared with the DASSH subchannel at the same position after a rotation; both codes sweep the wire-wrap flow in the same direction, so no reflection is needed.

DASSH does not solve a lateral momentum equation, so its mass flow rate is uniform over each type of subchannel and follows the UCTD flow split. The SCM mass flow rate of the interior subchannels is 3% to 8% higher than DASSH, most next to the edge subchannels, and the SCM edge subchannels carry 6% less flow. The SCM flow split is flatter than the UCTD flow split because of the turbulent exchange of axial momentum between subchannels; with a negligible $C_T$, SCM recovers the DASSH flow split. The DASSH temperature is up to 11 K higher in the interior and 26 K lower in the corner subchannel of TTC-35, which the thimble cools. Besides the thimble, most of the difference comes from the thermal mixing models, Chen-Todreas (1986) mixing with the lateral flow in SCM and the UCTD eddy diffusivity and wire-wrap swirl in DASSH, and less from the flow split and the sodium properties. PCTD changes the SCM solution mostly through the Pacio mixing at the center-edge and edge-corner gaps, which flattens the temperature profile: compared with UCTD, the interior subchannels carry 1.2% to 4.7% more flow and are up to 14 K cooler, most next to the edge subchannels, and the corner subchannel of TTC-35 is 11 K hotter.

The temperature is also compared with the published DASSH results of [DASSH Example-3](https://github.com/dassh-dev/examples/tree/master/Example-3) ([TTC results](https://github.com/dassh-dev/examples/blob/master/Example-3/results_ttc.png)), which are the DASSH results of [EBR-II.md] (`validation/EBR-II/TTC_DASSH.csv`). Example-3 uses the Chen-Todreas (1986) (CTD) friction and flow split, MIT mixing, and a quadratic axial power shape in the 59 fueled pins of XX09. Running `dassh_XX09_SS17.txt` with the correlations, duct heat transfer parameters, and pin power of Example-3 reproduces the published temperatures within 1.5 K. The flow split sets most of the difference between the two DASSH results. At the bundle Reynolds number of about $3.3 \times 10^4$, the CTD flow split of DASSH gives the interior, edge, and corner subchannels 0.975, 1.060, and 0.855 times the bundle average velocity, the UCTD flow split 0.887, 1.207, and 1.034, and SCM with UCTD 0.929, 1.130, and 1.029 at $z = 0.322$ m. With CTD instead of UCTD friction and flow split, the DASSH temperature of the interior subchannels is about 19 K lower, and the published DASSH temperature is 9 K to 11 K lower than SCM with UCTD in the interior. Compared with the measured TTC temperatures of [EBR-II.md] (`validation/EBR-II/TTC_EXP.csv`), all four results overpredict the interior temperature, on average by 18 K for SCM UCTD, 14 K for SCM PCTD, 29 K for DASSH UCTD, and 8 K for the published DASSH results. The root mean square errors over the nine TTC thermocouples are 17 K, 17 K, 26 K, and 11 K; SCM PCTD overpredicts the temperature of the corner subchannel of TTC-35 by 21 K. All results use a uniform radial pin power and therefore give a symmetric profile, while the measured profile is asymmetric and peaks at TTC-31.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_XX09_SS17_mdot.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-XX09-SS17-mdot
    caption=Subchannel mass flow rate along the TTC traverse of EBR-II XX09, SHRT-17 steady state.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_XX09_SS17_T.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-XX09-SS17-T
    caption=Subchannel temperature along the TTC traverse of EBR-II XX09, SHRT-17 steady state. The published DASSH results use the CTD friction and flow split, and the experiment is the measured TTC temperature.

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

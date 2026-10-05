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

The SCM friction factors are compared with the UCTD implementation of the DASSH subchannel code [!cite](atz2021ducted). DASSH computes the interior, edge, and corner subchannel friction factors in its UCTD flow split and uses the bundle friction factor for the pressure drop. The script `dassh_XX09_SS17.py` evaluates the DASSH subchannel friction factors for the same geometry with the DASSH flow split and transition interpolation, over the bundle Reynolds number range of the SCM sweep. The right side of [scm-friction-tri-wire] gives the relative difference between the SCM and DASSH UCTD friction factors at the local Reynolds numbers of the SCM sweep. DASSH does not implement the PCTD correlations, so PCTD is not compared with DASSH.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_friction_tri_wire.png
    style=width:100%;margin-bottom:2%;margin:auto;
    id=scm-friction-tri-wire
    caption=Left: friction factor of UCTD (black) and PCTD (red) in SCM and UCTD in DASSH (green) for wire-wrapped pins in a triangular lattice; the DASSH curves hide the SCM UCTD curves. Right: relative difference between the SCM and DASSH UCTD friction factors.

[tab-friction-tri-wire] gives the friction factors at local subchannel Reynolds numbers of $10^2$, $3 \times 10^3$, and $10^4$, interpolated in log-log from the curves of [scm-friction-tri-wire]. The Reynolds numbers avoid the onset of the transition regime near $10^3$, where the friction factor changes slope sharply and the interpolation between the sampled mass fluxes is not accurate. For both parameterizations and all subchannels, $Re = 10^2$ is in the laminar regime and $Re = 3 \times 10^3$ is inside the transition regime, with an intermittency factor $\psi$ of 0.37 to 0.63 based on the bulk Reynolds number; $Re = 10^4$ is near the transition-turbulent boundary. SCM UCTD and DASSH UCTD agree within 1% in the table and over the laminar and turbulent regimes. The largest difference, 7% in the edge subchannel, is at the onset of the transition regime. Both codes evaluate $\psi$ from the bulk Reynolds number, but SCM solves the lateral momentum equation and gives the edge subchannel about 6% lower local Reynolds number than the UCTD flow split of DASSH at the same bulk Reynolds number. At the same local Reynolds number, $\psi$ is therefore slightly different in the two codes, and with $\gamma = 1/3$ the friction factor changes steeply with $\psi$ near $\psi = 0$. PCTD gives up to 8% lower friction factors than UCTD in the transition and turbulent regimes.

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

The effect of the closures on the flow and temperature distribution is shown for the SHRT-17 steady state of the EBR-II XX09 assembly in [EBR-II.md]. `XX09_SS17.i` runs `validation/EBR-II/XX09_SCM_SS17.i` with UCTD friction and Chen-Todreas (1986) mixing, `mixing_model = 1986` of [SCMMixingChenTodreas.md], or with PCTD friction and mixing via `SCMClosures/Chen/friction_model=Pacio` and `SCMClosures/Chen_Todreas/mixing_model=Pacio`, and reports the mass flow rate and temperature of the subchannels of the TTC thermocouples at $z = 0.322$ m. The turbulent exchange of axial momentum uses $C_T = 1.0$, the default of the mixing closures, for which the momentum and enthalpy turbulent interchange flow rates are equal ([Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer)). `XX09_SCM_SS17.i` uses $C_T = 2.6$, but $C_T$ has been calibrated only for bare pins in a quadrilateral lattice ([thors.md]), and no calibrated value exists for wire-wrapped pins in a triangular lattice.

`dassh_XX09_SS17.txt` is a DASSH model of the same case with UCTD friction, flow split, and mixing: the same geometry, inlet temperature, bundle mass flow rate, uniform radial pin power, and axial power shape. As in SCM, the inner duct is adiabatic, and the thimble, the bypass annulus between the inner duct and the guide thimble in the DASSH results of [EBR-II.md], is not modeled. The two codes define the same subchannels with the same flow areas, and DASSH orients the hexagonal lattice 30 degrees apart from SCM. Each TTC subchannel of SCM is therefore compared with the DASSH subchannel at the same position after a rotation; both codes sweep the wire-wrap flow in the same direction, so no reflection is needed.

DASSH does not solve a lateral momentum equation, so its mass flow rate is uniform over each type of subchannel and follows the UCTD flow split. The SCM mass flow rate of the interior subchannels is 3% to 8% higher than DASSH, most next to the edge subchannels, and the SCM edge subchannels carry 6% less flow. The SCM flow split, given with the DASSH flow split in [tab-XX09-SS17-flow-split], is flatter than the DASSH flow split because of the turbulent exchange of axial momentum between subchannels; with a negligible $C_T$, SCM recovers the DASSH flow split. The DASSH temperature is up to 15 K higher than SCM UCTD in the interior and 14 K lower in the corner subchannel of TTC-35. Most of the difference comes from the thermal mixing models, Chen-Todreas (1986) mixing with the lateral flow in SCM and the UCTD eddy diffusivity and wire-wrap swirl in DASSH, and less from the flow split and the sodium properties. PCTD changes the SCM solution mostly through the Pacio mixing at the center-edge and edge-corner gaps, which flattens the temperature profile: compared with UCTD, the interior subchannels carry 1.2% to 4.7% more flow and are up to 14 K cooler, most next to the edge subchannels, and the corner subchannel of TTC-35 is 11 K hotter.

!table id=tab-XX09-SS17-flow-split caption=Average velocity of the interior, edge, and corner subchannels divided by the bundle average velocity of EBR-II XX09, SHRT-17 steady state, at the bundle Reynolds number of about $3.3 \times 10^4$. The SCM values are at $z = 0.322$ m.
| Flow split | Interior | Edge | Corner |
| :- | -: | -: | -: |
| DASSH CTD | 0.975 | 1.060 | 0.855 |
| DASSH UCTD | 0.887 | 1.207 | 1.034 |
| SCM UCTD | 0.929 | 1.130 | 1.029 |

The temperature is also compared with the published DASSH results of [DASSH Example-3](https://github.com/dassh-dev/examples/tree/master/Example-3) ([TTC results](https://github.com/dassh-dev/examples/blob/master/Example-3/results_ttc.png)), which are the DASSH results of [EBR-II.md] (`validation/EBR-II/TTC_DASSH.csv`). Example-3 models the thimble and uses the Chen-Todreas (1986) (CTD) friction and flow split, MIT mixing, and a quadratic axial power shape in the 59 fueled pins of XX09. Running `dassh_XX09_SS17.txt` with the thimble model in its comments and the correlations, duct heat transfer parameters, and pin power of Example-3 reproduces the published temperatures within 1.5 K. The flow split sets most of the difference between the two DASSH results: with the thimble model, CTD instead of UCTD friction and flow split lowers the DASSH temperature of the interior subchannels by about 19 K. [tab-XX09-SS17-T] compares all results with the measured TTC temperatures of [EBR-II.md] (`validation/EBR-II/TTC_EXP.csv`). All results overpredict the interior temperature, and SCM PCTD overpredicts the temperature of the corner subchannel of TTC-35 by 21 K. All results use a uniform radial pin power and therefore give a symmetric profile, while the measured profile is asymmetric and peaks at TTC-31.

!table id=tab-XX09-SS17-T caption=Subchannel temperature in K at the TTC thermocouples of EBR-II XX09, SHRT-17 steady state, $z = 0.322$ m, and its error against the experiment over the nine thermocouples. The published DASSH results use the CTD friction and flow split and model the thimble.
| Thermocouple | SCM UCTD | SCM PCTD | DASSH UCTD | DASSH CTD, published | Experiment |
| :- | -: | -: | -: | -: | -: |
| TTC-27 | 775.7 | 762.4 | 784.4 | 765.0 | 761.1 |
| TTC-28 | 816.6 | 808.9 | 831.1 | 805.9 | 792.5 |
| TTC-29 | 827.1 | 824.2 | 840.4 | 816.9 | 800.2 |
| TTC-30 | 828.1 | 826.3 | 841.4 | 818.9 | 805.9 |
| TTC-31 | 828.1 | 826.3 | 841.4 | 819.0 | 813.9 |
| TTC-32 | 827.1 | 824.2 | 840.4 | 817.1 | 810.6 |
| TTC-33 | 816.5 | 808.7 | 831.0 | 807.1 | 811.6 |
| TTC-34 | 774.9 | 761.0 | 784.0 | 770.0 | 785.0 |
| TTC-35 | 738.5 | 749.3 | 725.0 | 722.4 | 728.2 |
| Mean error | 13.8 | 9.2 | 23.4 | 3.7 | |
| Root mean square error | 17.4 | 17.1 | 27.8 | 10.5 | |

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_XX09_SS17_mdot.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-XX09-SS17-mdot
    caption=Subchannel mass flow rate along the TTC traverse of EBR-II XX09, SHRT-17 steady state.

!media media_scripts/verification/friction_model_verification/friction_model/scm_friction_factor_closures.py
    image_name=scm_XX09_SS17_T.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-XX09-SS17-T
    caption=Subchannel temperature along the TTC traverse of EBR-II XX09, SHRT-17 steady state. The published DASSH results use the CTD friction and flow split and model the thimble, and the experiment is the measured TTC temperature.

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

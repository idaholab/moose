# Crystal Plasticity FCC Dislocation Link Hu-Cocks Update

!syntax description /Materials/CrystalPlasticityFCCDislocationLinkHuCocksUpdate

## Description

This crystal plasticity dislocation glide model was initially developed to model thermal creep in 316 stainless steel, an austenitic face-centered cubic (FCC) material [!citep](hu2016multiscale).
The Hu-Cocks model uses a per-slip-plane basis as the foundation of the constitutive law formulation.
Rather than tracking the dislocation densities on individual slip systems (12 systems) as other crystal plasticity formulations have done, see [CrystalPlasticityKalidindiUpdate](/CrystalPlasticityKalidindiUpdate.md), the Hu-Cocks model tracks dislocation pinning points on each of the slip planes (4 planes).
Barriers to dislocation motion, including forest dislocations, solutes, and solid precipitates are also tracked on a per-slip plane basis.

`CrystalPlasticityFCCDislocationLinkHuCocksUpdate` is designed to be used in conjunction with the [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class to calculate the response of a FCC crystalline solid.
Details about the algorithm and specific stress and strain measures used in the `CrystalPlasticityUpdate` base class are given on the documentation page for [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md).

As in other crystal plasticity constitutive models compatible with [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md), the user must supply the slip plane normal and slip direction information, and the lengths of the unit cell lattice parameters.

## Constitutive Model Definition

The dislocation pinning point density, $N_d$, in the `CrystalPlasticityFCCDislocationLinkHuCocksUpdate` class is tracked as a stateful material property on each of the four slip planes, denoted with $\Omega$, of the FCC crystal.
However, to interface with the [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class, the dislocation glide (or slip) resistance and the applied resolved shear stress are calculated on a per-slip-system basis; slip systems are notated here with a $\alpha$ superscript.

Recall that for an FCC crystal, there are three slip directions per slip plane, for a total of 12 slip systems.
The slip resistance is calculated as a function of the pinning point density on each of the slip planes, and all three slip systems within a slip plane are assumed to have the same pinning point density value.

### Slip Rate Calculation

The slip increment on each slip system is defined with a power law relationship as
\begin{equation}
    \Delta \gamma^{\alpha} = \Delta \gamma_o \left| \frac{\tau^{\alpha}}{g^{\alpha}} \right| ^p sign \left( \tau^{\alpha} \right)
\end{equation}
where $\alpha$ indicates a slip system (a total of 12 for an FCC crystal), $\Delta \gamma_o$ is the reference shear strain increment, $\tau^{\alpha}$ is the applied shear stress on slip system $\alpha$, and $g^{\alpha}$ is the slip
system resistance value.

The shear strain increment $\Delta \gamma^{\Omega}$ for each slip plane $\Omega$ is defined as the sum of the slip system shear increments on that slip plane:
\begin{equation}
    \Delta \gamma^{\Omega} = \sum_{\alpha \in \Omega} \Delta \gamma^{\alpha}.
\end{equation}
The combined slip plane shear increment is used to calculate the pinning point increment as discussed in [pinning point density evolution ](#pinning_evolution) section.

### Slip System Resistance Calculation

The contributions to the slip system resistance, or strength, are defined as shown in [eqn:irrGeneralHardening].
Because the number densities of the pinning points and precipitates are expected to be within a factor of three of each other, the geometric mean is used to combine the contributions of these two features. The solute strengthening contribution is included as a linear sum.
\begin{equation}
\label{eqn:irrGeneralHardening}
    g^{\alpha} = \left( (g_d^{\alpha})^2 + (g_p)^2 \right)^{1/2} + g_s
\end{equation}
where $g_d^{\alpha}$ is the forest dislocation hardening, due to the pinning point density, $g_p$ is the solid precipitate strengthening, and $g_s$ is the solute strengthening.
These individual strengthening contributions are all calculated with an impenetrable barrier formulation.

The forest hardening term is calculated as a function of the pinning points per slip plane, $\Omega$,
[!citep](hu2016multiscale), in which all three slip systems, indicated by the superscript $\alpha$, are prescribed the same strength value:
\begin{equation}
    g_d^{\alpha} = \alpha_d Gb \sqrt{N_d^{\Omega}} \Bigg|_{\alpha \in \Omega}
\end{equation}
where $\alpha_d$ is a barrier strength parameter, $G$ is the shear modulus, $b$ is the Burgers vector, and $N_d^{\Omega}$ is the pinning point density on slip plane $\Omega$.

The solid precipitate strengthening term is independent of the slip planes and is computed directly as a function of the average precipitate radius $r_p$ and precipitate number density $N_p$
\begin{equation}
    g_p = \frac{\alpha_p Gb}{r_p}\sqrt{\frac{3}{2 \pi}\left( \frac{4}{3} \pi r_p^3 N_p \right)}
\end{equation}
where $\alpha_p$ is the solid precipitate barrier strength parameter [!citep](hu2016evaluation).
The solute strengthening is also independent of the slip planes,
\begin{equation}
    g_s = \frac{\alpha_s Gb}{L_s}, \quad \text{where} \quad L_s = \left( \frac{1}{cb} \right)^{1/2}.
\end{equation}
The barrier strength parameter is $\alpha_s$ and $c$ is the solute concentration
[!citep](hu2016multiscale).


### Pinning Point Density Evolution id=pinning_evolution

The evolution of the pinning point number density is incremented on each slip plane, $\Omega$,
\begin{equation}
    N_d^{\Omega} = \Delta N_d^{\Omega} + {N_d}_o.
\end{equation}
where the per-slip plane increment is calculated as a function of both self and latent evolution:
\begin{equation}
    \Delta N_d^{\Omega} = \Delta N^{\Omega}_{self} + \Delta N^{\Omega}_{latent}.
\end{equation}
From [!citep](hu2016multiscale, hu2016evaluation), the pinning point evolution due to interaction on the self slip plane $\Omega$ is defined as
\begin{equation}
    \Delta N^{\Omega}_{self} = \frac{j_s}{s} \Delta \gamma^{\Omega}
\end{equation}
and the pinning point density evolution due to latent planes is defined as
\begin{equation}
    \Delta N^{\Omega}_{latent} = \sum_{\omega \neq \Omega} \frac{j}{s}\frac{1}{(m-1)} \Delta \gamma^{\omega}
\end{equation}
where $m$ is the number of slip planes. For an FCC crystal, $m$ is defined as four. The coefficient ratio $j_s/s$ is the combined self-hardening coefficient and $j/s$ is the latent hardening combined coefficient ratio [!citep](hu2016multiscale).


## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/fcc_hucocks_single_crystal/fcc_hucocks_tensile_111orientation.i block=Materials/trial_xtalpl

`CrystalPlasticityFCCDislocationLinkHuCocksUpdate` must be run in conjunction with the crystal
plasticity specific stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/fcc_hucocks_single_crystal/fcc_hucocks_tensile_111orientation.i block=Materials/stress

!syntax parameters /Materials/CrystalPlasticityFCCDislocationLinkHuCocksUpdate

!syntax inputs /Materials/CrystalPlasticityFCCDislocationLinkHuCocksUpdate

!syntax children /Materials/CrystalPlasticityFCCDislocationLinkHuCocksUpdate

!bibtex bibliography

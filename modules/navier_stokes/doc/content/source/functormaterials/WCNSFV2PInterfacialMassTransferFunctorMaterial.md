# WCNSFV2PInterfacialMassTransferFunctorMaterial

This material computes the interfacial mass transfer rate of the two-phase mixture model, the
$\Gamma$ of the dispersed phase mass balance, from the transported interfacial area concentration.
It exists so that the rate is closed on the solved area rather than supplied, which is what keeps the
phase equation and the interfacial area transport equation from being driven by two independently
prescribed rates.

## Formulation

The rate follows the interfacial energy jump condition. Heat reaching the interface converts mass
from one phase to the other, so the rate is the interfacial heat flux divided by the latent heat,

\begin{equation}
  \Gamma = \frac{\chi_p\, h_i \left(T - T_{sat}\right)}{h_{fg}} ~,
\end{equation}

with $\chi_p$ the interfacial area concentration, $h_i$ the interfacial heat transfer coefficient per
unit area and $h_{fg}$ the latent heat. Positive generates the dispersed phase.

The mixture model carries one energy equation and therefore one temperature, so the phases are in
thermal equilibrium with each other and the driving potential is the departure of that shared
temperature from saturation, not a difference between phase temperatures. The interface is at
saturation, so the enthalpy the transfer must supply is $h_{fg}$. This is the driving potential the
mixture-model phase change closures of the CFD codes use, with the rate coefficient computed from the
solved area instead of being tuned.

## The heat transfer coefficient

The coefficient is the bubbly-flow interfacial Nusselt number of RELAP5/MOD3, the modified Lee-Ryley
correlation of Section 4.1.1.1.1 of [!cite](relap5manual),

\begin{equation}
  h_i = \frac{k_c}{d_b}\left(2 + 0.74\, Re_b^{1/2}\right) ~,
  \qquad d_b = \frac{\psi \alpha}{\chi_p} ~,
  \qquad Re_b = \frac{\rho_c d_b \left|\vec{u}_s\right|}{\mu_c} ~,
\end{equation}

with the Prandtl number dependence dropped, as that reference drops it for bubbly flow. The particle
size inverts the interfacial area concentration through the same relation
[LinearWCNSFV2PInterfaceAreaSourceSink.md](LinearWCNSFV2PInterfaceAreaSourceSink.md) uses, so both
describe the same particle population, and
[!param](/FunctorMaterials/WCNSFV2PInterfacialMassTransferFunctorMaterial/shape_factor) has to match
the value that object is given.

RELAP5 forms its volumetric coefficient as the product of this correlation with an interfacial area
of $3.6\alpha/d_b$, obtained from a critical Weber number. The transported area takes the place of
that algebraic estimate, and that substitution is the only change made to the reference closure: the
heat transfer physics is the correlation, the geometry is solved for.

## What this closure does not cover

Three limitations are worth stating before results are read.

The correlation was assessed against the algebraic area it was published with, so the product of it
with a transported area is no longer the quantity that reference validated. Comparisons against
boiling data are needed on their own account rather than inherited.

RELAP5 takes the larger of this correlation and a Plesset-Zwick bubble growth rate, the second
dominating at strong superheat. Only the Lee-Ryley branch is evaluated here, so rapid flashing is
outside what this closure has been written for.

Wall nucleation is not included. This is bulk transfer between phases already in contact, which
grows the particles present at a fixed number density, and that is the assumption the two thirds
exponent of the interfacial area source encodes. Vapour generated at a heated wall creates new
particles instead, and representing it needs a nucleation source the interfacial area equation does
not have.

## Use through the mixture Physics

The rate is normally created by
[WCNSLinearFVTwoPhaseMixturePhysics.md](WCNSLinearFVTwoPhaseMixturePhysics.md) rather than by hand.
Supplying
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/interfacial_area),
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/T_saturation)
and
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/interfacial_latent_heat)
builds this material and hands its property to the phase equation and to the latent heat term of the
energy equation. That property is named `<name of the Physics block>_interfacial_mass_transfer_rate`,
which is the name to give the
[!param](/LinearFVKernels/LinearWCNSFV2PInterfaceAreaSourceSink/mass_transfer_rate) of the
interfacial area source and sink, so that the area equation reads the same rate.

Supplying
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/interfacial_mass_transfer)
instead prescribes the rate, which is useful for exercising the coupling against a rate with an
analytic answer. The two are mutually exclusive.

!syntax parameters /FunctorMaterials/WCNSFV2PInterfacialMassTransferFunctorMaterial

!syntax inputs /FunctorMaterials/WCNSFV2PInterfacialMassTransferFunctorMaterial

!syntax children /FunctorMaterials/WCNSFV2PInterfacialMassTransferFunctorMaterial

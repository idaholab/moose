# Brine and carbon dioxide
[`PorousFlowFluidStateBrineCO2`](/porous_flow/PorousFlowFluidStateBrineCO2.md)

The brine-CO$_2$ model available in PorousFlow is a high precision equation of state
for brine and CO$_2$, including the mutual solubility of CO$_2$ into the liquid brine
and water into the CO$_2$-rich gas phase. This model is suitable for simulations of
geological storage of CO$_2$ in saline aquifers.

The mass fractions of CO$_2$ in the liquid phase and H$_2$O in the gas phase are calculated
using the accurate fugacity-based formulation of \citet{spycher2003} and \citet{spycher2005}.

The [`PorousFlowFluidStateBrineCO2`](/porous_flow/PorousFlowFluidStateBrineCO2.md)
`Material` provides all phase pressures, saturation, densities, viscosities etc, as well
as all mass fractions of all fluid components in all fluid phases in a single material.

To allow an initial saturation to be specified, the
[`PorousFlowFluidStateBrineCO2IC`](/porous_flow/PorousFlowFluidStateBrineCO2IC.md) initial
condition is provided. This initial condition calculates the total mass fraction of CO$_2$
summed over all phases for a given saturation.

##References
\bibliographystyle{unsrt}
\bibliography{porous_flow.bib}

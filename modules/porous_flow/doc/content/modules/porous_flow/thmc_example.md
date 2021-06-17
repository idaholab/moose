# Cold CO$_{2}$ injection into a reactive, elastic reservoir - a multi-phase THMC problem

[Another example](thm_example.md) describes cold CO$_{2}$ injection into a warmer, elastic reservoir, and the MOOSE results were benchmarked against analytical solutions.  This page extends that example by assuming the reservoir is chemically reactive to demonstrate how to solve thermal-hydrualic-mechanical-chemical (THMC) problems.  There are two possibilities for including geochemical reactions in such a model:

1. use the simple chemical-reactions functionality built into the PorousFlow module, as described in this page;
2. couple with the Geochemistry module for more sophisticated geochemical modelling functionality

The geochemistry included here is only meant to illustrate how the functionality of the PorousFlow module could be used, and is not intended to represent reactions in any realistic reservoir.

## The chemical reaction

Consider a hypothetical, kinetically-controlled, dissolution reaction of the form
\begin{equation}
\sum_{i}\nu_{i}m_{i} \rightleftarrows M \ .
\end{equation}
In this equation

- $\nu_{i}$ \[dimensionless\] is the stoichiometric coefficient for basis chemical species $i$
- $m_{i}$ \[mol\] is the mole number of the basis chemical species $i$
- $M$ \[mol\] is the mol number of mineral that is being dissolved

The general form of the reaction rate for such an equation in PorousFlow is
\begin{equation}
I = S\phi  r AV \exp\left(\frac{E}{R}\left(\frac{1}{T} - \frac{1}{T_{\mathrm{ref}}}\right) \right) \left| 1 - \left(\frac{\prod_{i} a_{i}^{\nu_{i}}}{K}\right)^{\theta}  \right|^{\eta}
\end{equation}
where

- $I$ \[L(mineral)/L(solution)/s\] is the reaction rate
- $S$ \[dimensionless\] is the saturation of the phase involved in the chemical reaction
- $\phi$ \[dimensionless\] is the porosity
- $r$ \[mol.m$^{-2}$.s$^{-1}$\] is the intrinsic reaction rate
- $A$ \[m$^{2}$/L(solution)\] is the specific reactive surface area
- $V$ \[L(mineral).mol$^{-1}$\] is the molar volume of the mineral
- $E$ \[J.mol$^{-1}$\] is the activation energy
- $R$ \[J.mol$^{-1}$.K$^{-1}$\] is the gas constant
- $T$ \[K\] is the temperature
- $T_{\mathrm{ref}}$ \[K\] is the reference temperature
- $a_{i}$ is the activity of the primary species $i$
- $K$ is the equilibrium constant for the reaction
- $\eta$ and $\theta$ are dimensionless exponents

This reaction rate without the $S \phi$ term is computed using a [PorousFlowAqueousPreDisChemistry](PorousFlowAqueousPreDisChemistry.md) Material.  The time-dependent volume-fraction of mineral is computed using a [PorousFlowAqueousPreDisMineral](PorousFlowAqueousPreDisMineral.md) Material.

In the case at hand, it is assumed that the hypothetical reaction only occurs when the gas phase is present.  It is also assumed that the activities of all the primary chemical species are fixed.  Physically, this could be due to the CO$_{2}$(g) causing changes in the equilibrium aqueous geochemistry, perhaps by altering the pH, which in turn causes mineral dissolution.  To model this, $S$ can be taken to be the gas saturation and $\prod_{i}a_{i} = 1$.  It is also assumed the Arrhenius prefactor is irrelevant.

These idealised, hypothetical assumptions lead to a particularly simple reaction rate.  In the presence of gas, the reaction rate is controlled by the temperature-dependence of the equilibrium constant, and reads
\begin{equation}
I = S\phi  r AV  \left| 1 - K^{-1} \right| \ ,
\end{equation}
with $\eta = 1 = \theta$.  Assuming that $K\neq 1$ and there is some gas present, this reaction will continue until all the mineral has dissolved.

Using $rAV = 10^{-6}\,$s$^{-1}$, the Materials that compute the reaction rate and the resulting Mineral concentration (m$^{3}$(mineral)/m$^{3}$(porous-material)) are

!listing modules/porous_flow/examples/thm_example/2D_c.i start=[predis] end=[predis_nodes]

The [PorousFlowDictator](PorousFlowDictator.md) must be enhanced to include the number of reactions and a specification of the phase number of the phase involved in these reactions.

!listing modules/porous_flow/examples/thm_example/2D_c.i start=[dictator] end=[pc]

The phase involved in the reactions is usually the aqueous phase, hence the `aqueous` in the keywords, but in this case setting `aqueous_phase_number = 1` means the $S$ in the above equation is actually the gas saturation.

The equilibrium constant is assumed to be temperature dependent:
\begin{equation}
\log_{10}K = \frac{358 - T}{358 - 294} \ .
\end{equation}

!listing modules/porous_flow/examples/thm_example/2D_c.i start=[eqm_const_auxk] end=[porosity_auxk]


## Impact of dissolution on porosity

In PorousFlow, the [porosity](PorousFlowPorosity.md) can depend on mineral concentration as well as the effective porepressure, strain and temperature.  In this case, assume that the porosity only depends on the degree of mineralisation:
\begin{equation}
\phi = \phi_{0} - M + M_{\mathrm{ref}} \ .
\end{equation}
Here:

- $\phi$ \[dimensionless\] is the porosity
- $\phi_{0}$ \[dimensionless\] is the reference porosity when $M=M_{\mathrm{ref}}$
- $M$ \[m$^{3}$(mineral)/m$^{3}$(porous-material)\] is the "concentration" of the mineral that is dissolving
- $M_{\mathrm{ref}}$ \[m$^{3}$(mineral)/m$^{3}$(porous-material)\] is the reference concentration

For this example, assume $\phi_{0} = 0.2$ and $M_{\mathrm{ref}} = 0.1$.  The relevant Material is:

!listing modules/porous_flow/examples/thm_example/2D_c.i start=[porosity_reservoir] end=[permeability_reservoir]

## Results

[2D_c_fig] shows the results.  The porosity changes due to reservoir dissolution are kinetically controlled and temperature-dependent, so do not occur as soon as the gas front reaches any given point.  Nevertheless, after gas has occupied a region for some time and cooled it, the mineral will completely dissolve, resulting in a porosity of $\phi = 0.3$.

Comparing with the case that has no chemistry active ([sg_cf_fig] from [here](thm_example.md)), it is seen that the CO$_{2}$ front appears at a similar position, but the gas saturation profile is influenced by the porosity changes.  

!media media/porous_flow/2D_c_fig.png caption=Gas saturation and porosity are impacted through reservoir-rock dissolution.  id=2D_c_fig

!media media/porous_flow/2D_thm_compare_sg_fig.png caption=Gas saturation when there is no chemistry active.  id=sg_cf_fig


!bibtex bibliography

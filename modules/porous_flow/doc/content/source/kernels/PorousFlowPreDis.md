# PorousFlow PreDis

!syntax description /Kernels/PorousFlowPreDis

This `Kernel` implements the residual
\begin{equation*}
  \phi_{\mathrm{old}} S_{\mathrm{aq}} \sum_{m}\nu_{m}\rho_{m}I_{m} \ .
\end{equation*}
In this equation, $\phi$ is the porosity (only the *old* value is used), $S_{\mathrm{aq}}$ is the aqueous saturation, the sum over $m$ is a sum
over all the precipitated-or-dissolved (`PreDis`) mineral species,
$\nu_{m}$ are stoichiometric coefficients, $\rho_{m}$ is the density
of a solid lump of the mineral, and $I_{m}$ is the mineral
reaction rate (m$^{3}$(precipitate)/m$^{3}$(solution).s$^{-1}$) which is computed by
[PorousFlowAqueousPreDisChemistry](PorousFlowAqueousPreDisChemistry.md).

Details concerning precipitation-dissolution kinetic chemistry may be found in the
[`chemical reactions`](/chemical_reactions/index.md) module.

!alert warning
The numerical implementation of the chemical-reactions part of `PorousFlow` is quite simplistic, with
very few guards against strange numerical behavior that might arise during the non-linear iterative
process that MOOSE uses to find the solution.  Therefore, care must be taken to define your chemical
reactions so that the primary species concentrations remain small, but nonzero, and that
mineralisation does not cause porosity to become negative or exceed unity.

This `Kernel` is usually added to a
[PorousFlowMassTimeDerivative](PorousFlowMassTimeDerivative.md)
`Kernel` to simulate precipitation-dissolution of a mineral from some primary chemical species.  For
instance in the case of just one precipitation-dissolution kinetic reaction
\begin{equation}
a + b  \rightleftharpoons  \mathrm{mineral}
\end{equation}
and including diffusion and dispersion, the `Kernels` block looks like

!listing modules/porous_flow/test/tests/chemistry/2species_predis.i start=[mass_a] end=[UserObjects]

Appropriate stoichiometric coefficients must be supplied to this `Kernel`.  Consider the reaction
system
\begin{equation}
\begin{array}{rcl}
 1a + 2b - 3c & \rightleftharpoons & \mathrm{mineral0} \\
4a -5b + 6c   & \rightleftharpoons & \mathrm{mineral1} \ .
\end{array}
\end{equation}

Then the stoichiometric coefficients for the `PorousFlowPreDis` Kernels would be:

 - `stoichiometry = '1 4'` for Variable `a`
 - `stoichiometry = '2 -5'` for Variable `b`
 - `stoichiometry = '-3 6'` for Variable `c`

!alert note
This `Kernel` lumps the mineral masses to the nodes.  It also only uses the *old* values of porosity,
which is an approximation: see [porosity](/porous_flow/porosity.md) for a discussion.

See [mass lumping](/porous_flow/mass_lumping.md) for details.

!syntax parameters /Kernels/PorousFlowPreDis

!syntax inputs /Kernels/PorousFlowPreDis

!syntax children /Kernels/PorousFlowPreDis

# PorousFlow Aqueous PreDis Mineral

!syntax description /Materials/PorousFlowAqueousPreDisMineral

This computes mineral concentrations that result from a precipitation-dissolution (`PreDis`) kinetic
reaction system.  The Material Property may be saved in an `AuxVariable` using the
[PorousFlowPropertyAux](PorousFlowPropertyAux.md) `AuxKernel`.  The mineral concentration computed
has units m$^{3}$(precipitate)/m$^{3}$(porous material).

!alert warning
The numerical implementation of the chemical-reactions part of `PorousFlow` is quite simplistic, with
very few guards against strange numerical behavior that might arise during the non-linear iterative
process that MOOSE uses to find the solution.  Therefore, care must be taken to define your chemical
reactions so that the primary species concentrations remain small, but nonzero, and that
mineralisation doesn't cause porosity to become negative or exceed unity.

The computation uses a *lagged* approach for porosity to calculate the mineral concentration, $C$:
\begin{equation}
C = C_{\mathrm{old}} + \phi_{\mathrm{old}} I S_{\mathrm{aq}} \mathrm{d}t \ ,
\end{equation}
where $I$ is the reaction rate (dependent on primary concentrations,
etc) and $S_{\mathrm{aq}}$ is the saturation of the aqueous phase.
Notice the use of the *old* value for porosity.  This is an
approximation.  It breaks the cyclic dependency between mineral
concentration and porosity: see [Porosity](/porous_flow/porosity.md)
for more details.

!syntax parameters /Materials/PorousFlowAqueousPreDisMineral

!syntax inputs /Materials/PorousFlowAqueousPreDisMineral

!syntax children /Materials/PorousFlowAqueousPreDisMineral

# PorousFlow Aqueous PreDis Chemistry
!syntax description /Materials/PorousFlowAqueousPreDisChemistry

This computes reaction rates resulting from a
precipitation-dissolution (`PreDis`) kinetic reaction system.

!!! warning
    The numerical implementation of the chemical-reactions part of `PorousFlow` is quite simplistic, with very few guards against strange numerical behavior that might arise during the non-linear iterative process that MOOSE uses to find the solution.  Therefore, care must be taken to define your chemical reactions so that the primary species concentrations remain small, but nonzero, and that mineralisation doesn't cause porosity to become negative or exceed unity.

Details concerning precipitation-dissolution kinetic chemistry may be found in the
[`chemical reactions`](/chemical_reactions/index.md) module.   There are three main differences between the `chemical reactions` module and `PorousFlow`.  These are:

 - The molar volumes must be specified in `PorousFlow`.  This is so that the concentrations may be measured in $m^{3}/m^{3}$ rather than mol.m$^{-3}$.

 - The equilibrium constants are defined in absolute terms, not as a log10.  Therefore a `PorousFlow` equilibrium constant might be $10^{-6}$, while in `chemical_reactions` it is simply $-6$.
 
 - Unlike the `chemical reactions` module, users of `PorousFlow` must specify the stoichiometric coefficients themselves.  In each reaction, the primary concentrations (variables) must be brought to the left-hand-side.  The right-hand-sides are the minerals, by definition.  For instance, consider a 2-reaction system consisting of 3 primary variables, $a$, $b$ and $c$.  The reactions are
 \begin{equation}
 \begin{array}{rcl}
 1a + 2b - 3c & \rightleftharpoons & \mathrm{mineral0} \\
4a -5b + 6c   & \rightleftharpoons & \mathrm{mineral1}
\end{array}
\end{equation}
Then the `reactions` input is `1 2 -3 4 -5 6`.

!syntax parameters /Materials/PorousFlowAqueousPreDisChemistry

!syntax inputs /Materials/PorousFlowAqueousPreDisChemistry

!syntax children /Materials/PorousFlowAqueousPreDisChemistry

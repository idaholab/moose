# GrandPotentialSinteringMaterial

Defines switching functions and thermodynamic parameters for the grand potential
sintering model.
The GrandPotentialSinteringMaterial defines switching functions for void and solid
phases as well as switching functions for solid and grain boundary regions.
It also defines the susceptibilty, vacancy densities and concentrations, potential
densities, and the phase field free energy terms $m$, $\kappa$, and $\gamma$.

There are three energy models that can be used for the solid-phase: parabolic
($f_s = \frac12k_s(c_s-c_s^{eq})^2$), dilute ($f_s = \frac{E_f}{V_a}+\frac{k_BT}{V_a}(c \ln c - c)$),
and ideal ($f_s = c\frac{E_f}{V_a} + \frac{k_BT}{V_a}[c\ln c + (1-c)\ln(1-c)]$).

The equilibrium solid-phase vacancy concentration is determined in a separate
material and referenced by this one.
This is done to maximize the flexibility of the sintering model to include effects
such as GB vacancy segregation and stoichiometry effects.

Additional option for strict mass conservation formulation can be defined using 'mass_conservation' flag set to "true", which generates the coefficients for void and solid phases.

!syntax parameters /Materials/GrandPotentialSinteringMaterial

!syntax inputs /Materials/GrandPotentialSinteringMaterial

!syntax children /Materials/GrandPotentialSinteringMaterial

!bibtex bibliography

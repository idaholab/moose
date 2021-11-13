# ElectrochemicalSinteringMaterial

Defines switching functions and thermodynamic parameters for the electrochemical grand potential
sintering model. In this model, an arbitrary number of charged defect species can exist.
The ElectrochemicalSinteringMaterial defines switching functions for void and solid
phases as well as switching functions for solid and grain boundary regions.
It also defines the grand potential densities, and the phase field free energy terms
$m$, $\kappa$, and $\gamma$.
For each defect species, an ElectrochemicalDefectMaterial object must also be included to
calculate the susceptibility and densities in each phase.

There are two energy models that can be used for the solid phase: parabolic
($f_s = \frac12k_s(c_s-c_s^{eq})^2$) and dilute ($f_s = \frac{E_f}{V_a}+\frac{k_BT}{V_a}(c \ln c - c)$).
The void phase uses a parabolic free energy. There is also a contribution to the grand potential
from the electric potential of the form $\frac12 \epsilon | \nabla V|^2$.

The equilibrium solid-phase vacancy concentrations for defect species are determined in separate
materials and need to be supplied to this one.
This is done to maximize the flexibility of the sintering model to include effects
such as GB vacancy segregation and stoichiometry effects.

!syntax parameters /Materials/ElectrochemicalSinteringMaterial

!syntax inputs /Materials/ElectrochemicalSinteringMaterial

!syntax children /Materials/ElectrochemicalSinteringMaterial

!bibtex bibliography

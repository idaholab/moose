# ElectrochemicalDefectMaterial

The ElectrochemicalDefectMaterial defines the susceptibility $\chi$ and defect densities $n^s, n^v$
in the solid and void phases for a defect species
in the electrochemical sintering model. One instantiation of this material must be included for
each defect species. Other material properties for this model are calculated by the
ElectrochemicalSinteringMaterial, which also must be included.

There are two energy models that can be used for the solid phase: parabolic
($f_s = \frac12k_s(c_s-c_s^{eq})^2$) and dilute ($f_s = \frac{E_f}{V_a}+\frac{k_BT}{V_a}(c \ln c - c)$).
The void phase uses a parabolic free energy. There is also a contribution to the grand potential
from the electric potential of the form $\frac12 \epsilon | \nabla V|^2$.

The equilibrium solid-phase vacancy concentrations for defect species are determined in separate
materials and need to be supplied to this one.
This is done to maximize the flexibility of the sintering model to include effects
such as GB vacancy segregation and stoichiometry effects.

!syntax parameters /Materials/ElectrochemicalDefectMaterial

!syntax inputs /Materials/ElectrochemicalDefectMaterial

!syntax children /Materials/ElectrochemicalDefectMaterial

!bibtex bibliography

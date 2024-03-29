# DesorptionFromMatrix

!syntax description /Kernels/DesorptionFromMatrix

The weak form for this term is:

!equation
(\dot{m}, \psi)

where $\dot{m}$ is the mass rate from the matrix to the porespace and $\psi$ the test function.
The mass rates used in the kernel are retrieved directly from material properties, with the property name
following the same convention as [LangmuirMaterial.md] and [MollifiedLangmuirMaterial.md], e.g. "mass_rate_from_matrix"
for the mass rate, and "dmass_rate_from_matrix_dC" / "dmass_rate_from_matrix_dp" for its derivatives with regards to the
nonlinear variables, concentration and pore pressure.

!syntax parameters /Kernels/DesorptionFromMatrix

!syntax inputs /Kernels/DesorptionFromMatrix

!syntax children /Kernels/DesorptionFromMatrix
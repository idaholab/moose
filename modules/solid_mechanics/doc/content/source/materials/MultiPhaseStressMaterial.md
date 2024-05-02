# MultiPhaseStressMaterial

!syntax description /Materials/MultiPhaseStressMaterial

The `MultiPhaseStressMaterial` is used in coupled phase field - solid mechanics calculations to compute
the mechanical deformation of multi-phase regions.

The switching (mixing) coefficients which blends the phases' stresses together must be provided
as a vector of material properties, using the [!param](/Materials/MultiPhaseStressMaterial/h) parameter.
A vector of material properties has a syntax similar to "h1 h2 h3 h4" if those are the names of the material properties.

The global stress is then defined as

!equation
\boldsymbol{\sigma}_g(\vec{r}) = \sum_i^N h_i(\vec{r}) \boldsymbol{\sigma}_{i}(\vec{r})

where $h_i$ are the switching function material properties, $\boldsymbol{\sigma}_{i}$ the phase stresses for each phase, $N$ the number of
phases considered, and $\vec{r}$ denotes the spatial dependence.

The material also defines the (rank four tensor of) derivatives of the stress with regards to the strain, accounting
for each phases' derivatives.

!syntax parameters /Materials/MultiPhaseStressMaterial

!syntax inputs /Materials/MultiPhaseStressMaterial

!syntax children /Materials/MultiPhaseStressMaterial

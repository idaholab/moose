# TwoPhaseStressMaterial

!syntax description /Materials/TwoPhaseStressMaterial

The `TwoPhaseStressMaterial` is used in coupled phase field - solid mechanics calculations to compute
the mechanical deformation of two phase regions.

The switching (mixing) coefficient which blends the two phase stresses together must be provided
as a material property, using the [!param](/Materials/TwoPhaseStressMaterial/h) parameter.

The global stress is then defined as

!equation
\boldsymbol{\sigma}_g(\vec{r}) = h(\vec{r}) \boldsymbol{\sigma}_{b}(\vec{r}) + (1 - h(\vec{r})) \boldsymbol{\sigma}_{a}(\vec{r}) + \boldsymbol{\sigma}_{g,extra}(\vec{r})

where $h$ is the switching function, $\boldsymbol{\sigma}_{b}$ and $\boldsymbol{\sigma}_{a}$ the stresses in the phase B and A respectively,
$\boldsymbol{\sigma}_{g,extra}$ an extra global stress, and $\vec{r}$ denotes the spatial dependence.

The material also defines the (rank four tensor of) derivatives of the stress with regards to the strain, accounting
for both phases' derivatives but neglecting any dependence of the extra global stress.

!syntax parameters /Materials/TwoPhaseStressMaterial

!syntax inputs /Materials/TwoPhaseStressMaterial

!syntax children /Materials/TwoPhaseStressMaterial

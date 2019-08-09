# AD Compute Multiple Porous Inelastic Stress

!syntax description /Materials/ADComputeMultiplePorousInelasticStress

## Description

`ADComputeMultiplePorousInelasticStress` computes the stress, the consistent tangent operator (or an
approximation), and a decomposition of the strain into elastic and inelastic components for a series
of different inelastic material models (e.g. creep and plasticity) which inherit from
`ADStressUpdateBase`. `ADComputeMultiplePorousInelasticStress` operates almost exactly the same as
[ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md) but adds a `porosity` material
property. After a inelastic strain $\epsilon_{in}$ is calculated, the porosity, $f$ is updated by,
\begin{equation}
  f(t+\Delta t) = f(t) + [1.0 - f(t)] * [\epsilon_{in}(t+\Delta t) - \epsilon_{in}(t)].\text{tr}()
\end{equation}

Refer to [ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md) for the remainder of
the details of how `ADComputeMultiplePorousInelasticStress` computes stress.

## Example Input Files

`ADComputeMultiplePorousInelasticStress` can take individual or combinations of different inelastic
material models that inherit from `ADStressUpdateBase`.

!listing modules/tensor_mechanics/test/tests/ad_viscoplasticity_stress_update/lps_dual.i block=Materials

!syntax parameters /Materials/ADComputeMultiplePorousInelasticStress

!syntax inputs /Materials/ADComputeMultiplePorousInelasticStress

!syntax children /Materials/ADComputeMultiplePorousInelasticStress

!bibtex bibliography

# Compute Multiple Porous Inelastic Stress

!syntax description /Materials/ComputeMultiplePorousInelasticStress

## Description

`ComputeMultiplePorousInelasticStress` computes the stress, the consistent tangent operator (or an
approximation), and a decomposition of the strain into elastic and inelastic components for a series
of different inelastic material models (e.g. creep and plasticity) which inherit from
`StressUpdateBase`. `ComputeMultiplePorousInelasticStress` operates almost exactly the same as
[ComputeMultipleInelasticStress](ComputeMultipleInelasticStress.md) but adds a `porosity` material
property. After a inelastic strain $\epsilon_{in}$ is calculated, the porosity, $f$ is updated by,
\begin{equation}
  f(t+\Delta t) = f(t) + [1.0 - f(t)] * [\epsilon_{in}(t+\Delta t) - \epsilon_{in}(t)].\text{tr}()
\end{equation}

Refer to [ComputeMultipleInelasticStress](ComputeMultipleInelasticStress.md) for the remainder of
the details of how `ComputeMultiplePorousInelasticStress` computes stress.

## Example Input Files

`ComputeMultiplePorousInelasticStress` can take individual or combinations of different inelastic
material models that inherit from `StressUpdateBase`.

!listing modules/tensor_mechanics/test/tests/viscoplasticity_stress_update/lps_dual.i block=Materials

!syntax parameters /Materials/ComputeMultiplePorousInelasticStress

!syntax inputs /Materials/ComputeMultiplePorousInelasticStress

!syntax children /Materials/ComputeMultiplePorousInelasticStress

!bibtex bibliography

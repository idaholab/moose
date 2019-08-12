# AD Compute Multiple Porous Inelastic Stress

!syntax description /Materials/ADComputeMultiplePorousInelasticStress

## Description

`ADComputeMultiplePorousInelasticStress` computes the stress and a decomposition of the strain into
elastic and inelastic components for a series of different inelastic material models (e.g. creep and
plasticity) which inherit from `ADStressUpdateBase`. The tangent operator and relevant Jacobian
information is computed using automatic differentiation techniques.
`ADComputeMultiplePorousInelasticStress` operates almost exactly the same as
[ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md) but adds a `porosity`
material property. After a inelastic strain $\epsilon_{in}$ is calculated, the porosity, $f$ is
updated by,
\begin{equation}
  \dot{f} = [1.0 - f] * \dot{\epsilon}_{in}.\text{tr}()
\end{equation}
The porosity can then in turn be used by [viscoplasticity methods](ADViscoplasticityStressUpdate.md)
or other porosity dependent materials

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

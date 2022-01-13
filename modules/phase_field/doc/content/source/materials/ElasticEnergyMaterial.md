# ElasticEnergyMaterial

!syntax description /Materials/ElasticEnergyMaterial

This material generates an elastic free energy contribution consistent with the
TensorMechanics stress divergence kernels. This allows for proper coupling of
the phase field problem and the mechanics problem. The contribution of the
`ElasticEnergyMaterial` and the chemical free energy (possibly defined using a
`DerivativeParsedMaterial`) are then summed up using a `DerivativeSumMaterial`
to form a _total free energy_ which is passed to the phase field kernels.

The `ElasticEnergyMaterial` applies a _linear_ elasticity model. It couples in
the strain $\epsilon$ and the elasticity tensor $C_{ijkl}$ to compute the elastic
energy

\begin{equation}
E = \frac12 C_{ijkl} \epsilon_{ij} \epsilon_{kl}.
\end{equation}

The material utilizes the derivatives of $C$ and $\epsilon$ to provide the
derivatives of $E$, which are required by the phase field equations.

```yaml
[Materials]
  # material subblocks that define stress and elasticity tensor properties
  # (and necessary derivatives) are omitted

  [./elasticenergy]
    type = ElasticEnergyMaterial
    args = 'c'
  [../]
[]
```

!syntax parameters /Materials/ElasticEnergyMaterial

!syntax inputs /Materials/ElasticEnergyMaterial

!syntax children /Materials/ElasticEnergyMaterial

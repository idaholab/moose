# ComputeMultipleInelasticStressSmallStrain

!syntax description /Materials/ComputeMultipleInelasticStressSmallStrain

## Description

`ComputeMultipleInelasticStressSmallStrain` computes the stress and decomposition of strain into elastic and inelastic components for materials with multiple inelastic models (e.g., plasticity, creep) using a **small strain formulation**. This class is analogous to [ComputeMultipleInelasticStress](ComputeMultipleInelasticStress.md) but uses total strains instead of strain increments and does not include finite strain rotation effects.

### Small Strain vs Finite Strain

The key differences between this class and `ComputeMultipleInelasticStress` are:

- **Strain Formulation**: Uses total small strains $\boldsymbol{\varepsilon} = \frac{1}{2}(\nabla \mathbf{u} + \nabla \mathbf{u}^T)$ instead of logarithmic strain increments
- **No Rotations**: Does not perform finite strain rotations or use rotation increment tensors
- **Parent Class**: Inherits from [ComputeStressBase](ComputeStressBase.md) instead of [ComputeFiniteStrainElasticStress](ComputeFiniteStrainElasticStress.md)
- **Total Formulation**: Works with total strains rather than incremental formulation

### Iterative Solution Procedure

The material uses an iterative return mapping algorithm to find the admissible stress state that satisfies all inelastic models simultaneously:

1. For each iteration:
   - Loop over all inelastic models
   - For each model, compute the current elastic strain by subtracting inelastic strains from other models
   - Form trial stress: $\boldsymbol{\sigma}^{trial} = \mathbf{C} : \boldsymbol{\varepsilon}^{elastic}$
   - Call the model's `updateState` method to compute admissible stress and inelastic strain increment
   - Track min/max stress for convergence checking

2. Check convergence based on the L2 norm of stress differences between models
3. If not converged and iterations remain, repeat

4. Once converged:
   - Combine inelastic strains from all models using specified weights
   - Compute final elastic strain: $\boldsymbol{\varepsilon}^{elastic} = \boldsymbol{\varepsilon}^{total} - \boldsymbol{\varepsilon}^{inelastic}$
   - Compute Jacobian (tangent operator) if needed

### Strain Decomposition

The mechanical strain is decomposed as:

\begin{equation}
\boldsymbol{\varepsilon}^{mechanical} = \boldsymbol{\varepsilon}^{elastic} + \boldsymbol{\varepsilon}^{inelastic}
\end{equation}

where the combined inelastic strain is a weighted sum:

\begin{equation}
\boldsymbol{\varepsilon}^{inelastic} = \sum_i w_i \boldsymbol{\varepsilon}^{inelastic}_i
\end{equation}

## Usage Notes

- This material should be paired with a small strain calculator such as [ComputeSmallStrain](ComputeSmallStrain.md)
- Do NOT use with incremental strain formulations (e.g., `ComputeFiniteStrain`)
- Inelastic models should be listed with creep models first, plasticity models last
- The `perform_finite_strain_rotations` parameter from the finite strain version is not needed here

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/multiple_inelastic_stress_small_strain/two_models.i block=Materials/stress

!syntax parameters /Materials/ComputeMultipleInelasticStressSmallStrain

!syntax inputs /Materials/ComputeMultipleInelasticStressSmallStrain

!syntax children /Materials/ComputeMultipleInelasticStressSmallStrain

## See Also

- [ComputeMultipleInelasticStress](ComputeMultipleInelasticStress.md) - Finite strain version
- [ComputeSmallStrain](ComputeSmallStrain.md) - Small strain calculator to pair with this class
- [StressUpdateBase](StressUpdateBase.md) - Base class for inelastic models

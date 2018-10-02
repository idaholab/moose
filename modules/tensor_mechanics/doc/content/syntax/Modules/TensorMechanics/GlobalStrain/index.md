# Global Strain

The global strain system enables capturing the volume change, shear deformation etc. while still maintaining the periodicity. It relaxes the stresses along the periodic directions and allows the corresponding deformation. This, combined with the periodic displacement, enforces the strain periodicity.

The [global strain](/GlobalStrain.md) is computed by setting integral of each of the stress components $\sigma_{ij}$ in the periodic direction to zero such that

\begin{equation}
	\int_\Omega {(\sigma_{ij} - \sigma^0_{ij})} d\Omega = 0 \.
	\label{stress_eq}
\end{equation}

Here, indices $i$ and $j$ corresponds to the periodic directions and associated periodic displacement components, respectively. $\sigma^0_{ij}$ denotes the components of the stress tensor representing the applied loads in the periodic directions. The components of the global strain are considered constant over the whole domain.

The total strain $\epsilon_{ij}^{total}$ at any point is calculated considering the contribution of the global strain tensor as,

\begin{equation}
\epsilon_{ij}^{total} =  \epsilon_{ij} + \epsilon_{ij}^{g} - \epsilon_{ij}^{0}\, \label{strain_eq}
\end{equation}

where $\epsilon_{ij}$ are the [strain components](/ComputeSmallStrain.md) calculated from the displacement gradients, $\epsilon_{ij}^g$ are the global strain components, and $\epsilon_{ij}^0$ are the eigen strain components. An additional [displacement field](/GlobalDisplacementAux.md) $\boldsymbol u^g$ is generated to visualize the effect of the global strain as

\begin{equation}
\boldsymbol {u}^g = \boldsymbol {r} \boldsymbol{\epsilon}^g \, \label{dispg_eq}
\end{equation}

where $\boldsymbol{r}$ is the position vector and $\boldsymbol{\epsilon}^global$ is the global strain tensor. Then, the total displacement is represented as

\begin{equation}
	\boldsymbol {u}^{total} = \boldsymbol {u} + \boldsymbol {u}^g \,
 	\label{disp_eq}
\end{equation}

where $\boldsymbol u$ is the local displacement field calculated from the mechanical equilibrium.

[GlobalStrainAction](/GlobalStrainAction.md) can be used to set up the model for global strain and corresponding displacement calculation. This action can be created using the following syntax.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain_action.i block=Modules/TensorMechanics/GlobalStrain

## Subblocks

The subblocks of the GlobalStrain action triggers MOOSE objects to be built.
It can be applied to the whole domain using a single subblock

```
[Modules/TensorMechanics/GlobalStrain]
  [./all]
    ...
  [../]
[]
```

or multiple subblocks can be used to apply block restrictions to the objects

```
[Modules/TensorMechanics/GlobalStrain]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

!syntax list /Modules/TensorMechanics/GlobalStrain objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/GlobalStrain objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/GlobalStrain objects=False actions=True subsystems=False

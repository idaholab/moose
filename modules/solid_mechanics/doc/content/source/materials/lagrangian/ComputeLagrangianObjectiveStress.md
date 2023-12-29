# ComputeLagrangianObjectiveStress

## Overview

This class is a bit different from the other constitutive model
base classes.  It provides an interface for implementing a constitutive
model using the traditional small deformation, engineering stress
and strain measures that translates this natively small deformations
constitutive model to provide a suitable response for a large
deformation formulation, as implemented in the
[total Lagrangian](TotalLagrangianStressDivergence.md)
and [updated Lagrangian](UpdatedLagrangianStressDivergence.md) kernels.
Specifically, the class provides a way to calculate the Cauchy stress
given the small, engineering stress and to convert the algorithmic tangent
provided by an engineering stress/strain model to a suitable
large deformation tangent tensor.

The user then implements the small stress update $s_{ij}$ and the
associated derivative with respect to the engineering strain
\begin{equation}
      \hat{T}_{ijkl} = \frac{d s_{ij}}{d \varepsilon_{kl}}
\end{equation}
The class then converts these measures to the updated Cauchy stress
and the tangent required by the parent class
[`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md).

## Conversion

The class converts the small strain model by integrating an objective
rate of the Cauchy stress [!cite](simo2006computational).
There are a wide variety of objective rates described in the literature
and the implementation in the Tensor Mechanics module provides  
a general form in which different rates can be implemented.
The current implementation provides two options: the Truesdell rate of the
Cauchy stress and the Jaumann rate of the Cauchy stress.  
The implicit version of the commercial [ABAQUS](https://www.3ds.com/products-services/simulia/products/abaqus/)
FEA code uses the Jaumann rate, so enabling this option allows users to
compare results to that product.

This conversion only needs to happen for large deformation kinematics.
The `ComputeLagrangianObjectiveStress` base class takes the
`large_kinematics` flag as input and only performs the
objective integration process if it is set to `true`.

The process starts by updating the small stress using the user-provided
constitutive model, defined (typically) in terms of the
mechanical strain tensor.  However, as with all constitutive models
designed for use with the Lagrangian kernels, the user can define the
stress update in terms of any kinematic measures provided
by the [new material system](NewMaterialSystem.md).

Most objective rates take the form
\begin{equation}
      \hat{\sigma}_{ij} = s_{ij}=\dot{\sigma}_{ij}-Q_{ik}\sigma_{kj}-\sigma_{ik}Q_{jk}+Q_{kk}\sigma_{ij}
\end{equation}
where $Q_{ik}$ is some kinematic measure and $s_{ij}$ is the small stress,
supplied by the constitutive model.  This equation basically
advects the stress using the kinematic tensor.
The $\hat{\sigma}_{ij}$
suggests that multiple objective rates of the Cauchy stress are possible --
i.e. there is no unique, universally accepted theory.
The choice of the kinematic tensor $Q_{ik}$ defines the
particular objective rate, so long as the model returns the correct tangent
matrix $\hat{T}_{ijkl}$.

The conversion process in `ComputeLagrangianObjectiveStress` must solve this
equation to find the updated Cauchy stress for an arbitrary kinematic
measure $Q$.
It turns out this update is linear and the solution for the updated Cauchy stress
is
\begin{equation}
      \sigma_{ij}=J_{ijmn}^{-1}\left(\sigma_{mn}^{n}+\Delta s_{mn}\right)
\end{equation}
where $\Delta s_{mn}$ is the increment in the small stress over some time
step and
\begin{equation}
      J_{ijmn}=\left(1+\Delta Q_{kk}\right)\delta_{im}\delta_{jn}-\Delta Q_{im}\delta_{jn}-\delta_{im}\Delta Q_{jn}
\end{equation}
with $\Delta Q_{ij} = \Delta t Q_{ij}$, i.e. the increment in the kinematic tensor.

The algorithmic tangent required by [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md)
is then given by
\begin{equation}
      T_{ijkl}=J_{ijmn}^{-1}\left(\hat{T}_{mnst}-\frac{\partial J_{mnst}}{\partial\Delta l_{kl}}\sigma_{st}\right)
\end{equation}
where the $\frac{\partial J_{mnst}}{\partial\Delta l_{kl}}$ tensor is another characteristic of the objective rate.
Rather than implement a 6th order tensor, the class instead implements a function giving the action of
this tensor on the Cauchy stress, i.e.
\begin{equation}
      U_{mnkl} = \frac{\partial J_{mnst}}{\partial\Delta l_{kl}}\sigma_{st}
\end{equation}
The two tensors $Q_{ij}$ and $U_{ijkl}$ then fully-define a particular objective rate.

!alert warning
All directional components of the constitutive model should be advected using the selected
objective rate integration.  This applies to the stress tensor, as described here,
but also any directional internal variable used by the stress update model.
This class cannot automatically perform this advection, meaning it is up to the model
implementer to do it if require.  Note this warning does *not* apply to the
common case of scalar internal variables, as these have no associated direction.

## Specific Objective Rates

The `ComputeLagrangianObjectiveStress` class choices between different objective rates with the `objective_rate`
input parameter.  Currently there are two choices for implemented rates: the (default) Truesdell rate, `truesdell`,
and the Jaumnn rate, `jaumann`.

### The Truesdell Rate of the Cauchy Stress

The Truesdell objective rate is defined by the kinematic tensor
\begin{equation}
      Q_{ik}=l_{ik}
\end{equation}
and the derivative tensor
\begin{equation}
      U_{mnkl}=\delta_{kl}\sigma_{mn}-\delta_{mk}\sigma_{ln}-\delta_{nk}\sigma_{ml}
\end{equation}

### The Jaumann Rate of the Cauchy Stress

The Jaumann objective rate is defined by
\begin{equation}
      Q_{ik}=w_{ik}
\end{equation}
with
\begin{equation}
      w_{ik}=\frac{1}{2}\left(l_{ik}-l_{ki}\right)
\end{equation}
and the associated derivative tensor
\begin{equation}
      U_{mnkl}=\frac{1}{2}\left(\delta_{ml}\sigma_{kn}+\delta_{nl}\sigma_{mk}-\delta_{mk}\sigma_{ln}-\delta_{nk}\sigma_{ml}\right)
\end{equation}

Applying the method is equivalent to updating the stress with the Hughes-Winget
method [!cite](hughes1980finite).

### The Green-Naghdi Rate of the Cauchy Stress

The Green-Naghdi objective rate is defined by
\begin{equation}
  Q_{ik} = \Omega_{ik} = \dot{R}_{ij} R_{kj}.
\end{equation}

The Green-Naghdi rate can be specified by `objective_rate = green_naghdi`.

Let $U^0$ be the derivative tensor of the Truesdell rate, the associated derivative tensor of the Green-Naghdi rate can be written as
\begin{equation}
  U_{ijkl} = U^0_{ijpq} \frac{\partial\Delta\Omega_{pq}}{\partial\Delta l_{kl}}.
\end{equation}

The chain rule further expands this as
\begin{equation}
  \frac{\partial\Delta\Omega_{pq}}{\partial\Delta l_{kl}} = \frac{\partial\Delta\Omega_{pq}}{\partial R_{or}} \frac{\partial R_{or}}{\partial F_{st}} \frac{\partial F_{st}}{\partial \Delta l_{kl}},
\end{equation}
while the first and the third derivatives on the right hand side are trivial, the second one is more involved:
\begin{equation}
  \frac{\partial R_{kl}}{\partial F_{mn}} = \frac{1}{\text{det}(Y)} \left( Z_{km} Y_{nl} - Z_{kn} Z_{lm} \right),
\end{equation}
where
\begin{equation}
  Y = \text{trace}(U) I - U, \quad Z = R Y.
\end{equation}

## Problems With Objective Rates

There are several well-known problems associated with integrating objective rates to provide large deformation constitutive models
based on small strain theory [!cite](simo2006computational,bavzant2014energy).

### Integrated rotations

A typical use for objective rate integration is to provide a constitutive model for materials that only undergo relatively small
stretches in a simulation that will require large rotations.  For example, a user might want to simulate
a cold work forming process for a metal part, where the material will not under large strains but will undergo large rotations.
One challenge with objective rate integration *as implemented here* is that the rotation kinematics are integrated in time, rather
than being applied directly from the simulation kinematics.  This means that the models will accurately capture large rotations
but only in the limit of zero time integration error.  In theory then, the rotational kinematics are only correct for infinitesimal
time steps.

!media tensor_mechanics/rotatecube.gif
       id=rotate
       style=width:50%;float:center;padding-top:1.5%;
       caption=Simulation of stretch plus rotation for an elastic material.

The animation in [rotate] illustrates one simple example: a block of material is stretched, developing some stress, and then rotated $90^\circ$.
A correct simulation of this process would first develop stress in the $z$-direction and then keep the magnitude of the stress constant as the
block of material rotates.
[rotation] shows the $zz$ and $yy$ components of the Cauchy stress, as integrated for an elastic material with the Truesdell rate, during this
process for different numbers of integration time steps during the rotational part of the deformation.  For large numbers of time
steps the simulation results are correct: the $yy$ component of the Cauchy stress at the end of the simulation is equal to the initial $xx$ stress
and the $zz$ component goes to zero as the block rotates.  But for fewer steps the rotational process is not integrated exactly, leading to errors in the
final stress tensor.

!media tensor_mechanics/rotation.png
       id=rotation
       style=width:50%;float:center;padding-top:1.5%;
       caption=Plot of $\sigma_{yy}$ and $\sigma_{zz}$ in the cube as it rotates, for different numbers of time steps.

This $90^\circ$ rotation without additional stretch is not a typical simulation.  More often, a simulation would be deforming the material
during the large rotations, which in turn requires a smaller time step to accurately resolve the material deformation itself.  However, this
example does illustrate one of the shortcomings of this particular implementation of objective integration.
Note this is not a generic shortcoming of objective rates, other integration approaches can achieve exact rotational kinematics
regardless of the time increment, including one of the options in the base tensor mechanics kernels [!cite](rashid1993incremental).

### Large shears

Models may exhibit anomalous, unphysical behavior when subjected to large shear deformations.  [shear] compares the results of
shearing a block of material to very large shear strains using both the Truesdell and Jaumann rates.  The
shear stress/strain response for the Jaumann model oscillates, which is not a reasonable, physical response for the
elastic material.  The Truesdell rate, which is used by default by `ComputeLagrangianObjectiveStress` models, avoids
this non-physical behavior.

!media tensor_mechanics/shearcompare.png
       id=shear
       style=width:50%;float:center;padding-top:1.5%;
       caption=Shear stress/shear strain plot comparing the Truesdell and Jaumann rates for very large shear deformations.

### Recommendations

For problems where the material will undergo very large stretches or a combination of large stretch and rotation the user
should consider defining the constitutive response with a hyperelastic formulation, for example using the
[`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md) or [`ComputeLagrangianStressPK2`](ComputeLagrangianStressPK2.md)
base classes.

!bibtex bibliography

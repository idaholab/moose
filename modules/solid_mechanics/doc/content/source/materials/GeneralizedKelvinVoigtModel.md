# Generalized Kelvin-Voigt Model

!syntax description /Materials/GeneralizedKelvinVoigtModel

## Description

The `GeneralizedKelvinVoigtModel` material represents a generalized Kelvin-Voigt model, that is, a
material composed of $N$ Kelvin-Voigt units assembled in series.  The material obeys to the following
constitutive equation:
\begin{equation}
\sigma_{ij} = C_{ijkl} \left( \epsilon^{mech}_{kl} - \sum\limits_{n=1}^{N} \alpha^{n}_{kl} \right)
\end{equation}

The $\alpha^{n}$ are the internal strains associated to each Kelvin-Voigt unit and obey the following
time-dependent differential equation:
\begin{equation}
\forall n \in [1, N]: \sigma_{ij} = C^{n}_{ijkl} \left( \alpha^{n} + \eta^{n} \dot{\alpha}^{n} \right)
\end{equation}
$C^{n}$ is the stiffness of the $n^{th}$ spring in the chain (a fourth-order tensor, identical in
symmetry and dimensions to a standard elasticity tensor), while $\eta^{n}$ is the viscosity of the
associated dashpot (a scalar with the dimension of time).

## Internal Time-Stepping Scheme

The constitutive equations are solved using a semi-implicit single-step first-order finite difference
scheme. The internal strains at time step $t+\Delta t$ are computed from their values at the previous
time step $t$:
\begin{equation}
\alpha^{n}(t+\Delta t) = \alpha^{n}(t) + \Delta t \left[ \theta^n \dot{\alpha}^n (t+\Delta t) + \left( 1 - \theta^n \right) \dot{\alpha}^n (t)  \right]
\end{equation}
$\theta$ is a scalar between 0 (fully explicit) and 1 (fully implicit) that controls the
time-stepping scheme (default value: 1). The value $\theta$ is determined by the "integration_rule"
input parameter, which can take one of the forms shown in [theta_formulations_gkvm].

!table id=theta_formulations_gkvm caption=Integration Rule and Time-Stepping Scheme
| Integration Rule | Value of $\theta$  | Unconditional Convergence |
|------------------|--------------------|---------------------------|
| BackwardEuler    | 1                  | yes                       |
| MidPoint         | 0.5                | yes                       |
| Newmark          | user-defined       | $\theta \geq 0.5$         |
| Zienkiewicz      | $\frac{1}{1 - e^{-\Delta t / \eta^n}} - \eta^n / \Delta t$ | yes |

!alert warning title=Convergence
The scheme is not valid for $\theta = 0$, so this value is forbidden.

Using this formalism, the stress-strain constitutive equation, which depends on the $\alpha^{n}(t +
\Delta t)$ (unknown) can be rewritten so that it only depends on the $\alpha^{n}(t)$ and
$\dot{\alpha}^{n}(t)$ (both being known).

For efficiency reasons, the $\alpha^{n}(t)$ and $\dot{\alpha}^{n}(t)$ are not stored separately, but
as a single variable $\alpha^{n}(t) + \Delta t \theta^{n} \dot{\alpha}^{n}(t)$.

!alert note title=Update the Scheme with a Linear Viscoelastic Manager Material
For the time-stepping scheme to be properly updated, a
[LinearViscoelasticityManager](/LinearViscoelasticityManager.md) object must be included in the input
file, and linked to the material

## Stress-Strain Computation

The material is compatible with either the total small strain approximation, or either of the
incremental strain approximation (incremental small strains or finite strains). The model requires
the stress calculators listed in [stress_calculators_gkvm].

!table id=stress_calculators_gkvm caption=Consistent Strain and Stress Calculators
| Strain | Stress | Additional Materials |
|--------|--------|----------------------|
| [ComputeSmallStrain](/ComputeSmallStrain.md) | [ComputeLinearViscoelasticStress](/ComputeLinearViscoelasticStress.md) | none |
| [ComputeIncrementalStrain](/ComputeIncrementalStrain.md) | [ComputeMultipleInelasticStress](/ComputeMultipleInelasticStress.md) | [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md) |
| [ComputeFiniteStrain](/ComputeFiniteStrain.md) | [ComputeMultipleInelasticStress](/ComputeMultipleInelasticStress.md) | [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md) |

The stress calculators use the actual elasticity tensor of the material $C_{ijkl}$, which is provided
by the material itself.

### Driving Eigenstrain (Optional)

If the user defines a driving eigenstrain, then the stress induced by this eigenstrain is added to
the creep calculation. Essentially, this replaces the differential relation in each material module
with:
\begin{equation}
\forall n \in [1, N]: \sigma_{ij} + C_{ijkl} \epsilon^{driving}_{kl} = C^{n}_{ijkl} \left( \alpha^{n} + \eta^{n} \dot{\alpha}^{n} \right)
\end{equation}

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/visco/visco_finite_strain.i block=Materials/kelvin_voigt

with the required strain calculator

!listing modules/solid_mechanics/test/tests/visco/visco_finite_strain.i block=Materials/strain

the required stress calculator

!listing modules/solid_mechanics/test/tests/visco/visco_finite_strain.i block=Materials/stress

and the additional material to define the viscoelastic behavior

!listing modules/solid_mechanics/test/tests/visco/visco_finite_strain.i block=Materials/creep

and the required Linear Viscoelasticity Manager User Object:

!listing modules/solid_mechanics/test/tests/visco/visco_finite_strain.i block=UserObjects

!syntax parameters /Materials/GeneralizedKelvinVoigtModel

!syntax inputs /Materials/GeneralizedKelvinVoigtModel

!syntax children /Materials/GeneralizedKelvinVoigtModel

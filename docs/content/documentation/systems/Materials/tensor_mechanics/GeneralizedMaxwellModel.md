# GeneralizedMaxwellModel

This material represents a generalized Maxwell model, that is, a material composed of $N$ Maxwell units assembled in series.

### Constitutive Equations
The material obeys to the following constitutive equation:

$$
\sigma_{ij} = C_{ijkl} \epsilon^{mech}_{kl} - \sum\limits_{n=1}^{N} C^{n}_{ijkl} \alpha^{n}_{kl} 
$$

The $\alpha^{n}$ are the internal strains associated to each Maxwell unit, and $C^{n}$ the stiffness of the corresponding spring (a fourth-order tensor, identical in symmetry and dimensions to a standard elasticity tensor). The $\alpha^{n}$ obey the following time-dependent differential equation:

$$
\forall n \in [1, N]: \epsilon^{mech} - \alpha^{n} = \eta^{n} \dot{\alpha}^{n}
$$

With $\eta^{n}$ is the viscosity of the associated dashpot (a scalar with the dimension of time).

### Internal Time-Stepping Scheme

The constitutive equations are solved using a semi-implicit single-step first-order finite difference scheme. The internal strains at time step $t+\Delta t$ are computed from their values at the previous time step $t$:

$$
\alpha^{n}(t+\Delta t) = \alpha^{n}(t) + \Delta t \left( \theta^n \dot{\alpha}^n (t+\Delta t) + \left( 1 - \theta^n \right) \dot{alpha}^n (t)  \right)
$$

$\theta$ is a scalar between 0 (fully explicit) and 1 (fully implicit) that controls the time-stepping scheme (default value: 1). $\theta$ is determined by the "integration_rule" input parameter, which can take one of the following values:

| Rule | Value of $\theta$  | Unconditional convergence |
| BackwardEuler | 1 | yes |
| MidPoint | 0.5 | yes |
| Newmark | user-defined | $\theta \geq 0.5$ |
| Zienkiewicz | $\frac{1}{1 - e^{-\Delta t / \eta^n}} - \eta^n / \Delta t$ | yes |

!!! Convergence
    The scheme is not valid for $\theta = 0$, so this value is forbidden.

Using this formalism, the stress-strain constitutive equation, which depends on the $\alpha^{n}(t + \Delta t)$ (unknown) can be rewritten so that it only depends on the $\alpha^{n}(t)$ and $\dot{\alpha}^{n}(t)$ (both being known).

For efficiency reasons, the $\alpha^{n}(t)$ and $\dot{\alpha}^{n}(t)$ are not stored separately, but as a single variable $\alpha^{n}(t) + \Delta t \theta^{n} \dot{\alpha}^{n}(t)$.

!!! Update
    For the time-stepping scheme to be properly updated, a [LinearViscoelasticityManager](/LinearViscoelasticityManager.md) object must be included in the input file, and linked to the GeneralizedMaxwellModel material

### Stress-Strain Computation

The GeneralizedMaxwellModel is compatible with either the total small strain approximation, or either of the incremental strain approximation (incremental small strains or finite strains). It needs the following stress calculators:

| Strain | Stress | Additional Materials |
| [ComputeSmallStrain](/ComputeSmallStrain.md) | [ComputeLinearViscoelasticStress](/ComputeLinearViscoelasticStress.md) | - |
| [ComputeIncrementalSmallStrain](/ComputeIncrementalSmallStrain.md) | [ComputeMultipleInelasticStress](/ComputeMultipleInelasticStress.md) | [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md) |
| [ComputeFiniteStrain](/ComputeFiniteStrain.md) | [ComputeMultipleInelasticStress](/ComputeMultipleInelasticStress.md) | [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md) |

### Driving Eigenstrain (Optional)

If the user defines a driving eigenstrain, then the stress induced by this eigenstrain is added to the creep calculation. Essentially, this replaces the differential relation in each Kelvin-Voigt module with:

$$
\forall n \in [1, N]: \epsilon^{mech} + \epsilon^{driving} - \alpha^{n} = \eta^{n} \dot{\alpha}^{n}
$$

### Syntax

!syntax description /Materials/GeneralizedMaxwellModel

!syntax parameters /Materials/GeneralizedMaxwellModel

!syntax inputs /Materials/GeneralizedMaxwellModel

!syntax children /Materials/GeneralizedMaxwellModel

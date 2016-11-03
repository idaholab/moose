#Stresses in Tensor Mechanics

## Some Identities involving the Stress Tensor
Denote the stress tensor by $\sigma_{ij}$.  Assume it is symmetric.  A useful quantity, called the "mean stress", is
$$
\sigma_{m} = \mbox{Tr}\sigma/3 = \sigma_{ii}/3 \ .
$$
Another useful quantity is the traceless part of $\sigma$, which is called the deviatoric stress, denoted by $s_{ij}$:
$$
s_{ij} = \sigma_{ij} - \delta_{ij}\sigma_{m} = \sigma_{ij} - \delta_{ij}\sigma_{kk}/3 \ .
$$
In many calculations it is useful to use the invariants of $s$, which are
$$
\begin{array}{rcl}
J_{2} & = & \frac{1}{2}s_{ij}s_{ij} \ , \\
J_{3} & = & \mbox{det}s \ .
\end{array}
$$
Clearly $J_{2} \geq 0$, and
often the square-root of $J_{2}$ is written as:
$$
\bar{\sigma} = \sqrt{J_{2}} \ .
$$
Alternative forms for $J_{3}$ are
$$
\begin{array}{rcl}
J_{3} & = & \mbox{det}s \\
& = & \frac{1}{6}\epsilon_{ijk}\epsilon_{mnp}s_{im}s_{jn}s_{kp} \\
& = & \frac{1}{3}s_{ij}s_{jk}s_{ki}
\end{array}
$$
All scalar functions of the stress tensor can be written in terms of its invariants.  For instance,
$$
s_{ij}s_{jk}s_{kl}s_{li} = 2J_{2}^{2}
$$

## Elasticity
Elastic materials do not experience permanent deformation, and all elastic strain and elastic stress is recoverable.  Elastic stress is related to elastic strain through the elasticity tensor
$$
\sigma_{ij} = C_{ijkl} \epsilon_{kl}
$$.
The two simplified elastic stress materials in Tensor Mechanics are

1. [ComputeLinearElasticStress](/Materials/ComputeLinearElasticStress.md) for small total strain formulations, and

2. [ComputeFiniteStrainElasticStress](/Materials/ComputeFiniteStrainElasticStress.md) for incremental and finite strain formulations.

## Plasticity with User Objects
This approach to modeling plasticity problems uses as stress material to call several `UserObjects`, where each user object calculates and returns a specific materials property, e.g. a crystal plasticity lip system strength.  The stress material then calculates the current stress state based on the returned material properties.

### MultiSurface Plasticity
In MOOSE, multi-surface plasticity is implemented through the [ComputeMultiPlasticityStress](/Materials/ComputeMultiPlasticityStress.md) Material. This assumes that there is exactly one internal parameter per single-surface plasticity model, and that the functions for single-surface plasticity model depend only on its internal parameter: there must not be 2 or more internal parameters per single-surface plasticity model.)  In this case
$$
\begin{array}{rcll}
f_{\alpha} & = & f_{\alpha}(\sigma, q_{\alpha}) & \text{yield functions} \\
r^{\alpha}_{ij} & = & r^{\alpha}_{ij}(\sigma, q_{\alpha}) & \text{flow potentials} \\
h^{\alpha}_{a} & = & \delta^{\alpha}_{a}h^{\alpha}(\sigma, q_{\alpha} & \text{hardening potentials})
\end{array}
$$
where there is no sum on $\alpha$.

The Newton-Raphson procedure attempts to solve three types of equation:

1. Denoted by `f` in the code.  $f_{\alpha} = 0$ for all active $\alpha$, up to a tolerance specified by the `yield_function_tolerance` of the plastic model

2. Denoted by `epp` in the code.  $$0 = -E^{-1}_{ijkl}(\sigma_{kl}^{\mathrm{trial}} - \sigma_{kl}) + \sum_{\mathrm{active}\ \alpha}\gamma^{\alpha}r^{\alpha}_{ij}(\sigma, q)$$, up to a tolerance specified by `ep_plastic_tolerance`.  In the code there is a variable `delta_dp` that is shorthand for  $$E^{-1}_{ijkl}(\sigma_{kl}^{\mathrm{trial}} - \sigma_{kl}), \quad or \quad \dot{\epsilon}^{\mathrm{p}}_{ij}$$

3. Denoted by `ic` in the code.  $0 = q_{\alpha} - q_{\alpha}^{\mathrm{old}} + \gamma_{\alpha}h^{\alpha}_{\alpha}$, up to a tolerance specified by `internal_constraint_tolerance` of the plastic model.  There is no sum on $\alpha$ in this expression.

In addition to these constraints, the Kuhn-Tucker and consistency conditions must also be satisfied.  If the above constraints are satisfied, then these last conditions amount to: if $f_{\alpha}=0$ (up to a tolerance), then $\gamma^{\alpha}\geq 0$; otherwise $\gamma^{\alpha}=0$.

### Crystal Plasticity
The `UserObject` based crystal plasticity system is designed to facilitate the implementation of different constitutive laws in a modular way. Both **phenomenological** constitutive models and **dislocation-based** constitutive models can be implemented through this system. This system consists of one material class [FiniteStrainUObasedCP](/Materials/FiniteStrainUObasedCP.md) and four userobject classes:

* [CrystalPlasticitySlipRate](/UserObjects/CrystalPlasticitySlipRateGSS.md)
* [CrystalPlasticitySlipResistance](/UserObjects/CrystalPlasticitySlipResistanceGSS.md)
* [CrystalPlasticityStateVarRateComponent](/UserObjects/CrystalPlasticityStateVarRateComponentGSS.md)
* [CrystalPlasticityStateVariable](/UserObjects/CrystalPlasticityStateVariable.md)

### Hyperelastic Viscoplastic
The Hyperelastic Viscoplastic model is based on the multiplicative decomposition of the total deformation ($\underline{F}$) gradient into an elastic ($\underline{F}^e$) and viscoplastic ($\underline{F}^{vp}$) component. The viscoplastic component of deformation is evolved in the intermediate configuration following

$$
\dot{\underline{F}}^{vp}\underline{F}^{vp-1} = \sum_{i=1}^{N} \dot{\lambda^i}\underline{r}^i
$$

where $\dot{\lambda^i}$ and $\underline{r}^i$ are the flow rate and flow directions, respectively, and, $N$ is the number of flow rates. This representation allows different flow rates and directions to be superimposed to obtain an effective viscoplastic deformation of the material.


The integration of the model is performed using a combination of Material and DiscreteElementUserObject classes.  In the material class, [FiniteStrainHyperElasticViscoPlastic](/Materials/FiniteStrainHyperElasticViscoPlastic.md) the following residual equations are set for every flow rate. The flow rates and directions are also calculated using UserObjects. The material class declares properties associated with each of the UserObjects and calls the functions in the UserObjects to perform the update as described above. The available base classes of UserObjects are as follows:

* `HEVPFlowRateUOBase`
* `HEVPInternalVarRateUOBase`
* `HEVPInternalVarUOBase`
* `HEVPStrengthUOBase`


##Creep and Plasticity with StressUpdate Materials
In addition to the User Object based plasticity models, another set of plasticity and creep material models have been developed with the `StressUpdateBase` class, which allows for iterations within the material itself.  These iterative materials are designated by `StressUpdate` at the end of the class name. These classes use a Radial Return von Mises, or J2, approach to determine the inelastic, creep, damage, or plasticity strain increments at each time step.  The advantage of the stress update materials is the ability to combine multiple inelastic stress calculations, such as creep and plasticity. The [Stress Update Radial Return Mapping Algorithm](/Materials/RadialReturnStressUpdate.md) discusses the general algorithm to return the stress state to the yield surface.

The stress update materials are not called by MOOSE directly but instead only by other materials using the `computeProperties` method. Separating the call to the stress update materials from MOOSE allows us to iteratively call the update materials as is required to achieve convergence.

For **isotropic materials** the radial return approach offers distinct advantages:

  - **Faster simulation run times**: The isotropic material iteration algorithm uses single variable `Reals` to compute and converge the inelastic strain instead of inverting the full `Rank-4` elasticity tensor required in more complicated anisotropic algorithms.
  - **Easy to understand**: The return mapping algorithm implemented in [RadialReturnStressUpdate](Materials/RadialReturnStressUpdate.md) is the classical radial return method based on the von Mises yield criterion.
  - **Applicable to a variety of models**: The radial return method provides the flexibility to include creep, plasticity, and damage within a single simulation.  The [ComputeReturnMappingStress](/Materials/ComputeReturnMappingStress.md) class calls each individual creep and plasticity model to iterate separately over the inelastic strain increment before checking for the convergence of the combined total radial return stress increment required to return the stress state to the yield surface.


The stress update materials each individually calculate, using the [Newton Method](http://mathworld.wolfram.com/NewtonsMethod.html), the amount of effective inelastic strain required to return the stress state to the yield surface.
$$
\Delta p^{(t+1)} = \Delta p^t + d \Delta p
$$
where the change in the iterative effective inelastic strain is defined as the yield surface over the derivative of the yield surface with respect to the inelastic strain increment. In the case of isotropic linear hardening plasticity, with the hardening function $$ r = hp$$, the effective plastic strain increment has the form:
$$
 d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_{yield}}{3G + h}
$$
where G is the isotropic shear modulus, and $\sigma^{trial}_{effective}$ is the scalar von Mises trial stress.  

When more than one stress update material is included in the simulation `ComputeReturnMappingStress` will iterate over the change in the calculated stress until the return stress has reached a stable value.

# CrystalPlasticityKalidindiUpdate

!syntax description /Materials/CrystalPlasticityKalidindiUpdate

!alert warning
Under development

The evolution equations are implemented in an updated Lagrianian incremental
form, and a trial stress increment is predicted at each simulation iteration on
each of the quadrature points. Using a Newton Rhapson approach, outlined below
in [xtalpl_nr_pk2convergence] we consider the system convergence when the
residual of the second Piola-Kirchhoff stress increments from the current and
previous iterations is within tolerances specified by the user. The converged
Cauchy stress value and the corresponding strain are then calculated, before
being passed back to MOOSE for the FEM integration residual calculation.

!media media/tensor_mechanics/crystal_plasticity/crystal_plasticity_stress_update_algorithm.png
    id=xtalpl_nr_pk2convergence
    caption=The flowchart for the calculation of the stress and strain measures within the `CrystalPlasticityUpdate` class as implemented in the tensor mechanics module of MOOSE is shown here. The components involved in the Newton-Rhapson iteration are shown in light blue and the components shown in light orange are executed once per MOOSE iteration.
    style=display:block;margin-left:auto;margin-right:auto;width:70%

This Cauchy stress residual calculation is based on the stress divergence
theorem, and MOOSE reduces the residual from the left-hand side of this equation
to the specified tolerance. The Newton-Rhapson iteration algorithm implemented
in MOOSE separates the iteration over the second Piola-Kirchhoff stress residual
from the physically based constitutive models used to calculate the plastic
velocity gradient, [xtalpl_constitutive_convergence]. The convergence algorithm
for Newton-Rhapson iteration is taken from other approaches already implemented
in MOOSE and is adapted for our crystal plasticity framework.

!media media/tensor_mechanics/crystal_plasticity/crystal_plasticity_stress_update_constitutive_convergence.png
    id=xtalpl_constitutive_convergence
    caption=The algorithm components of the Newton-Rhapson iteration are shown in light blue and the crystal plasticity constitutive model components are shown in light green..
    style=display:block;margin-left:auto;margin-right:auto;width:70%


### Constitutive Model Definition

Constitutive models are used to calculate the plastic slip rate.
In this crystal plasticity material the slip rate is modeled as a power law:
\begin{equation}
  \label{eqn:powerLawSlipRate}
  \dot{\gamma}^{\alpha} = \dot{\gamma}_o \left| \frac{\tau^{\alpha}}{g^{\alpha}} \right|^{1/m} sign \left( \tau^{\alpha} \right)
\end{equation}
where $\dot{\gamma}_o$ is a reference slip rate, $\tau^{\alpha}$ is the applied
shear stress on each slip system $\alpha$, $g^{\alpha}$ is the slip system
strength, or resistance to slip, and $m$ is the strain rate sensitivity
exponent. The strength of each slip system is solved with an iterative process
as a function of the slip increment
\begin{equation}
  \label{eqn:slipStrengthEvolution}
  g^{\alpha} =  g_o + \Delta \gamma^{\alpha} q^{\alpha \beta} h _o \left| 1 - \frac{g^{\alpha}}{g_{sat}}  \right|^a sign \left( 1 - \frac{g^{\alpha}}{g_{sat}} \right)
\end{equation}
where $q^{\alpha \beta}$ is a hardening coefficient matrix that accounts for the
different in self and latent hardening, [eqn:selfLatentQMatrix], $h_o$
is an initial hardening term, $g_{sat}$ is a constant saturated hardening value,
and $a$ is the hardening exponent [!citep](kalidindi1992). The self and latent
hardening of the crystal is defined for an FCC system as
\begin{equation}
  \label{eqn:selfLatentQMatrix}
  q^{\alpha \beta} = \begin{Bmatrix}
                       1.0 & q   & q   & q  \\
                       q   & 1.0 & q   & q  \\
                       q   & q   & 1.0 & q  \\
                       q   & q   & q   & 1.0
                     \end{Bmatrix}
\end{equation}
where $q$ is a constant value of latent hardening among non-coplanar slip
systems. In [eqn:selfLatentQMatrix] the slip systems which share the
same slip plane normal (e.g. $[\bar{1}11]$) are coplanar and grouped together
with a latent hardening rate of unity [!citep](kalidindi1992). Each matrix entry in
[eqn:selfLatentQMatrix] represents the interaction among two different
coplanar slip system groups, that is a total of six slip systems
[!citep](kalidindi1992).


## Units Assumed in the Crystal Plasticity Materials

The simulation domain for crystal plasticity models is resolved on the order of
individual crystal grains, and, as such, the mesh size is small. Although MOOSE
itself is dimension agnostic, the crystal plasticity models are implemented in
the +mm-MPa-s unit system+. This dimension system choice impacts the
input files in the following manner:

- Mesh dimensions should be constructed in mm
- Elastic constant values (e.g. Young's modulus and shear modulus) are entered in MPa
- Initial slip system strength values are entered in MPa
- Simulation times are given in s
- Strain rates and displacement loading rates are given in 1/s and mm/s, respectively

In physically based models, which maybe based on this class, initial densities
of crystal defects (e.g. dislocations, point defects) should be given in
1/mm$^2$ or 1/mm$^3$

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/trial_xtalpl

`CrystalPlasticityKalidindiUpdate` must be run in conjunction with the crystal
plasticity specific  stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/stress

!syntax parameters /Materials/CrystalPlasticityKalidindiUpdate

!syntax inputs /Materials/CrystalPlasticityKalidindiUpdate

!syntax children /Materials/CrystalPlasticityKalidindiUpdate

!bibtex bibliography

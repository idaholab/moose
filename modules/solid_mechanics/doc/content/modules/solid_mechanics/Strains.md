# Strain Formulations in Solid Mechanics

The solid mechanics module offers three different types of strain calculation:
Small linearized total strain, small linearized incremental strain, and finite incremental strain.

## Small Linearized Total Strain

For linear elasticity problems, the Solid Mechanics module includes a small strain
and total strain material [ComputeSmallStrain](/ComputeSmallStrain.md).  This material
is useful for verifying material models with hand calculations because of the
simplified strain calculations.

Linearized small strain theory assumes that the gradient of displacement with
respect to position is much smaller than unity, and the squared displacement
gradient term is neglected in the small strain definition to give:
\begin{equation}
\epsilon = \frac{1}{2} \left( u \nabla + \nabla u \right) \quad when \quad \frac{\partial u}{ \partial x} << 1
\end{equation}
For more details on the linearized small strain assumption and derivation, see a Continuum Mechanics text such as [!cite](malvern1969introduction) or [!cite](bower2009applied), specifically [Chapter 2](http://solidmechanics.org/Text/Chapter2_1/Chapter2_1.php#Sect2_1_7).

Total strain theories are path independent: in MOOSE, path independence means
that the total strain, from the beginning of the entire simulation, is used to
calculate stress and other material properties.  Incremental theories, on the other
hand, use the increment of strain at timestep to calculate stress.  Because the
total strain formulation `ComputeSmallStrain` is path independent, no old values
of strain or stress from the previous timestep are stored in MOOSE.  For a comparison
of total strain vs incremental strain theories with experimental data,
see [!cite](shammamy1967incremental).

The input file syntax for small strain is

!listing modules/solid_mechanics/tutorials/basics/part_1.1.i block=Physics/SolidMechanics/QuasiStatic
end=stress

## Incremental Small Strains

Applicable for small linearized strains, MOOSE includes an incremental small
strain material, [ComputeIncrementalStrain](/ComputeIncrementalStrain.md).
As in the small strain material, the incremental small strain class assumes the
gradient of displacement with respect to position is much smaller than unity,
and the squared displacement gradient term is neglected in the small strain definition
to give:
\begin{equation}
\epsilon = \frac{1}{2} \left( u \nabla + \nabla u \right) \quad when \quad \frac{\partial u}{ \partial x} << 1
\end{equation}
As the class name suggests, `ComputeIncrementalStrain` is an incremental formulation.
The stress increment is calculated from the current strain increment at each time
step. In this class, the rotation tensor is defined to be the rank-2 Identity tensor:
no rotations are allowed in the model. Stateful properties, including `strain_old`
and `stress_old`, are stored. This incremental small strain material is useful as
a component of verifying more complex finite incremental strain-stress calculations.

The input file syntax for incremental small strain is

!listing modules/solid_mechanics/test/tests/thermal_expansion/constant_expansion_coeff.i block=Physics/SolidMechanics


## Finite Large Strains

For finite strains, use [ComputeFiniteStrain](/ComputeFiniteStrain.md) in which an incremental form is employed such that the strain increment and the rotation increment are calculated.

### Incremental Deformation Gradient

The finite strain mechanics approach used in the MOOSE solid_mechanics module
is the incremental corotational form from [!cite](rashid1993incremental). In this
form, the generic time increment under consideration is such that $t \in [t_n, t_{n+1}]$.
The configurations of the material element under consideration at $t = t_n$ and
$t = t_{n+1}$ are denoted by $\kappa_n$, and $\kappa_{n + 1}$, respectively. The
incremental motion over the time increment is assumed to be given in the form of
the inverse of the deformation gradient $\hat{\boldsymbol{F}}$ of
$\kappa_{n + 1}$ with respect to $\kappa_n$, which may be written as

\begin{equation}
\hat{\boldsymbol{F}}^{-1} = 1 - \frac{\partial \hat{\boldsymbol{u}}}{\partial \boldsymbol{x}},
\end{equation}
where $\hat{\boldsymbol{u}}(\boldsymbol{x})$ is the incremental displacement field for the time step, and
$\boldsymbol{x}$ is the position vector of materials points in $\kappa_{n+1}$. Note that
$\hat{\boldsymbol{F}}$ is NOT the deformation gradient, but rather the incremental deformation gradient
of $\kappa_{n+1}$ with respect to $\kappa_n$. Thus,
\begin{equation}
\hat{\boldsymbol{F}} = \boldsymbol{F}_{n+1} \boldsymbol{F}_n^{-1}
\end{equation}
where $\boldsymbol{F}_n$ is the total deformation gradient at time $t_n$.

For this form, we assume
\begin{equation}
\begin{aligned}
\dot{\boldsymbol{F}} \boldsymbol{F}^{-1} =& \boldsymbol{D}\ \mathrm{(constant\ and\ symmetric),\ } t_n<t<t_{n+1}\\
\boldsymbol{F}(t^{-}_{n+1}) =& \hat{\boldsymbol{U}}\ \mathrm{(symmetric\ positive\ definite)}\\
\boldsymbol{F}(t_{n+1}) =& \hat{\boldsymbol{R}} \hat{\boldsymbol{U}} = \hat{\boldsymbol{F}}\ (\hat{\boldsymbol{R}}\ \mathrm{proper\ orthogonal})
\end{aligned}
\end{equation}

In solid mechanics, there are two decomposition options to obtain the strain increment:
TaylorExpansion and EigenSolution, with the default set to TaylorExpansion. See the
[ComputeFiniteStrain](/ComputeFiniteStrain.md) for a description of both decomposition options.

### Large Strain Closed Loop Loading Cycle id=large_strain_closed_loop_loading_cycle

It is important to note that some inaccuracies are inherently present in
incremental (hypoelastic) representations of finite strain behavior. This is
an issue for the MOOSE implementation discussed here, as well as the
implementations of other major commercial codes. The magnitude of the error
increases as the magnitude of the strain increases, so this is typically only
a practical issue for very large strains, for which hyperelastic models are
more appropriate. This can be demonstrated with the closed loop loading cycle
illustrated in [closed_loop_cycle_loading]. Since the initial and final
configurations are the same, with elastic material, one would expect that the
stress would return to zero in the final undeformed state. With this
incremental formulation, however, a residual stress remains, and the magnitude
of that residual stress increases with the magnitude of the loading cycle.
This occurs because the integral of the rate-of-deformation over the full
loading cycle is not zero. A detailed explanation can be found in
[!cite](belytschko2003).

!media solid_mechanics/closed_loop_large_deform_cycle_loading.png
       id=closed_loop_cycle_loading
       style=width:95%;float:right;padding-top:1.5%;
       caption=Closed loop large deformation loading cycle.

### Volumetric Locking Correction

In [ComputeFiniteStrain](/ComputeFiniteStrain.md) $\hat{\boldsymbol{F}}$ is calculated and can optionally include a volumetric locking correction following the B-bar method:
\begin{equation}
\hat{\boldsymbol{F}}_{corr} = \hat{\boldsymbol{F}} \left( \frac{|\mathrm{av}_{el}(\hat{\boldsymbol{F}})|}{|\hat{\boldsymbol{F}}|} \right)^{\frac{1}{3}},
\end{equation}
where $\mathrm{av}_{el}()$ is the average value for the entire element.

Once the strain increment is calculated, it is added to the total strain from $t_n$. The total strain from $t_{n+1}$ must then be rotated using the rotation increment.

The input file syntax for a finite incremental strain material is

!listing modules/solid_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Physics/SolidMechanics

!bibtex bibliography

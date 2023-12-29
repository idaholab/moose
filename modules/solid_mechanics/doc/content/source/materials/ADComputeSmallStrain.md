# ADComputeSmallStrain

!syntax description /Materials/ADComputeSmallStrain

## Description

The material `ADComputeSmallStrain` is designed for linear elasticity problems,
which calculates the small, total strain. This material is useful for verifying
material models with hand calculations because of the simplified strain
calculations. This material supplies material properties with all derivatives
required to form an exact Jacobian.

Linearized small strain theory assumes that the gradient of displacement with
respect to position is much smaller than unity, and the squared displacement
gradient term is neglected in the small strain definition to give:

\begin{equation}
\epsilon = \frac{1}{2} \left( u \nabla + \nabla u \right) \quad when \quad \frac{\partial u}{ \partial x} << 1
\end{equation}

For more details on the linearized small strain assumption and derivation, see a
Continuum Mechanics text such as [!cite](malvern1969introduction) or
[!cite](bower2009applied), specifically
[Chapter 2](http://solidmechanics.org/Text/Chapter2_1/Chapter2_1.php#Sect2_1_7).

Total strain theories are path independent: in MOOSE, path independence means
that the total strain, from the beginning of the entire simulation, is used to
calculate stress and other material properties.  Incremental theories, on the
other hand, use the increment of strain at timestep to calculate stress.
Because the total strain formulation `ADComputeSmallStrain` is path independent,
no old values of strain or stress from the previous timestep are stored in
MOOSE. For a comparison of total strain vs incremental strain theories with
experimental data, see [!cite](shammamy1967incremental).

!syntax parameters /Materials/ADComputeSmallStrain

!syntax inputs /Materials/ADComputeSmallStrain

!syntax children /Materials/ADComputeSmallStrain

!bibtex bibliography

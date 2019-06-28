# Compute Small Strain

!syntax description /Materials/ComputeSmallStrain

## Description

The material `ComputeSmallStrain` is designed for linear elasticity problems, which calculates the
small, total strain.  This material is useful for verifying material models with hand calculations
because of the simplified strain calculations.

Linearized small strain theory assumes that the gradient of displacement with respect to position is
much smaller than unity, and the squared displacement gradient term is neglected in the small strain
definition to give:
\begin{equation}
\epsilon = \frac{1}{2} \left( u \nabla + \nabla u \right) \quad when \quad \frac{\partial u}{ \partial x} << 1
\end{equation}
For more details on the linearized small strain assumption and derivation, see a Continuum Mechanics text such as [!cite](malvern1969introduction) or [!cite](bower2009applied), specifically [Chapter 2](http://solidmechanics.org/Text/Chapter2_1/Chapter2_1.php#Sect2_1_7).

Total strain theories are path independent: in MOOSE, path independence means that the total strain,
from the beginning of the entire simulation, is used to calculate stress and other material
properties.  Incremental theories, on the other hand, use the increment of strain at timestep to
calculate stress.  Because the total strain formulation `ComputeSmallStrain` is path independent, no
old values of strain or stress from the previous timestep are stored in MOOSE.
For a comparison of total strain vs incremental strain theories with experimental data, see [!cite](shammamy1967incremental).

## Example Input File Syntax

The small strain calculator can be activated in the input file through the use of the TensorMechanics
Master Action, as shown below.

!listing modules/tensor_mechanics/tutorials/basics/part_1.1.i block=Modules/TensorMechanics/Master

!alert note title=Use of the Tensor Mechanics Master Action Recommended
The [TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) is designed to
automatically determine and set the strain and stress divergence parameters correctly for the
selected strain formulation.  We recommend that users employ the
[TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) whenever possible
to ensure consistency between the test function gradients and the strain formulation selected.

Although not recommended, it is possible to directly use the `ComputeSmallStrain` material in an
input file.

!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch_quadratic.i
         block=Materials/strain

!syntax parameters /Materials/ComputeSmallStrain

!syntax inputs /Materials/ComputeSmallStrain

!syntax children /Materials/ComputeSmallStrain

!bibtex bibliography

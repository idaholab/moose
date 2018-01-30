# Compute Eigenstrain
!syntax description /Materials/ComputeEigenstrain

## Description
The class `ComputeEigenstrain` allows the user to specify a constant value of an eigenstrain for a simulation.
The eigenstrain is added to the mechanical strain, which can be elastic or inelastic, before computing the corresponding stress measure:
\begin{equation}
  \epsilon_{ij}^{total} = \epsilon_{ij}^{mechanical} + \epsilon_{ij}^{eigenstrain}
\end{equation}

Eigenstrain is the term given to a strain which does not result directly from an applied force.
[Chapter 3](http://onlinelibrary.wiley.com/doi/10.1002/9780470117835.ch3/pdf) of **Fundamentals of Micromechanics of Solids** describes the relationship between total, elastic, and eigen- strains and provides examples using thermal expansion and dislocations.
Eigenstrains are also referred to as residual strains, stress-free strains, or intrinsic strains; translated from German, [Eigen](http://dict.tu-chemnitz.de/deutsch-englisch/Eigen....html) means own or intrinsic in English.
The term eigenstrain was introduced by [T. Mura in 1982](http://link.springer.com/chapter/10.1007/978-94-011-9306-1_1).

Based on the number and values of constants provided as the argument to the `eigen_base` parameter, `ComputeEigenstrain` will build an isotropic, symmetric, or skew-symmetric Rank-2 eigenstrain tensor.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/visco/gen_kv_driving.i block=Materials/eigen

!syntax parameters /Materials/ComputeEigenstrain

!syntax inputs /Materials/ComputeEigenstrain

!syntax children /Materials/ComputeEigenstrain

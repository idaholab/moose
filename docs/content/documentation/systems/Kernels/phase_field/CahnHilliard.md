# CahnHilliard
!syntax description /Kernels/CahnHilliard

Implements the term

$$
\nabla M\nabla \frac{\delta F}{\delta c} = \nabla M\nabla \frac{\partial f}{\partial c}
$$

$F$ is the free energy functional of the system that is defined as $F=\int f(c) d\Omega$.

$c$ is the variable the kernel is acting on, $M$ (`mob_name`) is a scalar (isotropic)
mobility, and $f$ (`f_name`) is a free energy density
provided by the [function material](../../introduction/FunctionMaterials).

Note that this makes the assumption that $F$ is _not_ depending on $\nabla c$. The $\nabla c$ dependent terms
that arise from the gradient interface energy are handled separately in the [`CHInterface`](/CHInterface.md) kernel.

!syntax parameters /Kernels/CahnHilliard

!syntax inputs /Kernels/CahnHilliard

!syntax children /Kernels/CahnHilliard

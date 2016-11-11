
# CahnHilliard
!description /Kernels/CahnHilliard

Implements the term

$$
\nabla M\nabla \frac{\delta F}{\delta c} = \nabla M\nabla \frac{\partial f}{\partial c}
$$

$F$ is the free energy functional of the system that is defined as $F=\int f(c) d\Omega$.

$\eta$ is the variable the kernel is acting on and $f$ is a free energy density
provided by the [function material](../../introduction/FunctionMaterials) specified in `f_name`.

Note that this makes the assumption that $F$ is _not_ depending on $\nabla c$. The $\nabla c$ dependent terms
that arise from the gradient interface energy are handled separately in the [`CHInterface`](/Kernels/CHInterface.md) kernel.

!parameters /Kernels/CahnHilliard

!inputfiles /Kernels/CahnHilliard

!childobjects /Kernels/CahnHilliard

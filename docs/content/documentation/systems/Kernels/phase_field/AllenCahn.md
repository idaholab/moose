
# AllenCahn
!description /Kernels/AllenCahn

Implements the term

$$
L\frac{\delta F}{\delta\eta} = L\frac{\partial f}{\partial\eta}
$$

$F$ is the free energy functional of the system that is defined as $F=\int f(\eta) d\Omega$.

$\eta$ is the variable the kernel is acting on and $f$ is a free energy density
provided by the [function material](../../introduction/FunctionMaterials) specified in `f_name`.

Note that this makes the assumption that $F$ is _not_ depending on $\nabla\eta$. The $\nabla \eta$ dependent terms
that arise from the gradient interface energy are handled separately in the [`ACInterface`](/ACInterface.md) kernel.

!parameters /Kernels/AllenCahn

!inputfiles /Kernels/AllenCahn

!childobjects /Kernels/AllenCahn

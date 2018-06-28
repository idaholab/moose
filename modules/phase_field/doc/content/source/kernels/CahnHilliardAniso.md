# CahnHilliardAniso

!syntax description /Kernels/CahnHilliardAniso

This is anisotropic version of [`CahnHilliard`](/CahnHilliard.md) and expects a tensor valued mobility $M$.

Note that this kernel implements only the component of the free energy functional $F$ that is
*not* depending on $\nabla c$. The $\nabla c$ dependent terms that arise from the gradient
interface energy are handled separately in the [`CHInterface`](/CHInterface.md) kernel.

!syntax parameters /Kernels/CahnHilliardAniso

!syntax inputs /Kernels/CahnHilliardAniso

!syntax children /Kernels/CahnHilliardAniso

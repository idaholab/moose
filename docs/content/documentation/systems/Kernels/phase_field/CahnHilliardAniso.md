
# CahnHilliardAniso
!description /Kernels/CahnHilliardAniso

This is anisotropic version of [`CahnHilliard`](/CahnHilliard.md), which expects a tensor valued mobility $M$.

Note that this makes the assumption that $F$ is _not_ depending on $\nabla c$. The $\nabla c$ dependent terms
that arise from the gradient interface energy are handled separately in the [`CHInterfaceAniso`](/CHInterfaceAniso.md) kernel.

!parameters /Kernels/CahnHilliardAniso

!inputfiles /Kernels/CahnHilliardAniso

!childobjects /Kernels/CahnHilliardAniso

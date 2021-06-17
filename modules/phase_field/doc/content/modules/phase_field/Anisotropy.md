# Anisotropy

Objects for modeling anisotropic behavior.

## Kernels

- [`CahnHilliardAniso`](/CahnHilliardAniso.md): Bulk Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy and a tensor (anisotropic) mobility

- [`CHInterfaceAniso`](/CHInterfaceAniso.md): Gradient energy Cahn-Hilliard Kernel with a tensor (anisotropic) mobility

- [`MatAnisoDiffusion`](/MatAnisoDiffusion.md): Diffusion equation Kernel that takes an anisotropic Diffusivity from a material property

- [`SplitCHWResAniso`](/SplitCHWResAniso.md): Split formulation Cahn-Hilliard Kernel for the chemical potential variable with a tensor (anisotropic) mobility

## Materials

- [`ConstantAnisotropicMobility`](/ConstantAnisotropicMobility.md): Provide a constant mobility tensor value

- [`CompositeMobilityTensor`](/CompositeMobilityTensor.md): Assemble a mobility tensor from multiple tensor contributions weighted by material properties

- [`GBAnisotropy`](/GBAnisotropy.md): Specifying grain boundary energy and mobility parameters for individual grain boundaries as defined by order parameter pairs


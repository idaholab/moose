# FVDiffusionInterface

!syntax description /FVInterfaceKernels/FVDiffusionInterface

The diffusive flux is obtained from a two point gradient, and the diffusivity is
interpolated to the interface.

## Example input file syntax

In this example, two diffusion problems with a source terms are solved on each side
of the interface, and heat is exchanged by diffusion at the interface using the average
of the volumetric diffusivities as the interface diffusion coefficient.

!listing test/tests/fviks/diffusion/test.i block=FVInterfaceKernels/interface

!syntax parameters /FVInterfaceKernels/FVDiffusionInterface

!syntax inputs /FVInterfaceKernels/FVDiffusionInterface

!syntax children /FVInterfaceKernels/FVDiffusionInterface

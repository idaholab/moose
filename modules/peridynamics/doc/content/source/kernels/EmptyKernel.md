# Empty Kernel

## Description

Kernel `EmptyKernel` can be applied to phantom elements used to generate sidesets from nodesets in peridynamics analysis. This kernel is needed since MOOSE requires at least a kernel and a material object for each mesh block. Arbitrary material class can be used for the phantom element mesh block, this `EmptyKernel` object will contribute zero residual and Jacobian to the nodes connected by phantom elements. The actual residual and Jacobian are handled by peridynamics kernels.

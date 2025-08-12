# AbaqusForceBC

!syntax description /NodalKernels/AbaqusForceBC

# Description

This NodalKernel acts as a component of the Abaqus nodal (essential) boundary condition system. It applies an additional force to the node that corresponds to the reaction force of a previously active and now deactivated Dirichlet type boundary condition. This object needs to be a [NodalKernel](NodalKernel.md) as it _adds_ a force at boundary nodes, rather than _prescribing_ it.

!syntax parameters /NodalKernels/AbaqusForceBC

!syntax inputs /NodalKernels/AbaqusForceBC

!syntax children /NodalKernels/AbaqusForceBC

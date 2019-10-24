# CZM InterfaceKernel
!syntax description /InterfaceKernels/CZMInterfaceKernel

## Description

This class implements traction equilibrium across an interface. Specifically this is a small deformation implementation (geometrical quantities, such as surface normal, are always referred to the initial configuration). This kernel acts only on one displacement component and therefore one must setup N kernel one for each dimension of the mesh.
The `CZMInterfaceKernel` use the traction computed from `CZMMaterial` to compute the displacement residual (Nodal Force).
The responsibility of the `CZMInterfaceKernel` is to integrate the traction over the interface to compute the Nodal Force and to assemble the proper Jacobian.
It is responsibility of the `CZMMaterial` to calculate and store traction and derivatives.

## examples

!listing modules/tensor_mechanics/test/tests/CZM/czm_3DC_load_complex.i block=InterfaceKernels/interface_x


!syntax parameters /InterfaceKernels/CZMInterfaceKernel

!syntax inputs /InterfaceKernels/CZMInterfaceKernel

!syntax children /InterfaceKernels/CZMInterfaceKernel

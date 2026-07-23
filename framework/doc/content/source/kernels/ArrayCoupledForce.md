# ArrayCoupledForce

!syntax description /Kernels/ArrayCoupledForce

`ArrayCoupledForce` implements a source term for an array variable that is proportional to a
coupled standard or array variable. When `v` is an array variable, set
[!param](/Kernels/ArrayCoupledForce/is_v_array) to `true`; it must have the same number of components
as the kernel variable.

!syntax parameters /Kernels/ArrayCoupledForce

!syntax inputs /Kernels/ArrayCoupledForce

!syntax children /Kernels/ArrayCoupledForce

# ADInertialForceShell

!syntax description /Kernels/ADInertialForceShell

It accounts for inertia on both displacement (translational) and rotational degrees of freedom.
Forward mode automatic differentiation is used to compute an exact Jacobian. Please refer to [ShellElements](/ShellElements.md) for details.

Each `ADInertialForceShell` kernel calculates the force only along one coordinate direction. So, a separate `ADInertialForceShell` input block should be set up for each coordinate direction. Additionally, separate
`ADInertialForceShell` kernels should be specified for the rotation variables.

!syntax parameters /Kernels/ADInertialForceShell

!syntax inputs /Kernels/ADInertialForceShell

!syntax children /Kernels/ADInertialForceShell
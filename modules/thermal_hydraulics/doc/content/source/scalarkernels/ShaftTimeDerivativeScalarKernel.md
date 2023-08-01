# ShaftTimeDerivativeScalarKernel

!syntax description /ScalarKernels/ShaftTimeDerivativeScalarKernel

The momentum equation time derivative term is expressed in terms of the sum of the moment of inertia of the
connected shafts $L_{sum}$ :

!equation
L_{sum} \dfrac{du}{dt}

!alert note
The time integration scheme will be shared with the other non-linear variables,
as specified in the [TimeIntegrator](syntax/Executioner/TimeIntegrator/index.md).
To use a different time integrating scheme, this scalar kernel should be replaced with a custom implementation.

!alert note
In THM, most kernels are added automatically by components. This scalar kernel is created by the
[Shaft.md] component.

!syntax parameters /ScalarKernels/ShaftTimeDerivativeScalarKernel

!syntax inputs /ScalarKernels/ShaftTimeDerivativeScalarKernel

!syntax children /ScalarKernels/ShaftTimeDerivativeScalarKernel

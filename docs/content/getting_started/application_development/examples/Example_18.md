# Coupling ODE into PDE

## Complete Source Files
---

- [ex18.i](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/ex18.i)
- [ImplicitODEx.h](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/include/scalarkernels/ImplicitODEx.h)
- [ImplicitODEx.C](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/src/scalarkernels/ImplicitODEx.C)
- [ImplicitODEy.h](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/include/scalarkernels/ImplicitODEy.h)
- [ImplicitODEy.C](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/src/scalarkernels/ImplicitODEy.C)
- [ScalarDirichletBC.h](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/include/bcs/ScalarDirichletBC.h)
- [ScalarDirichletBC.C](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/src/bcs/ScalarDirichletBC.C)
- [ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex18_scalar_kernel/src/base/ExampleApp.C)

## Results
---

!media media/examples/ex18-diffused.png width=30% float=left caption=Diffused

!media media/examples/ex18-ode-plot.png width=30% float=left margin-left=1% caption=ODE Plot

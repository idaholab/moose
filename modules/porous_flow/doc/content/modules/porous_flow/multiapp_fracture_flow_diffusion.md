# Fracture flow using a MultiApp approach: Diffusion in mixed dimensions

## Background

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This page is part of a set that describes a MOOSE MultiApp approach to simulating such models:

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)

## The diffusion equation with a mixed-dimensional problem

In this section, the diffusion equation is used to explore a mixed-dimensional problem, where the fracture is a 1D line "living inside" the 2D matrix.  In reality, the fracture has a certain aperture, $a$, but it is so small that it may be approximated by a 1D line in MOOSE and its kernels multiplied by $a$: see [mathematics and physical interpretation](multiapp_fracture_flow_equations.md).

## Geometry and mesh

Two cases are explored: "conforming" and "nonconforming".   In the conforming case, all fracture nodes are also matrix nodes: the fracture elements are actually created from a sideset of the 2D matrix elements.  The conforming case is shown in [fracture_diffusion_conforming_geometry] and [fracture_diffusion_conforming_mesh]: the solution domain consists of the `fracture` subdomain (1D red line) and the `matrix` subdomain (in blue), which share nodes.  In the nonconforming case, no fracture nodes coincide with matrix nodes.  The nonconforming case is shown in [fracture_diffusion_nonconforming_mesh].

!media porous_flow/examples/multiapp_flow/fracture_diffusion_conforming_geometry.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=fracture_diffusion_conforming_geometry
	caption=The geometry of the conforming case, where the red line is the fracture

!media porous_flow/examples/multiapp_flow/fracture_diffusion_conforming_mesh.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=fracture_diffusion_conforming_mesh
	caption=The matrix mesh in the conforming case: the fracture lies exactly on the matrix nodes

!media porous_flow/examples/multiapp_flow/fracture_diffusion_nonconforming_mesh.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=fracture_diffusion_nonconforming_mesh
	caption=The matrix mesh in the nonconforming case, where the red line is the fracture

The conforming case is explored using a non-MultiApp approach and a MultiApp approach, while the nonconforming case can only be explored using a MultiApp approach.

In all cases, the finite-element mesh dictates the spatial resolution of the numerical solution, and the analysis that follows ignores this by using the same spatial resolution in each model.  However, it is important to remember that in practice, the use of finite elements means the solution is never "exact".  For instance, using large matrix elements will probably lead to poor results.  Further discussion may be found in the [small fracture network](multiapp_fracture_flow_PorousFlow_3D.md) example.  Large elements also produce more noticable overshoots and undershoots in the solution, which may be observed in the current models if the matrix `ny` is too small.

## Physics

As described in the [mathematical background](multiapp_fracture_flow_equations.md), the physics is governed by

\begin{equation}
\begin{aligned}
\label{eqn.coupled.basic}
0 &=  c_{m}\dot{T}_{m} -  \nabla(\lambda_{m}\nabla T_{m}) +  h(T_{m} - T_{f})\delta(f) \ , \\
0 &= ac_{f}\dot{T}_{f} - a\tilde{\nabla}(\lambda_{f}\tilde{\nabla} T_{f}) + h(T_{f} - T_{m}) \ .
\end{aligned}
\end{equation}

The two variables, $T_{f}$ and $T_{m}$, are called `frac_T` and `matrix_T` in the MOOSE input files.  Each obeys a diffusion equation, with heat transfer between the two variables, as written in the [eqn.coupled.basic].  The heat-transfer coefficient, $h$, may be written

\begin{equation}
\label{eqn.suggested.h}
h = \frac{h_{\mathrm{s}}\lambda_{\mathrm{m}}^{nn} (L_{\mathrm{right}} + L_{\mathrm{left}})}{h_{\mathrm{s}}L_{\mathrm{right}}L_{\mathrm{left}} + \lambda_{\mathrm{m}}^{nn}(L_{\mathrm{right}} + L_{\mathrm{left}})} \ ,
\end{equation}

When using a MultiApp, the fracture appears as a set of Dirac sources in the matrix subdomain:

\begin{equation}
0 = c_{m}\dot{T}_{m} - \nabla(\lambda_{m}\nabla T_{m}) - H\delta(f) \ ,
\end{equation}

where

\begin{equation}
H = h(T_{f} - T_{m}) \ .
\end{equation}

In the MultiApp approach, $H$ is generated as an `AuxVariable` by the `fracture` App, so exists only in the `fracture` subdomain.  It is then passed to the `matrix` App, and applied as a `DiracKernel`.

The boundary conditions are "no flow", except for the very left-hand side of the fracture domain, where temperature is fixed at $T_{f} = 1$.  The initial conditions are $T_{m} = 0 = T_{f}$.

Each of the heat capacities are unity, $c_{m} = 1 = c_{f}$, the conductivity in the fracture is $\lambda_{f} = 1.0$, and is $\lambda_{m} = 10^{-3}$ in the matrix.  The fracure has aperture $a=10^{-2}$.

Assume that $h_{s} = 0.02$.  This is the heat-transfer coefficient to use in the conforming case, since the matrix nodes align exactly with the fracture.  [eqn.suggested.h] may be used for the nonconforming case.  In this situation $L_{\mathrm{left}} = 0.1 = L_{\mathrm{right}}$ and $\lambda_{m}^{nn} = 10^{-3}$, so $h = 0.01$.

Each simulation runs with `end_time = 100`.


## No MultiApp: the benchmark

In the conforming case, a MultiApp approach need not be taken, and the Kernels (given by [eqn.coupled.basic]) are:

!listing fracture_diffusion/no_multiapp.i block=Kernels

Evaluating the `fromFracture` heat transfer Kernel only on `block = fracture` implements the Dirac delta function $\delta(f)$ in [eqn.coupled.basic].  The matrix temperature is shown in [no_multiapp_matrix_T]

!media porous_flow/examples/multiapp_flow/no_multiapp_matrix_T.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=no_multiapp_matrix_T
	caption=Benchmark matrix temperature at the end of simulation

The solution of this problem is [exactly solvable](https://github.com/idaholab/moose/blob/master/modules/misc/doc/fracture_flow) in terms of error functions, and by choosing appropriate time-step and element sizes, MOOSE produces the expected result.  Some examples of how MOOSE's solution depends upon time-step size are shown in [frac_no_multiapp_frac_T].  Evidently, reducing the time-step below 10.0 does not impact the solution very much.  Hence, the solution using $\Delta t = 0.125$ is used as the *benchmark* for the remainder of this section.

!media porous_flow/examples/multiapp_flow/frac_no_multiapp_frac_T.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=frac_no_multiapp_frac_T
	caption=Impact of time-step size on the fracture temperature at the end of the simulation

## A MultiApp approach for the conforming case

In this case, the matrix App is the main App, and the fracture App is the subApp.  The fracture input file has the following features.

- An `AuxVariable` called `transferred_matrix_T` that is $T_{m}$ interpolated to the fracture mesh.

- An `AuxVariable` called `joules_per_s` that is the heat rate coming from each node.  Mathematically this is $h(T_{f} - T_{m})L$, where $L$ is the "volume" modelled by the fracture node.  This is populated by the `save_in` feature:

!listing fracture_diffusion/fracture_app_dirac.i block=Kernels

- A `NodalValueSampler` `VectorPostprocessor` that captures all the `joules_per_s` values at each fracture node

!listing fracture_diffusion/fracture_app_dirac.i block=VectorPostprocessors

The matrix input file has the following features

- Transfers that send $T_{m}$ to the fracture App, and receive the `joules_per_s` from the fracture App

!listing fracture_diffusion/matrix_app_dirac.i block=Transfers

- This latter `Transfer` writes its information into a `VectorPostprocessor` in the matrix App.  That is then converted to a Dirac source by a `ReporterPointSource` `DiracKernel`:

!listing fracture_diffusion/matrix_app_dirac.i block=DiracKernels

## A MultiApp approach for the nonconforming case

This is identical to the conforming case except:

- the matrix mesh is nonconforming, as shown above
- the heat transfer coefficient is different, as described above

## Results

The L2 error of the fracture temperature in each approach (square-root of the sum of squares of differences between the $T_{f}$ and the benchmark $\Delta t = 0.125$ result) is plotted in [frac_l2_error].  As expected, the error is proportional to $\Delta t$.  The error when using the MultiApp approaches is larger than the non-MultiApp approach, because $T_{f}$ is fixed when $T_{m}$ is being solved for, and vice versa.  The conforming and nonconforming cases produce similar results for larger time-steps, but as the time-step size reduces the results are slightly different, just because of the small differences in the effective finite-element discretisation.

!media porous_flow/examples/multiapp_flow/frac_l2_error.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=frac_l2_error
	caption=L2 error of each approach (with respect to the benchmark where $\Delta t$ is 0.125) to modelling the mixed-dimensional diffusion equation

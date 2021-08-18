# Fracture flow using a MultiApp approach: Primer

## Background

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This page is part of a set that describes a MOOSE MultiApp approach to simulating such models:

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)


## Primer using the diffusion equation with no fractures

Before considering porous flow in a mixed-dimensional fracture-matrix system, consider the simpler situation involving two coupled diffusion equations in 1D.  Hence there is *no* "fracture" and "matrix" in this section: the labels "f" and "m" simply distinguish two different variables, and the dimensionality is all one dimensional.  Assume physical parameters have been chosen appropriately so that (see the [page desribing mathematics and physics of heat transfer](multiapp_fracture_flow_equations.md))

\begin{equation}
\begin{aligned}
0 &= \dot{T}_{f} - \nabla^{2}T_{f} + h(T_{f} - T_{m}) \ , \\
0 &= \dot{T}_{m} - \nabla^{2}T_{m} + h(T_{m} - T_{f}) \ .
\end{aligned}
\end{equation}

It is important that the same numerical value of $h$ is used in both formulae, otherwise heat energy would not be conserved in this system.

In this section, assume the boundary conditions are
\begin{equation}
T_{f}(x = \pm\infty) = 0 = T_{m}(x = \pm\infty)
\end{equation}
and the initial conditions are
\begin{equation}
\begin{aligned}
T_{f}(t = 0) &= \delta(x) \ , \\
T_{m}(t = 0) &= 0 \ ,
\end{aligned}
\end{equation}
where $\delta$ is the Dirac delta functions.  These conditions make the analytic solution easy to derive.

Physically, this system represents the situation in which the $T_{f}$ system is initially provided with a unit of heat energy at $x=0$, and that heat energy is allowed to disperse under diffusion, and transfer to the $T_{m}$ system, which also disperses it.  To derive the solution, the sum of the two governing equations yields the standard diffusion equation (which may be solved using the [fundamental solution](https://en.wikipedia.org/wiki/Heat_equation)), while the difference yields the diffusion equation augmented with a decay term.  The final result is:
\begin{equation}
\begin{aligned}
\label{eqn.analytical.pulse}
T_{f}(t, x) &= \frac{1 + e^{-2ht}}{4\sqrt{\pi t}}\exp\left(-\frac{x^{2}}{4t}\right) \ , \\
T_{m}(t, x) &= \frac{1 - e^{-2ht}}{4\sqrt{\pi t}}\exp\left(-\frac{x^{2}}{4t}\right) \ .
\end{aligned}
\end{equation}

## A single variable (no heat transfer)

When $h=0$, the system becomes decoupled and no heat will be transferred from the fracture to the matrix.  The solution is $T_{m} = 0$, and $T_{f}$ given by the fundamental solution.  This may be solved by MOOSE without any MultiApp system using the following input file

!listing diffusion_multiapp/single_var.i

The result depends on the spatial and temporal discretisation.  The temporal-discretisation dependence of $T_{f}$ is shown in [diffusion_single_var]

!media porous_flow/examples/multiapp_flow/diffusion_single_var.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=diffusion_single_var
	caption=Decoupled case fracture temperature: comparison of the analytical solution with the MOOSE results as computed using no MultiApp

## Two coupled variables (no MultiApp)

The system is coupled when $h\neq 0$.  A MultiApp approach is not strictly needed in this case, because there are no meshing problems: the domain is just the real line.  Hence, the system may be solved by MOOSE using the following input file

!listing diffusion_multiapp/two_vars.i

The result for $T_{f}$ depends on the spatial and temporal discretisation.  The temporal-discretisation dependence is shown in [diffusion_two_vars].  Notice that the matrix has removed heat from the fracture, so the temperature is decreased compared with the $h=0$ case (i.e. $T_{f}(x=0)\approx0.02$ in [diffusion_two_vars] which is lower than it was for the uncoulpled case with $h=0$ shown in [diffusion_single_var] where $T_{f}(x=0)\approx0.03$).

!media porous_flow/examples/multiapp_flow/diffusion_two_vars.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=diffusion_two_vars
	caption=Coupled case fracture temperature: comparison of the analytical solution with the MOOSE results as computed using no MultiApp

## A MultiApp approach

Using a MultiApp approach for the $h\neq 0$ case yields similar results.  The MultiApp methodology used throughout this set of pages is as follows.  One timestep involves:

1. One timestep of the fracture physics is solved, holding the matrix variables fixed.
2. Transfers from the fracture system to the matrix system are performed.
3. One timestep of the matrix system is solved, holding the fracture variables fixed.
4. Transfers from the matrix system to the fracture system are performed.

Upon reflection, the reader will realise there are many potential ways of actually implementing this.  In the case at hand, the fracture physics ($T_{f}$) is governed by the "main" App, and the matrix physics ($T_{m}$) by the "sub" App.  On the other hand, in the pages [describing diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md) and PorousFlow models in [2D](multiapp_fracture_flow_PorousFlow_2D.md) and [3D](multiapp_fracture_flow_PorousFlow_3D.md), the matrix physics is controlled by the main App and the fracture by the sub App.  This latter approach is probably advantageous in most situations because it easily allows the fracture App to take smaller time-steps.

!alert note
The MultiApp approach typically breaks the unconditional stability of MOOSE's implicit time-stepping scheme.  See the discussion at the end of this page.

### Transfer of heat energy ("heat" MultiApp)

In order to conserve heat energy, the following approach may be used

1. One timestep of the fracture App is solved, holding the $T_{m}$ fixed, using [PorousFlowHeatMassTransfer](PorousFlowHeatMassTransfer.md) Kernel to implement the $h(T_{f} - T_{m})$ term, and recording the heat lost to the matrix, $h(T_{f} - T_{m})$ into an `AuxVariable`.
2. The heat lost to the matrix is transferred to the matrix App.
3. One timestep of the matrix system is solved, using a `CoupledForce` Kernel to inject the heat gained from the fracture at each node.
4. The resulting $T_{m}$ is transferred to the fracture App.

This is implemented using the following `AuxKernel` in the fracture App:

!listing diffusion_multiapp/fracture_app_heat.i block=AuxKernels

along with the following Transfers:

!listing diffusion_multiapp/fracture_app_heat.i block=Transfers

and the `Kernel` in the matrix App:

!listing diffusion_multiapp/matrix_app_heat.i start=[fromFrac] end=[]

A couple of subtleties are that the `CoupledForce` Kernel will smooth the nodal `heat_to_matrix` AuxVariable (since it uses quad-point values) and that a `save_in` cannot be employed in the `frac_app_heat.i` input file [PorousFlowHeatMassTransfer](PorousFlowHeatMassTransfer.md) Kernel, since that would include the nodal volume.  (The inclusion of nodal volume is *exactly* what is required in the [diffusion](porous_flow/multiapp_fracture_flow_diffusion.md), [2D](porous_flow/multiapp_fracture_flow_PorousFlow_2D.md) and [3D](porous_flow/multiapp_fracture_flow_PorousFlow_3D.md) cases, since the amount of heat energy (measured in Joules) is transferred between the Apps in those cases, but in the current situation `CoupledForce` requires an energy density measured in Joules.m$^{-3}$.)  The results are shown in [fracture_app_heat].

!media porous_flow/examples/multiapp_flow/fracture_app_heat.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=fracture_app_heat
	caption=Coupled case: comparison of the analytical solution with the MOOSE results as computed using a "heat" MultiApp

### Transfer of temperature ("T" MultiApp)

An alternative approach is to transfer $T_{m}$ and $T_{f}$:

1. One timestep of the fracture App is solved, holding $T_{m}$ fixed, using [PorousFlowHeatMassTransfer](PorousFlowHeatMassTransfer.md) Kernel to implement the $h(T_{f} - T_{m})$ term.
2. The resulting $T_{f}$ is transferred to the matrix App.
3. One timestep of the matrix system is solved, holding $T_{f}$ fixed, using [PorousFlowHeatMassTransfer](PorousFlowHeatMassTransfer.md) Kernel to implement the $h(T_{f} - T_{m})$ term (using the same $h$ as the fracture App).
4. The resulting $T_{m}$ is transferred to the fracture App.

The disadvantage of this approach is that it doesn't conserve heat energy, however, the advantage is that the original differential equations are clearly evident.  Given that the error of using a MultiApp is $\Delta t$ irrespective of the type of Transfer implemented, the non-conservation of heat energy, which is also proportional to $\Delta t$, is probably not of critical importance.  This idea is implemented using the following `Kernel` in the fracture App:

!listing diffusion_multiapp/fracture_app.i start=[toMatrix] end=[]

along with the following Transfers:

!listing diffusion_multiapp/fracture_app.i block=Transfers

and the `Kernel` in the matrix App:

!listing diffusion_multiapp/matrix_app.i start=[fromFrac] end=[]

The results are shown in [fracture_app].

!media porous_flow/examples/multiapp_flow/fracture_app.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=fracture_app
	caption=Coupled case: comparison of the analytical solution with the MOOSE results as computed using a "T" MultiApp

## Error in each approach

The L2 error in each approach (square-root of the sum of squares of differences between the MOOSE result and the analytical result) is plotted in [diffusion_l2_error].  The errors are very similar for each of the models explored in this section.  The magnitude of the error is largely unimportant: the scaling with time-step size is more crucial, and in this case it follows the [expected first-order result](https://web.mit.edu/10.001/Web/Course_Notes/Differential_Equations_Notes/node3.html).

\begin{equation}
\mathrm{L2 error} \propto \mathrm{d}t \ .
\end{equation}

!media porous_flow/examples/multiapp_flow/diffusion_l2_error.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=diffusion_l2_error
	caption=L2 error of each approach as a function of time-step size

## Final remarks on stability

One aspect that is not captured in this analysis is stability.  The non-MultiApp approaches ("No heat transfer" and "Coupled, no MultiApp") use fully-implicit time-stepping, so are unconditionally stable.  Conversely, the MultiApp approaches break this unconditional stability, which could be important in PorousFlow applications.  For instance, the matrix temperature is "frozen" while the fracture App is solving.  If a very large time-step is taken before the matrix App is allowed to evolve, this would lead to huge, unphysical heat losses to the matrix system.  The fracture temperature could reduce to the matrix temperature during fracture evolution, and then the matrix temperature could rise significantly during its evolution when it receives the large quantity of heat from the fracture.  This oscillation is unlikely to become unstable, but is clearly unphysical.

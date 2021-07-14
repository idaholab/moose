# Fracture flow using a MultiApp approach: Equations

## Background

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This page is part of a set that describes a MOOSE MultiApp approach to simulating such models:

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)

## The heat equation in fracture-matrix systems

### Uncoupled equations

The 3D heat equation is

\begin{equation}
\label{eqn.3D.heat.eqn}
0 = c\frac{\partial T}{\partial t} - \nabla(\lambda \nabla T) \ .
\end{equation}
Here:

- $T$ is the temperature, with SI units K;
- $c$ is the volumetric heat capacity of the medium (which is the product of specific heat capacity and density), with SI units J.m$^{-3}$.K$^{-1}$;
- $t$ is time and $\nabla$ is the vector of spatial derivatives;
- $\lambda$ is the thermal conductivity, with SI units J.s$^{-1}$.m$^{-1}$.K$^{-1}$.

This equation (multiplied by the test functions) gets integrated over the elemental volume in the finite-element scheme.  The fracture is actually a 3D object, but it is so thin that temperature (and $c$ and $\lambda$) do not vary appreciably over its thickness (aperture).  Therefore, the integral over the thickness may be done explicitly, and the fracture modelled as a 2D object, with heat equation reading

\begin{equation}
\label{eqn.2D.heat.eqn}
0 = ac\frac{\partial T}{\partial t} - a\tilde{\nabla}(\lambda \tilde{\nabla} T) \ ,
\end{equation}

where $a$ is the fracture thickness (aperture), and $\tilde{\nabla}$ are derivatives transverse to the fracture.  This equation is integrated (by MOOSE) over the fracture-element 2D area in the finite-element method.

!alert note
The preceeding equation implies all the Kernels for heat and mass flow in the fracture system must be multiplied by the fracture aperture $a$.   That is, in the MOOSE input file governing the 2D physics, the coefficient of the [CoefTimeDerivative](CoefTimeDerivative.md) Kernel is $ac$, and the coefficient of the [AnisotropicDiffusion](AnisotropicDiffusion.md) is $a\lambda$.   If tensorial quantities such as $\lambda$ or the permeability tensor are anisotropic, they need to be expressed in 3D space.

### Coupling

In order to specify the coupling between the fracture and matrix system precisely, let us introduce some notation.  Suppose the position of the fracture is given by a level-set function, $f = f(x, y, z)$ (where $x$, $y$ and $z$ are the Cartesian coordinates): that is, the fracture is at points where $f=0$, but not at points where $f\neq 0$.  For example, if the fracture occupies the $(x, y)$ plane, then $f=z$ would be a suitable function.  The one-dimensional Dirac delta function, $\delta(f)$, has SI units m$^{-1}$, and is zero everywhere except on the fracture.  When $\delta(f)$ is integrated over a direction normal to the fracture, the result is $\int_{\mathrm{normal}}\delta(f) = 1$.  When $\delta(f)$ is integrated over a volume containing the fracture, the result is $\int_{V}\delta(f) = A$, where $A$ is the area of the fracture in the volume.

The approach explored in this page uses two temperature variables, $T_{m}$ and $T_{f}$, which are the temperature in the matrix and fracture, respectively.  $T_{m}$ is defined throughout the entire matrix (including the embedded fracture), while $T_{f}$ is defined on the fracture only.  These are assumed to obey:

\begin{equation}
\begin{aligned}
\label{eqn.coupled.basic}
0 &=  c_{m}\dot{T}_{m} -  \nabla(\lambda_{m}\nabla T_{m}) +  h(T_{m} - T_{f})\delta(f) \ , \\
0 &= ac_{f}\dot{T}_{f} - a\tilde{\nabla}(\lambda_{f}\tilde{\nabla} T_{f}) + h(T_{f} - T_{m}) \ .
\end{aligned}
\end{equation}

In these equations, $h$ is the heat-transfer coefficient between the matrix and the fracture, with SI units J.m$^{-2}$.s$^{-1}$.K$^{-1}$. If $T_{f} > T_{m}$ then the heat-transfer term takes heat energy from the $T_{f}$ system and applies it to the $T_{m}$ system.  The heat-transfer coefficient plays a central role in this page and is discussed in detail below.

When the second (fracture) equation is integrated over a (2D) portion of the fracture of area $A$, the heat-energy transferred to the matrix is $Ah\langle T_{f} - T_{m} \rangle$, where the $\langle\rangle$ indicates the average over $A$.  This is equal to the heat-energy transferred according to the first equation, when it is integrated over a volume containing the same portion of fracture (remember $\int_{V}\delta(f) = A$).  Therefore, heat energy is conserved in this system.

### The heat-transfer coefficient

Heat-transfer coefficients have been used by engineers to accurately account for the complicated processes that occur at the interface between two materials.  If the two materials have temperature $T_{a}$ and $T_{b}$, respectively, and are connected by an area $A$ then the rate of heat transfer between them is $Ah(T_{a} - T_{b})$, with SI units J.s$^{-1}$.  Heat-transfer coefficients have been estimated for many different situations (see, for instance [Wikipedia](https://en.wikipedia.org/wiki/Heat_transfer_coefficient)).

To motivate a numerical value for $h$, assume that the fracture width is tiny compared with any relevant length scales in the matrix (such as the finite-element sizes).  While the temperature distribution in the immediate vicinity of the fracture will be governed by complicated processes, the point of $h$ is to hide that complexity.  Assume there is a "skin" around the fracture in which the complicated processes occur.  Denote the temperature just outside this skin by $T_{s}$, so the heat transfer through the skin is

\begin{equation}
\label{heat.transfer.skin}
Q_{s} = h_{\mathrm{s}}(T_{f} - T_{s}) \ ,
\end{equation}
where $T_{f}$ is the fracture temperature, $h_{\mathrm{s}}$ is the heat-transfer coefficient of the skin, with SI units J.m$^{-2}$.s$^{-1}$.K$^{-1}$, and $Q_{s}$ is the heat flux, with SI units J.s$^{-1}$.m$^{-2}$.  A discussion on the numerical value of $h_{s}$ is found below.

Just as the fracture may be considered two-dimensional (implemented by including its aperture in the fracture `Kernels` and using 2D finite elements), the size of the skin is also ignored.  Thereby, the matrix "sees" the fracture as an object of $T_{s}$ (*not* $T_{f}$) sitting within it.  Consider [skin_two_nodes], where an object of temperature $T_{s}$ sits between two finite element nodes, of temperature $T_{0}$ and $T_{1}$.

!media porous_flow/examples/multiapp_flow/skin_two_nodes.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=skin_two_nodes
	caption=An object of temperature $T_{s}$ sits a distance $L_{0}$ from a finite-element node of temperature $T_{0}$, and a distance $L_{1}$ from a node of temperature $T_{1}$.

The flow from the fictitious "skin" object to each matrix finite-element nodes is governed by the heat equation.  In the linear limit, [eqn.3D.heat.eqn] implies

\begin{equation}
\begin{aligned}
\label{eqn.linear.q0q1}
Q_{0} &= \lambda \frac{T_{s} - T_{0}}{L_{0}} \ , \\
Q_{1} &= \lambda \frac{T_{s} - T_{1}}{L_{1}} \ .
\end{aligned}
\end{equation}

Assuming the sum of these equals $Q_{s}$, that is $Q_{s} = Q_{0} + Q_{1}$:
\begin{equation}
h_{\mathrm{s}}(T_{f} - T_{s}) = \lambda \left( \frac{T_{s} - T_{0}}{L_{0}} + \frac{T_{s} - T_{1}}{L_{1}}\right) \ .
\end{equation}
Using this equality allows $T_{s}$ to be written in terms of the other temperature values, and substituting the expression into [heat.transfer.skin] yields the heat-loss from the fracture:
\begin{equation}
Q = \frac{h_{\mathrm{s}}\lambda (L_{0} + L_{1})}{h_{\mathrm{s}}L_{0}L_{1} + \lambda(L_{0} + L_{1})} \left( \frac{L_{1}}{L_{0} + L_{1}}(T_{f} - T_{0}) + \frac{L_{0}}{L_{0} + L_{1}}(T_{f} - T_{1}) \right) \ .
\end{equation}
Notice that the prefactors $L_{1}/(L_{0}+L_{1})$ are exactly what a linear-lagrange element in MOOSE would prescribe to a `DiracKernel` sitting at the fracture position.  Hence, the effective heat-transfer coefficient that would be used in a MOOSE model of this situation is
\begin{equation}
h = \frac{h_{\mathrm{s}}\lambda (L_{0} + L_{1})}{h_{\mathrm{s}}L_{0}L_{1} + \lambda(L_{0} + L_{1})} \ .
\end{equation}

!alert note
Notice a key assumptions made in the presentation above --- the linear approximation of [eqn.linear.q0q1], where it is assumed that the matrix element sizes are small enough to resolve the physics of interest.  This means that phenomena associated with short-time, small-scale physics won't be resolvable in models with large elements, when using the heat-transfer coefficients recommended in this page.  This is because that in various real-life scenarios there may be *no* heat flow between the "0" position and the skin even if $T_{s}\neq T_{0}$ (contradicting [eqn.linear.q0q1] ) because, for instance, changes of $T_{s}$ take some time to be felt by the "0" position.  The solution is to use smaller elements to resolve this short-time, small-scale phenomena.  The [small fracture network model](multiapp_fracture_flow_PorousFlow_3D.md) provides an example.

The same analysis may be performed in the 2D-3D situation.  While [skin_two_nodes] is only a 1D picture, flow from the fracture to the matrix only occurs in the normal direction, so it also represents the 2D-3D situation.  With an arbitrary-shaped element containing a portion of a fracture, the heat flows to each node, and hence $h$, depend on the shape of the element.  However, following the approach of the Peaceman borehole [Peaceman borehole](/PorousFlowPeacemanBorehole.md), the following rule-of-thumb is suggested for MOOSE simulations:

\begin{equation}
\label{eqn.suggested.h}
h = \frac{h_{\mathrm{s}}\lambda_{\mathrm{m}}^{nn} (L_{\mathrm{right}} + L_{\mathrm{left}})}{h_{\mathrm{s}}L_{\mathrm{right}}L_{\mathrm{left}} + \lambda_{\mathrm{m}}^{nn}(L_{\mathrm{right}} + L_{\mathrm{left}})} \ ,
\end{equation}

where $h$ depends on the matrix element surrounding the fracture:

- $\lambda_{\mathrm{m}}^{nn}$ is the component of the matrix thermal conductivity in the normal direction to the fracture;
- $L_{\mathrm{right}}$ is the average of the distances between the fracture and the nodes that lie on its right side;
- $L_{\mathrm{left}}$ is the average of the distances between the fracture and the nodes that lie on its left side (opposite the right side).

The result of [eqn.suggested.h] depends on $L_{\mathrm{left}}$ and $L_{\mathrm{right}}$.  In most settings, it is appropriate to assume that $L_{\mathrm{left}} = L_{\mathrm{right}} = L$ since this corresponds to making a shift of the fracture position by an amount less than the finite-element size.  Since the accuracy of the finite-element scheme is governed by the element size, such small shifts introduce errors that are smaller than the finite-element error.  This means [eqn.suggested.h] reads

\begin{equation}
\label{eqn.suggested.h.L}
h = \frac{2h_{\mathrm{s}}\lambda_{\mathrm{m}}^{nn}L}{h_{\mathrm{s}}L^{2} + 2\lambda_{\mathrm{m}}^{nn}L} \ .
\end{equation}

The heat-transfer through the skin is likely to exceed heat conduction through the skin, since the heat transfer involves faster processes such as convection.  Therefore, $h_{\mathrm{s}} = b\lambda_{\mathrm{m}}/ s$, where $s$ is the skin size and the numerical quantity $b>1$.  This yields

\begin{equation}
\label{eqn.simple.L}
h = \frac{2\lambda_{\mathrm{m}}^{nn}}{L + 2s/b} \rightarrow \frac{2\lambda_{\mathrm{m}}^{nn}}{L} \ ,
\end{equation}

where the final limit is for $s \ll L$, which is likely to be reasonably correct in most simulations.

Finally, consider the case which has very large fracture elements compared with matrix elements.  Then it could happen that $a A\gg V$, where $A$ is now the fracture area modelled by one finite-element fracture node, and $V$ is the volume modelled by one finite-element matrix node.  Then the single fracture node can apply a lot of heat to the "small" matrix node, which will likely cause oscillations or even numerical instability if $\Delta t$ is too large, in the MultiApp approach.

## Mass flow

A "mass-transfer coefficient" $h^{\mathrm{mass}}$ may be used to model fluid mass transfer between the fracture and matrix.  The equations for all fluid phases are structurally identical, but just have different numerical values for viscosity, relative permeability, etc, so in this section consider just one phase.  Define the potential $\Phi$ (with SI units Pa) by

\begin{equation}
\label{eqn.flow.potential}
\Phi = P + \rho g z
\end{equation}

Here

- $P$ is the porepressure (SI units Pa);
- $\rho$ is the fluid density (SI units kg.m$^{-3}$);
- $g$ is the gravitational acceleration (SI units m.s$^{-2}$ or Pa.m$^{2}$.kg$^{-1}$);
- $z$ is the coordinate direction pointing "upwards" (i.e. in the opposite direction to gravity) with SI units m.  If gravity acted in a different direction then $z$ would be replaced by another coordinate direction.

In the Darcy setting modelled by PorousFlow, the equivalent of the heat transfer $h(T_{f} - T_{m})$ found in [eqn.coupled.basic] is

\begin{equation}
Q^{\mathrm{mass}} = h^{\mathrm{mass}}(\Phi_{f} - \Phi_{m})
\end{equation}

and the equivalent of [heat.transfer.skin] is

\begin{equation}
Q_{\mathrm{s}}^{\mathrm{mass}} = h_{\mathrm{s}}^{\mathrm{mass}}(\Phi_{f} - \Phi_{s}) \ .
\end{equation}

Here

- $Q_{\mathrm{s}}^{\mathrm{mass}}$ is the mass flux through the skin, with SI units kg.s$^{-1}$.m$^{-2}$.
- $h_{\mathrm{s}}^{\mathrm{mass}}$ is the mass-transfer coefficient of the skin, with SI units kg.s$^{-1}$.m$^{-2}$.Pa$^{-1}$.

The heat-flow arguments presented above may now be followed using $\Phi$ instead of $T$, and $\rho k k_{\mathrm{rel}} / \mu$ in place of $\lambda$, to yield a suggestion for the mass-transfer coefficient in the 2D-3D situation:

\begin{equation}
\label{eqn.suggested.hmass}
h^{\mathrm{mass}} = \frac{h_{\mathrm{s}}^{\mathrm{mass}}\frac{\rho k_{\mathrm{m}}^{nn}k_{\mathrm{rel}}}{\mu} (L_{\mathrm{right}} + L_{\mathrm{left}})}{h_{\mathrm{s}}^{\mathrm{mass}}L_{\mathrm{right}}L_{\mathrm{left}} + \frac{\rho k_{\mathrm{m}}^{nn} k_{\mathrm{rel}}}{\mu}(L_{\mathrm{right}} + L_{\mathrm{left}})} \ ,
\end{equation}

where $h^{\mathrm{mass}}$ depends on the matrix element surrounding the fracture in the following way

- $\rho$ is the density of the fluid phase (SI units kg.m$^{-3}$).
- $k_{\mathrm{m}}^{nn}$ is the component of the matrix permeability tensor in the normal direction to the fracture (SI units m$^{2}$).
- $k_{\mathrm{rel}}$ is the relative permeability of the fluid phase (dimensionless).
- $\mu$ is the dynamic viscosity of the fluid phase (SI units Pa.s).
- $L_{\mathrm{right}}$ is the average of the distances between the fracture and the nodes that lie on its right side.
- $L_{\mathrm{left}}$ is the average of the distances between the fracture and the nodes that lie on its left side (opposite the right side).

This appears in the mass-transfer: $Q^{\mathrm{mass}} = h^{\mathrm{mass}}(\Phi_{f} - \Phi_{m})$, where $\Phi$ is given in [eqn.flow.potential].
The arguments that led to [eqn.suggested.h.L] and [eqn.simple.L] lead in this case to

\begin{equation}
\label{eqn.simple.hmass}
h^{\mathrm{mass}} = \frac{2\rho k_{\mathrm{m}}^{nn}k_{\mathrm{rel}}}{\mu L} \ .
\end{equation}

There are numerical subtleties in these expressions that may not impact many simulations, but modellers should be aware of the following.

- $\Phi$ seen by the fracture is the matrix nodal values interpolated to the fracture node.  There are clearly many ways of doing this interpolation, eg, $\rho z$ could be interpolated from the fracture nodes, or $\rho z$ could be evaluated at the fracture node given its position, porepressure and temperature.
- Similar remarks hold for $\rho k_{\mathrm{m}}^{nn}k_{\mathrm{rel}}/\mu$.
- Stability of the numerics will be improved by evaluating $\rho k_{\mathrm{m}}^{nn}k_{\mathrm{rel}}/\mu$ at the upwind position.  That is, if fluid is flowing from the fracture to the matrix, this term would best be evaluated using $(P, T)$ of the skin, while for the reverse flow the evaluation should be done using the nodal matrix values.  However, it is usually more convenient to fix the evaluation to use the elemental-averaged values of the matrix (using a constant, monomial `AuxVariable` for the matrix).


## Heat advection

Heat advection follows the same principle as mass transfer.  The heat transfer by advection through the skin:

\begin{equation}
Q_{\mathrm{s}} = eh_{\mathrm{s}}^{\mathrm{mass}}(\Phi_{f} - \Phi_{s}) \ .
\end{equation}

where $e$ is the fluid enthalpy, and the other terms are given above.  In the finite-element setting $Q = eh^{\mathrm{mass}}(\Phi_{f} - \Phi_{m})$, and

\begin{equation}
\label{eqn.suggested.ehmass}
eh^{\mathrm{mass}} = \frac{eh_{\mathrm{s}}^{\mathrm{mass}}\frac{e\rho k_{\mathrm{m}}^{nn}k_{\mathrm{rel}}}{\mu} (L_{\mathrm{right}} + L_{\mathrm{left}})}{eh_{\mathrm{s}}^{\mathrm{mass}}L_{\mathrm{right}}L_{\mathrm{left}} + \frac{e\rho k_{\mathrm{m}}^{nn} k_{\mathrm{rel}}}{\mu}(L_{\mathrm{right}} + L_{\mathrm{left}})} \ .
\end{equation}

The arguments that led to [eqn.suggested.h.L] and [eqn.simple.L] lead in this case to

\begin{equation}
\label{eqn.simple.ehmass}
eh^{\mathrm{mass}} = \frac{2e\rho k_{\mathrm{m}}^{nn}k_{\mathrm{rel}}}{\mu L} \ .
\end{equation}


As in the mass-transfer case, the numerics will be impacted by where these terms are evaluated.

## Combinations of transfer

In many simulations, heat transfer by both conduction and convection will be active, in addition to mass transfer.  The transfers are simply added together.  For instance

\begin{equation}
Q^{\mathrm{heat}} = h(T_{f} - T_{m}) + eh^{\mathrm{mass}}(\Phi_{f} - \Phi_{m}) \ .
\end{equation}

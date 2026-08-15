# Mass lumping

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)

This page describes mass lumping.  There is an added complication in mechanically-coupled systems that involve mesh deformation, and [another page](porous_flow/time_derivative.md) describes the numerical implementation in such cases.

Consider here just the fluid-flow equation, as the heat-energy equation is analogous.  The
time-derivative term is discretised as

\begin{equation}
\label{eq:lump_begin}
\psi \frac{M^{\kappa} - M^{\kappa}_{\mathrm{old}}}{\mathrm{d}t} \ .
\end{equation}

Here $\psi$ is the test function that we are integrating against.  An alternative discretisation
would be $\psi\phi(S\rho)'\dot{P}$ (in the single-phase situation), but [eq:lump_begin] conserves
mass more effectively than other alternatives.

In the standard finite-element scheme, $M^{\kappa}$ and its individual parts (porosity, saturation,
etc) are evaluated at each quadrature point.  However, in PorousFlow, everything in the time
derivative is evaluated at the nodes.  Specifically, $M^{\kappa}$ at a node depends only on the
independent variables at that node.  It has been shown in many studies that this lumping is
advantageous for mass conservation and reduces spurious oscillations of the pressure around sharp
fronts [!citep](celia1990).

The cause of oscillations around sharp fronts, and how mass lumping removes the oscillations, can be
illustrated through a simple example.

!media media/porous_flow/mass_lumping.png
       style=width:60%;margin-left:10px;
       id=fig:mass_lumping
       caption=Two elements of length $L$.  Linear Lagrange shape/test functions for each node are
               shown in red ($S_{0}$ for node 0, $S_{1}$ for node 1, and $S_{2}$ for node 2).
               Gravity acts in the direction $-z$.  Gauss points are shown in green.

Consider the situation in [fig:mass_lumping], and suppose that Node 2 has high potential, and that
Nodes 0 and 1 are at residual saturation where the relative permeability is zero.  Then fluid will
flow from Node 2 to Node 1 (and then to Node 0 in the next time step).  For simplicity, imagine that
the fluid mass, $M$, is a linear function of the potential.  Then, up to constants, the discretised
mass-conservation equation without mass lumping reads

\begin{equation}
\left(
\begin{array}{ccc}
2 & 1 & 0 \\
1 & 4 & 1 \\
0 & 1 & 2
\end{array}
\right)
\left(
\begin{array}{c}
\dot{M}_{0} \\
\dot{M}_{1} \\
\dot{M}_{2}
\end{array}
\right)
=
\left(
\begin{array}{c}
0 \\
1 \\
-1
\end{array}
\right)
\end{equation}

The matrix on the LHS comes from performing the numerical integration of $M$ over the two elements.
Note that it is not diagonal because the integration over an element depends on the mass at both of
its two nodes.  The RHS encodes that no fluid is flowing between Nodes 0 and 1, but fluid is flowing
from Node 2 to Node 1.

The important point is the solution of these sets of equations is
\begin{equation}
\dot{M}_{0} < 0 \ .
\end{equation}
This means the finite element solution of the mass-conservation equation will be oscillatory around
fronts.

However, with mass lumping, the matrix in the above equation becomes diagonal,
and the solution is $\dot{M}_{0} = 0$.  Explicitly, the contribution to the
residual at node $a$ is

\begin{equation}
\sum_{\mathrm{qp}}\psi_{a}^{\mathrm{qp}} \frac{M_{a}^{\kappa} -
  M^{\kappa}_{a,\ \mathrm{old}}}{\mathrm{d} t} \ .
\end{equation}
where $M$ is evaluated at node $a$ using the independent (nonlinear) variables evaluated at that
node, and qp are the quadpoints.

There is one small complication.  Porosity may be dependent on volumetric strain, which is dependent
on the gradients of the displacement variables, which are evaluated at the quadpoints, not the nodes.
In this case, the porosity at node $a$ is assumed to be dependent on the volumetric strain evaluated
at the closest quadpoint to the node.


## Which nodes are evaluated id=which_nodes

The nodal quantities are evaluated at those nodes that carry a degree of freedom of the
PorousFlow variables, which is not necessarily every node of the element.  The two coincide for
a consistent Lagrange discretisation, and differ when a first-order variable is used on a
second-order mesh: a first-order porepressure on a TET10 element has 4 degrees of freedom against
the element's 10 nodes, and only those 4 vertices are evaluated.

It follows that every variable a nodal `Material` reads at the nodes must share an FE type, since
the single node counter used to index the nodal `Material` properties has to be a valid
degree-of-freedom index for all of them at once.  The [PorousFlowDictator](dictator.md) resolves
that type from the variables the `Materials` actually read, and produces an error if they disagree.

The displacements are not subject to this, because no `Material` reads them at the nodes:
`PorousFlowVolumetricStrain` is evaluated at the quadpoints only.  Second-order displacements may
therefore be used alongside a first-order porepressure.

A variable that is *elemental* (a `MONOMIAL` `AuxVariable`, say) has no nodal degrees of freedom at
all, so a nodal `Material` reads it at the quadpoints instead.  This is how a spatially varying
reference temperature or mineral concentration may be supplied to
[PorousFlowPorosity](PorousFlowPorosity.md) as a constant monomial.


!bibtex bibliography

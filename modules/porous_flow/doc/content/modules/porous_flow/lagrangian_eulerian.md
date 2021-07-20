# Lagrangian and Eulerian coordinates

PorousFlow is formulated in Lagrangian coordinates.  This page contains a description of Eulerian and Lagrangian coordinate systems and the continuity equation in each.  This page is self contained and uses notation that might be different from other PorousFlow documentation.

## Eulerian coordinates

Introduce the notion of the "spatial coordinate frame".  It is the
coordinate frame of a stationary observer who is looking at the
deforming porous solid from the outside.  Denote its coordinates by
${\mathbf x}$, and the derivatives with respect those coordinates by
$\nabla$.

Let $\Omega$ be a volume that is attached to particles of the
porous-solid skeleton.  As the porous solid deforms, so too will
$\Omega$.  Denote the velocity of the porous solid is ${\mathbf
  v}_{s}$, measured in the spatial coordinate frame: ${\mathbf v}_{s}
= {\mathbf v}_{s}(x, t)$.  Then the change of a small volume element
${\mathrm d}\Omega$ is computed by calculating the Jacobian, and it is
\begin{equation}
\frac{{\mathrm d}}{{\mathrm d} t} ({\mathrm d}\Omega) = \nabla\cdot{\mathbf v}_{s} {\mathrm d}\Omega \ .
\end{equation}
Remember the derivative $\nabla$ is differentiating with respect to
the coordinates of the spatial coordinate frame.  This formula is easy
to motivate for readers familiar with solid mechanics because $\nabla\cdot{\mathbf v}_{s} = \dot{\epsilon}_{ii}$,
which is the time derivative of the volumetric strain.

Let $M$ represent a quantity that is attached to the porous-solid
skeleton, for instance the mass density of the solid.  Express $M$ in
the spatial coordinate frame: $M=M(x, t)$.  As the porous
solid deforms
\begin{equation}
\frac{{\mathrm d}}{{\mathrm d} t}M = \frac{\partial}{\partial t}M + {\mathbf v}_{s}\cdot
\nabla M \ .
\end{equation}
The second term is easy to motivate by considering a constant
velocity with spatially-dependent but temporally-constant $M$.

The continuity equation is
\begin{equation}
\frac{{\mathrm d}}{{\mathrm d} t}\int_{\Omega}M {\mathrm d}\Omega + \int_{\partial
  \Omega}{\mathbf F}\cdot{\mathbf n}\,{\mathrm d} A = 0 \ ,
\end{equation}
where ${\mathbf F}$ is the flux of $M$ out of $\Omega$, $n$ is the
outward unit normal, and ${\mathrm d} A$ is the area element on
$\partial\Omega$ (which is the surface of $\Omega$).  Using the above
expressions, and the divergence theorem, the continuity equation reads
\begin{equation}
\int_{\Omega} \left( \frac{\partial}{\partial t}M + \nabla\cdot(M{\mathbf
  v}_{s}) + \nabla\cdot{\mathbf F}  \right){\mathrm d}\Omega = 0 \ .
\label{equation.continuity.eul}
\end{equation}
Specialising $M$ and ${\mathbf F}$ to fluids and heat gives the
equations mentioned in the main text.

## Lagrangian coordinates

Introduce the notion of the "material coordinate frame".  It is the
coordinate frame of an observer who is fixed to a certain point in the
porous solid (eg, a particular finite-element node).  Denote the
coordinates in this frame by $X$.  This is the frame used by
PorousFlow: the material coordinate frame can be considered to be the
mesh.  Fluid properties (pressures, mass fractions), the temperature,
etc, are all stored at the finite-element nodes or the quadpoints, and
move with the mesh.  At the very least, an Eulerian description would
be inconvenient when visualising with paraview.

Introduce the material derivative $\mathrm{D}/\mathrm{D} t$.  It is the total time derivative as seen by an observer living in the Lagrangian frame.  For instance, if the mesh isn't deforming, but is moving as a rigid body through space, then $\mathrm{D}/\mathrm{D} t = 0$ since the observer will see no change.  If $M$
is any property that is expressed in terms of the "spatial coordinate
frame" (Eulerian coordinates): $M=\tilde{M}(x,t)$ (for some function $\tilde{M}$) then the material
derivative is defined to be
\begin{equation}
\frac{\mathrm{D}}{\mathrm{D} t}\tilde{M}(x, t) = \frac{\partial
}{\partial t}\tilde{M}(x, t) + {\mathbf{v}}_{s}\cdot \nabla
\tilde{M}(x, t)\ .
\end{equation}
The continuity
[equation.continuity.eul] can be re-written as
\begin{equation}
0 = \frac{\mathrm{D}}{\mathrm{D} t}M + M\nabla\cdot {\mathrm{v}}_{s} + \nabla\cdot{\mathrm F}
\ .
\end{equation}
However, if $M$ is expressed in terms of the Lagrangian coordinates:
\begin{equation}
M = M(X, t) \ ,
\end{equation}
(generally $M(X, t)$ will have a different functional form than
$\tilde{M}(x,t)$, thus the tilde to emphasise the difference)
then the material derivative is expressed by
\begin{equation}
\frac{\mathrm{D}}{\mathrm{D} t}M(X, t) = \frac{\partial}{\partial
  t}M(X, t) \ ,
\end{equation}
but the continuity equation has the identical form:
\begin{equation}
0 = \frac{\mathrm{D}}{\mathrm{D} t}M + M\nabla\cdot {\mathrm{v}}_{s} + \nabla\cdot{\mathrm F}
\ .
\end{equation}


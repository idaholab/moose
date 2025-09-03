# Magnetostatic Problem on a Topologically Closed Conductor

## Summary

Solves for the magnetic field around a topologically closed toroidal conductor carrying a net
  current, using a magnetic vector potential discretized using $H(\mathrm{curl})$ conforming Nédélec
  elements, following the method described in [P. Dular, International Compumag Society Newsletter, 7, no. 2 (2000): 4-7.](https://hdl.handle.net/2268/191358) for enforcing
  global constraints.

## Description

This problem solves the magnetostatic problem with strong form:

\begin{equation}
\begin{split}
\vec \nabla \cdot \vec B
&= 0 \\
 \vec \nabla \times \vec H &= \vec J  \\
 \vec \nabla \times \vec E &= - \partial_t \vec B = \vec 0
\end{split}
\end{equation}

with the constitutive relations

\begin{equation}
\begin{split}
\vec B
&= \mu \vec H \\
 \vec E &= \rho \vec J + \vec E_{ext}\\
 \vec J &= \sigma (\vec E - \vec E_{ext})
\end{split}
\end{equation}

where $\vec B$ is the magnetic flux density, $\vec H$ is the magnetic field, $\vec J$ is the current
 density, $\vec E$ is the electric field, $\vec E_{ext}$ is the externally applied electric field,
 $\mu$ is the material permeability, $\rho$ is the material resistivity, and $\sigma$ is the material
 conductivity. To enforce the divergence free condition on $\vec B$, we shall also introduce the
 magnetic vector potential $\vec A$ defined such that $\vec \nabla \times \vec A = \vec B$.

In addition, we wish to impose a global voltage constraint on a toroidal conductor domain $\Omega_c$ of the form

\begin{equation}
    \oint_{\mathcal C} \vec E \cdot d\vec l = -\mathcal V
\end{equation}

where $\mathcal C$ is any path traversing around the conductor lying entirely within $\Omega_c$ with winding
 number equal to one, and $\mathcal V$ is the loop voltage imposed on the conductor.

In a simply connected domain, satisfying  $\vec \nabla \times \vec E =  \vec 0$ can be achieved by
 introducing a continuous single-valued scalar electric potential $\phi$ such that
 $\vec E = -\vec\nabla \phi$. However, in order for the electric field $\vec E$ to be nonzero circulating the closed
 conductor, this is no longer sufficient, since $\oint_{\mathcal C} \vec\nabla \phi \cdot d\vec l = 0$.
 We shall therefore represent the electric field by

\begin{equation}
    \vec E = - \vec \nabla \phi + \vec E_{ext}.
\end{equation}

With this, we can arrive at the weak form for $\vec \nabla \times \vec H = \vec J$ we intend to solve, of the form

\begin{equation}
\begin{split}
(\mu^{-1} \vec \nabla \times \vec A, \vec \nabla \times \vec A')_\Omega - (\vec H \times \vec n, \vec A')_{\partial \Omega}
+(\sigma \vec \nabla \phi, \vec A')_{\Omega_c} +
(\sigma \vec E_{ext}, \vec A')_\Omega
&= 0 \,\,\, \forall \vec A' \in V_A \\
 (\sigma \vec \nabla \phi, \vec \nabla \phi')_{\Omega_c}
+(\sigma \vec E_{ext}, \vec \nabla \phi')_{\Omega_c} &= 0 \,\,\, \forall \phi' \in V_\phi \\
 (\sigma \vec \nabla \phi, \vec E_{ext}')_{\Omega_c}
+(\sigma \vec E_{ext}, \vec E_{ext}')_{\Omega_c} &= IV(\vec E_{ext}') \,\,\, \forall \vec E_{ext}' \in V_E
\end{split}
\end{equation}

where

\begin{equation}
\begin{split}
\vec A, \vec A' \in H(\mathrm{curl})(\Omega) &: \vec A \times \hat n= \vec 0 \,\,\, \mathrm{on}\,\, \partial\Omega \\
\phi, \phi' \in H^1(\Omega_c) &: \vec \nabla \phi \cdot \hat n= 0 \,\,\, \mathrm{on}\,\, \partial\Omega_c \\
\vec E_{ext}, \vec E_{ext}' \in H(\mathrm{curl})(\Omega_c) &: \vec \nabla \times \vec E_{ext} = \vec 0 \,\,\, \mathrm{in}\,\, \Omega_c, \,  \oint_{\mathcal C}  \vec E_{ext} \cdot d\vec l = \mathcal V
\end{split}
\end{equation}

## Discretizing the External Electric Field

We must now find a suitable choice of basis ${\vec c_i \in H(\mathrm{curl})(\Omega_c)}$ for the
external electric field, such that

\begin{equation}
    \vec E_{ext} = - \sum_{i \in \Omega_c} V_i \vec c_i
\end{equation}

for some unique set of degrees of freedom $\{V_i\}$. We shall also wish to restrict the support of $\{\vec c_i\}$ to the smallest number of elements possible in $\Omega_c$ to minimise the number of degrees of freedom required to represent $\vec E_{ext}$.

To do this, we first identify a 'cut' surface $\Gamma_c$ of the toroidal the conductor that is required to make it simply connected.
Next, we define a one element wide 'transition' region $\Omega_t \subset \Omega_c$
on one side of the cut surface, using an `MFEMCutTransitionSubMesh` object, comprised of all
elements with at least one vertex lying on the cut surface that lie on one side of the cut.

Next, to enforce $\vec \nabla \times \vec E_{ext} =0$ in this region, we define a scalar field variable $v$ defined on a (nodal) $H^1$ conforming finite element space on this submesh such that $\vec E_{ext} = -\vec \nabla v$:

\begin{equation}
  v = \sum_{i \in \Omega_t} V_i v_i
\end{equation}

so ${\vec c_i = \vec \nabla v_i}$.

Finally, to impose the global voltage constraint $\mathcal V = \oint_{\mathcal C} \vec E_{ext} \cdot d\vec l$, we identify that

\begin{equation}
  \mathcal V = \oint_{\mathcal C} \vec E_{ext} \cdot d\vec l = v|_{\Gamma_c} - v|_{\partial \Omega_t \backslash \Gamma_c}
\end{equation}

since $\vec c_i = 0$ outside $\Omega_t$. The loop voltage constraint is thus strongly imposed by
 Dirichlet conditions on $v$, setting $v=0$ on $\partial \Omega_t \backslash \Gamma_c$ and
 $v =\mathcal V$ on $\Gamma_c$. For lowest order $H^1$ conforming representations of $v$, this fully
 constrains $\vec E_{ext}$ in the transition region.

This is shown explicitly in the example file:

!listing test/tests/mfem/submeshes/cut_closed_coil.i

## Solving for the Total Electric Field

With $\vec E_{ext}$ now constrained, we now must solve for the remaining degrees of freedom in
 $\vec E$ arising due to surface charges in the coil as required for continuity of current in the conductor
 ($\vec \nabla \cdot \vec J = 0$), by solving

\begin{equation}
(\sigma \vec \nabla \phi, \vec \nabla \phi')_{\Omega_c}
+(\sigma \vec E_{ext}, \vec \nabla \phi')_{\Omega_t} = 0 \,\,\, \forall \phi' \in V_\phi
\end{equation}

for $\phi$. We then can calculate the total electric field from the sum of its induced and external components.

## Solving for the Magnetic Flux Density

Finally, with the electric field in the conductor, we can now find the magnetic flux density around the current density flowing in the coil in the magnetostatic limit.

\begin{equation}
(\mu^{-1} \vec \nabla \times \vec A, \vec \nabla \times \vec A')_\Omega - (\vec H \times \vec n, \vec A')_{\partial \Omega}
+(\sigma \vec \nabla \phi, \vec A')_{\Omega_c} +
(\sigma \vec E_{ext}, \vec A')_\Omega = 0 \,\,\, \forall \vec A' \in V_A
\end{equation}

This is typically solved using `HypreAMS` as a preconditioner. Note that the above magnetostatic
problem is singular in non-conductive regions, and thus either `singular = true` should be passed to
the preconditioner or a small mass term $(\epsilon \vec A, \vec A')_\Omega$ with $\epsilon << 1$
should be added for stability.

## Example File

The full closed coil magnetostatic example detailed above can be found below:

!listing test/tests/mfem/submeshes/av_magnetostatic.i

## Global Current Constraints

Strongly constraining the total current through the conductor instead of the loop voltage is also
possible, by defining a cut plane in the domain outside the conductor region using the H-phi
formulation instead of the A-V formulation detailed above; the details are similar, using a magnetic
scalar potential instead of an electric scalar potential. The constraint to be enforced is

\begin{equation}
\oint_{\mathcal C} \vec H \cdot d\vec l = I
\end{equation}

where $\mathcal C$ is any path encircling the conductor, and I is the total current passing through
the path $\mathcal C$. Outside the conductor, the magnetic field $\vec H$ is curl-free, and thus can
be expressed by the gradient of a scalar magnetic potential (up to a sign).

An example file for this approach can be found below:

!listing test/tests/mfem/submeshes/hphi_magnetostatic.i

# Magnetostatic Problem on a Topologically Closed Conductor

## Summary

Solves for the magnetic field around a topologically closed toroidal conductor carrying a net
  current, using a magnetic vector potential discretized using $H(\mathrm{curl})$ conforming Nédélec
  elements, following the method described in [P. Dular. "Local and Global Constraints in Finite
  Element Modeling and the Benefits of Nodal and Edge Elements Coupling." International Compumag
  Society Newsletter, 7, no. 2 (2000): 4-7.](https://hdl.handle.net/2268/191358) for enforcing
  global constraints.

## Description

This problem solves the magnetostatic problem with strong form:

\begin{equation}
\vec \nabla \times \left(\mu^{-1} \vec \nabla \times \vec A \right) = \vec J
\end{equation}

where the current $\vec J$ is determined from the source electric field in the conductor arising from imposed loop voltage $\mathcal V$ on a cross section of a toroidal conductor. Specifically, the global constraint we shall impose is

\begin{equation}
    \oint_{\mathcal C_i} \vec E \cdot d\vec l = -\mathcal V
\end{equation}

where $\mathcal C_i$ is any path traversing around the conductor with winding number equal to one.

## Constraining the Electric Field

In order for the electric field $\vec E$ to be nonzero circulating the closed conductor, it cannot be represented by the gradient of a single-valued continuous scalar potential $\phi$ over the conductor (up to a sign), since the conductor domain is no longer simply connected inside the meshed region. By necessity, we must add an additional contribution to $\vec E$ from a set of global vector basis functions ${\vec c_i}$ associated with the cut $\Gamma_C$ that makes the closed conductor simply connected.

We represent the electric field by

\begin{equation}
    \vec E = -\sum_{n \in \Omega_c} E_n \vec \nabla \phi_n - \sum_{i \in \Gamma_C} V_i \vec c_i
\end{equation}
\begin{equation}
    \vec E = \vec E_{ind} + \vec E_{ext}
\end{equation}

where the loop voltage $\mathcal V = \oint_{\mathcal C_i}  \sum_{i \in \Gamma_C} V_i \vec c_i \cdot d\vec l$.

### Defining Cut Functions

We shall wish to restrict the support of $\{\vec c_i\}$ to the smallest number of elements possible to minimise the number of new degrees of freedom associated with the electric field.

We will construct the basis functions $\vec c_i$ by first defining a 'transition' region $\Omega_t$ on one side of the cut surface, using an `MFEMCutTransitionSubMesh` object, comprised of all elements with at least one vertex lying on the cut surface that lie on one side of the cut.

Next, we define a scalar variable $v$ defined on a (nodal) $H^1$ conforming finite element space on this submesh:

\begin{equation}
    v = \sum_{i \in \Omega_t} V_i \varphi_i
\end{equation}

On all vertices in the transition region not lying on the cut surface, we set $v=0$ via a Dirichlet condition. If we take the cut functions $\vec c_i = \nabla \varphi_i$ on the cut surface, we can identify $v$ on the cut surface as the loop voltage, which we can use to strongly impose $v=\mathcal V$ via a second Dirichlet condition.

This is shown explicitly in the example file:

!listing test/tests/mfem/submeshes/cut_transition.i

## Solving for the Induced Electric Field

With $\vec E_{ext}$ now constrained, we now must solve for the remaining degrees of freedom in $\vec E$ arising due to surface charges in the coil as required for continuity of current in the conductor ($\vec \nabla \cdot \vec J = 0$).

Expressing $\vec J = \sigma \vec E$, we can express $\vec \nabla \cdot \vec J = 0$ in weak form as

\begin{equation}
  -(\sigma \vec \nabla \phi, \vec \nabla \phi')_{\Omega_c} 
  -(\sigma \vec E_{ext}, \vec \nabla \phi')_{\Omega_t} = 0 \,\,\, \forall \phi' \in W
\end{equation}

Finally, we can calculate the total electric field from the sum of its induced and external components

!listing test/tests/mfem/submeshes/cut_closed_coil.i

## Solving for the Magnetic Flux Density

Finally, with the electric field in the conductor, we can now find the magnetic flux density around the current density flowing in the coil in the magnetostatic limit.

\begin{equation}
(\mu^{-1} \vec \nabla \times \vec A, \vec \nabla \times \vec A')_\Omega -(\vec H \times \vec n, \vec A')_{\partial \Omega}
&= (\sigma \vec E, \vec A')_\Omega \,\,\, \forall \vec A' \in V
\end{equation}

This is typically solved using `HypreAMS` as a preconditioner. Note that the above magnetostatic problem is singular in non-conductive regions, and thus either `singular = true` should be passed to the preconditioner or a small mass term $(\epsilon \vec A, \vec A')_\Omega$ with $\epsilon << 1$ should be added for stability.

## Example File

The full closed coil magnetostatic example detailed above can be found below:

!listing test/tests/mfem/submeshes/cut_magnetostatic.i

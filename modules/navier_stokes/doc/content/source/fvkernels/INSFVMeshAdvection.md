# INSFVMeshAdvection

As outlined in [!citep](maury1996characteristics), when converting a time
derivative term in the reference frame to a time derivative term in the
arbitrary Lagrangian-Eulerian (ALE) frame, the following equation should be used

\begin{equation}
\label{maury}
\frac{\partial\phi}{\partial t} = \frac{\partial \phi_{\tau}}{\partial t} -
\vec{c}_{\tau} \nabla\phi_{\tau}
\end{equation}

where $\phi$ is an arbitrary quantity we are taking the time derivative of,
$\tau$ denotes the ALE frame at a given time point, and $\vec{c}$ denotes the
mesh velocity or rate of change of coordinates in the ALE frame; this is
equivalent to $\partial\vec{d}/\partial t$, e.g. the partial derivative of the
displacement field with respect to time on the reference mesh.

In a Navier-Stokes setting, we are interested in implementing [maury] multiplied
through by the density $\rho$:

\begin{equation}
\label{NS_ALE}
\rho\frac{\partial\phi}{\partial t} = \rho\frac{\partial \phi_{\tau}}{\partial t} -
\rho\vec{c}_{\tau} \nabla\phi_{\tau}
\end{equation}

In the finite volume setting we compute advective fluxes on faces. This requires
constructing divergence terms of the form $\nabla \cdot \Box$. To do so we
rewrite the last term on the right hand side of [NS_ALE] using the product rule
(dropping the $\tau$ subscripts from here on out) as

\begin{equation}
-\rho\vec{c}\nabla\phi = -\nabla\cdot\left(\rho\vec{c}\phi\right) +
\phi\nabla\cdot\left(\rho\vec{c}\right)
\end{equation}

For a constant density $\rho$ can be moved outside of divergence operator
leaving the last term on the right hand side as

\begin{equation}
\label{mesh_advection}
\rho\phi\nabla\cdot\vec{c}
\end{equation}

where again $\vec{c} = \partial\vec{d}/\partial t$. `INSFVMeshAdvection`
implements [mesh_advection]. The `advected_quantity` parameter  specifies $\phi$. For
energy transport, this would be the specific enthalpy $h$. For momentum
transport it is the velocity, e.g. the `variable` parameter itself. The momentum
transport version of this object is [INSFVMomentumMeshAdvection.md]. It
suppresses the `advected_quantity` parameter in order to advect the `variable`.

!syntax parameters /FVKernels/INSFVMeshAdvection

!syntax inputs /FVKernels/INSFVMeshAdvection

!syntax children /FVKernels/INSFVMeshAdvection

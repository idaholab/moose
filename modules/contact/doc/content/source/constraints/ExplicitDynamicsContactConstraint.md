# ExplicitDynamicsContactConstraint

!syntax description /Constraints/ExplicitDynamicsContactConstraint

This object implements node-face constraints for the enforcement of normal
mechanical contact in explicit dynamics. Surrogate balance-momentum equations are
solved at each node on the secondary surface.

This method MUST be used with the [DirectCentralDifference](source/timeintegrators/DirectCentralDifference.md) time integrator found in the Solid Mechanics module.

Following the work of [!citep](heinstein2000contact), the contact is constrained as such,

\begin{equation}
    \begin{aligned}
        \mathbf{a_n} &= \mathbf{M}^{-1}(\mathbf{F}^\text{ext}-\mathbf{F}^{\text{int}}(\mathbf{u}_n)-\mathbf{G}^T_n\lambda_n)\\
    \end{aligned}
\end{equation}
where $\lambda_n$ is the contact force, and $\mathbf{G_n}$ is the Jacobian of the contact constraints. $\lambda_n$ is calculated with the following iteration,
\begin{equation}
    \begin{aligned}
        \mathbf{v}^{0}_{n+\frac{1}{2}} &= \mathbf{M}^{-1}(\mathbf{F}^\text{ext}-\mathbf{F}^{\text{int}}(\mathbf{u}_n)),\\
        \mathbf{v}^{j+1}_{n+\frac{1}{2}} &= \mathbf{v}^j_{n+\frac{1}{2}}-\Delta t \mathbf{M}^{-1}\mathbf{G_n}(\lambda^{j+1}_n-\lambda^{j}_n),\\
        j & \leftarrow j+1, \text{next } j,
    \end{aligned}
\end{equation}

where $\mathbf{v}^0_{n+\frac{1}{2}}$ is the initial guess for the midstep velocity.

$\lambda_n$ is calculated at each node as a function of the wave speed and density of the primary and secondary material, the gap rate $\mathbf{\dot g}$, and tributary area $\Delta A$,
\begin{equation}
    \begin{aligned}
        \lambda_{n_q} = \frac{\rho^{(1)}\rho^{(2)}c^{(1)}c^{(2)}}{\rho^{(1)}c^{(1)}+\rho^{(2)}c^{(2)}}\mathbf{\dot g}_{n+\frac{1}{2}}\mid_q \Delta A_q,
    \end{aligned}
\end{equation}

where the subscript $_q$ is a nodal index. With this formulation, the constraint force will be calculated within a few iterations.
<!-- For relevant equations, see [!citep](heinstein2000contact), in particular,
Equations (15), (21), (26) and (29). -->

!syntax parameters /Constraints/ExplicitDynamicsContactConstraint

!syntax inputs /Constraints/ExplicitDynamicsContactConstraint

!syntax children /Constraints/ExplicitDynamicsContactConstraint

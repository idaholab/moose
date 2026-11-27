# ADInertialForceShell

This kernel accounts for inertia on both the displacement (translational) and the rotational degrees of freedom of the shell element.
Forward mode automatic differentiation is used to compute an exact Jacobian. Please refer to [ShellElements](/ShellElements.md) for details.

For a translational component, the kernel adds the following residual at node $k$ of each element:

\begin{equation}
f_k = \rho\, a \sum_{l=1}^{4} \left( \int_A h_k h_l \, dA \right) \left( \ddot{u}_l + \eta\, \dot{u}^{*}_l \right)
\end{equation}

For a rotational component, the kernel adds the following residual at node $k$ of each element:

\begin{equation}
m_k = \frac{\rho\, a^3}{12} \sum_{l=1}^{4} \left( \int_A h_k h_l \, dA \right) \left( \ddot{\theta}_l + \eta\, \dot{\theta}^{*}_l \right)
\end{equation}

In these equations, $a$ is the [!param](/Kernels/ADInertialForceShell/thickness), $\rho$ is the [!param](/Kernels/ADInertialForceShell/density) material property, and $h_k$ are the in-plane shape functions. Thus the mass per unit area is $\rho a$ for the translations, and the rotary inertia per unit area is $\rho a^3 / 12$. The thickness uses the same length unit as the mesh. The translational components $u$ are global displacement components. The rotational components $\theta$ are the two nodal rotations of the shell kinematics. The mass is consistent.

The mass-proportional Rayleigh damping coefficient $\eta$ is the [!param](/Kernels/ADInertialForceShell/eta) parameter. The HHT-weighted velocities are $\dot{u}^{*} = (1 + \alpha_H)\, \dot{u} - \alpha_H\, \dot{u}_{old}$ and $\dot{\theta}^{*} = (1 + \alpha_H)\, \dot{\theta} - \alpha_H\, \dot{\theta}_{old}$, with $\alpha_H$ the [!param](/Kernels/ADInertialForceShell/alpha) parameter. The allowed range is $-0.3333 \le \alpha_H \le 0$. Both [!param](/Kernels/ADInertialForceShell/eta) and [!param](/Kernels/ADInertialForceShell/alpha) apply to the translational and the rotational residuals. The nodal velocities and accelerations carry AD derivatives, so the Jacobian includes the mass and the Rayleigh damping terms.

Each `ADInertialForceShell` kernel calculates the force only along one coordinate direction. So, a separate `ADInertialForceShell` input block should be set up for each coordinate direction. Additionally, separate
`ADInertialForceShell` kernels should be specified for the rotation variables.

!syntax parameters /Kernels/ADInertialForceShell

!syntax inputs /Kernels/ADInertialForceShell

!syntax children /Kernels/ADInertialForceShell

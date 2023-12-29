The stress divergence kernel handles the calculation of the residual, $\mathbb{R}$, from the
governing equation and the calculation of the Jacobian.  From the strong form of the governing
equation for mechanics, neglecting body forces,
\begin{equation}
\nabla \cdot \sigma = 0
\end{equation}
the weak form, using Galerkin's method and the Gauss divergence theorem, becomes
\begin{equation}
\int_S n \cdot (\sigma \psi) dS - \int_V \sigma \cdot \nabla \psi dV = 0
\end{equation}
in which $\psi$ is the test function.  The second term of the weak form equation is the residual
contribution calculated by the stress divergence kernel.

The calculation of the Jacobian can be approximated with the elasticity tensor if the simulation
solve type is +JFNK+:

\begin{equation}
\frac{d(\sigma_{ij} \cdot d(\psi) / dx_j)}{du_k} = \frac{d(C_{ijmn} \cdot du_m / dx_n \cdot d\psi /dx_j)}{du_k}
\end{equation}
which is nonzero for $m == k$.

If the solve type for the simulation is set to +NEWTON+ the finite deformation Jacobian will need
to be calculated.  Set the parameter `use_finite_deform_jacobian = true` in this case.


!alert note title=Use of the Tensor Mechanics Master Action Recommended
The `use_displaced_mesh` parameter must be set correcting to ensure consistency in the equilibrium
equation: if the stress is calculated with respect to the deformed mesh, the test function gradients
must also be calculated with respect to the deformed mesh. The
[Tensor Mechanics MasterAction](/Modules/TensorMechanics/Master/index.md) is designed to
automatically determine and set the parameter correctly for the selected strain formulation.  We
recommend that users employ the
[Tensor Mechanics MasterAction](/Modules/TensorMechanics/Master/index.md) whenever possible
to ensure consistency between the test function gradients and the strain formulation selected.

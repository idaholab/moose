The stress divergence kernel handles the calculation of the residual, $\mathbb{R}$, from the governing equation and the calculation of the Jacobian.

From the strong form of the governing equation for mechanics, neglecting body forces,
$$
\nabla \cdot \sigma = 0
$$

the weak form, using Galerkin's method and the Gauss divergence theorem, becomes
$$
\int_S n \cdot (\sigma \psi) dS - \int_V \sigma \cdot \nabla \psi dV = 0
$$
in which $\psi$ is the test function.  The second term of the weak form equation is the residual contribution calculated by the stress divergence kernel.

The calculation of the Jacobian can be approximated with the elasticity tensor if the simulation solve type is **JFNK**:
$$
\frac{d(\sigma_{ij} \cdot d(\psi) / dx_j)}{du_k} = \frac{d(C_{ijmn} \cdot du_m / dx_n \cdot d\psi /dx_j)}{du_k}
$$
which is nonzero for $m == k$.

If the solve type for the simulation is set to **NEWTON** the finite deformation Jacobian will need to be calculated.  Set the parameter `use_finite_deform_jacobian = true` in this case.


!!! note "Settings for Finite Strain Simulations"
    Set the use_displaced_mesh parameter equal to true for finite strain (large deformation) problems to ensure that the stress divergence residual is calculated correctly.  If you are unsure if your problem is finite strain or small strain, use the setting `use_displaced_mesh = true`.

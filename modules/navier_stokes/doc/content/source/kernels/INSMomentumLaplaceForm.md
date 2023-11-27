# INSMomentumLaplaceForm

!syntax description /Kernels/INSMomentumLaplaceForm

This class implements the same terms as described in
[INSMomentumTractionForm.md] but with a simplification applied to the viscous
stress. For constant density and viscosity, the following simplification can be
applied, as described in [!citep](peterson2018overview):

\begin{aligned}
\nabla\cdot\sigma &= \nabla\cdot\left(-p\bm{I} + \mu\left(\nabla\vec{u} + \left(\nabla\vec{u}\right)^T\right)
                  &= -\nabla p + \mu\left(\nabla\cdot\nabla\vec{u} + \nabla\cdot\left(\nabla\vec{u}\right)^T\right)
                  &= -\nabla p + \mu[left(\nabla\left(\nabla\cdot\vec{u}\right) + \nabla\cdot\left(\nabla\vec{u}\right)^T\right)
                  &= -\nabla p + \mu\nabla\cdot\left(\nabla\vec{u}\right)^T
                  &= -\nabla p + \mu\nabla^2\vec{u}
\end{aligned}

Moving from lines 2 to 3 in the above equation we assumed sufficient smoothness in
$\vec{u}$ to interchange the divergence and gradient operations. In moving from
lines 3 to 4 we applied the incompressibility constraint $\nabla\cdot\vec{u} =
0$. The final form is a vector Laplacian (which can be different in
non-Cartesian coordinate systems from a componentwise scalar Laplacian), hence
the designation the "Laplace" form.

!syntax parameters /Kernels/INSMomentumLaplaceForm

!syntax inputs /Kernels/INSMomentumLaplaceForm

!syntax children /Kernels/INSMomentumLaplaceForm

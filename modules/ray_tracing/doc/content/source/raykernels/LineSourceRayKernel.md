# LineSourceRayKernel

!syntax description /RayKernels/LineSourceRayKernel

While this summary describes the non-AD form available in `LineSourceRayKernel`, an AD-form is also available as the `ADLineSourceRayKernel`.

The strong form, given the line

!equation
L = \{\vec{r}_1 + t\vec{r}_2 \mid t \in [0, 1]\}\,

is defined as

!equation
\underbrace{-f(r')}_{\text{LineSourceRayKernel}} + \sum_{i=1}^n \beta_i(u,r) = 0\,,\quad r\in\Omega\,,\quad r'\in L

where $f$ is the source term (negative if a sink) along $L$ and the second term on the left hand side represents the strong forms of other kernels and boundary conditions. The `LineSourceRayKernel` weak form, in inner-product notation, is defined as

!equation
R_i(u_h) = (\psi_i, -f)_L \quad \forall ~ \psi_i,

where the $\psi_i$ are the test functions, and $u_h$ are the trial solutions in the finite dimensional space $\mathcal{S}^h$ for the unknown ($u$).

The Jacobian term for this kernel is zero: $\frac{\partial R_i(u_h)}{\partial u_j} = 0$, since it is assumed that $f$ +is not+ a function of the unknown $u$.

The force is constructed through a user supplied constant $c$, [function](/Functions/index.md) value evaluated at the current time and quadrature point $f$, [postprocessor](/Postprocessors/index.md) value $p$, and/or [Ray.md] data $r$. The constant $c$, supplied through the parameter [!param](/RayKernels/LineSourceRayKernel/value), may also be controlled over the course of a transient simulation with a [`Controls`](/Controls/index.md) block.  $c$, $f$, $p$ are supplied through the input parameters [!param](/RayKernels/LineSourceRayKernel/value), [!param](/RayKernels/LineSourceRayKernel/function), and [!param](/RayKernels/LineSourceRayKernel/postprocessor) respectively. $r$ is supplied through the input parameters [!param](/RayKernels/LineSourceRayKernel/ray_data_factor_names) and [!param](/RayKernels/LineSourceRayKernel/ray_aux_data_factor_names) as the product of the ray data with the given names. Not supplying $c$, $f$, $p$, or $r$ through its corresponding parameter is equivalent to setting its value to unity.

!syntax parameters /RayKernels/LineSourceRayKernel

!syntax inputs /RayKernels/LineSourceRayKernel

!syntax children /RayKernels/LineSourceRayKernel

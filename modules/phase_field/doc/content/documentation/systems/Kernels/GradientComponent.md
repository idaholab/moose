# GradientComponent

!syntax description /Kernels/GradientComponent

Implements the weak form residual
\begin{equation}
(u - \nabla_\alpha v, \psi),
\end{equation}
where $u$ is the kernel variable, $\alpha$ (`component`) is a coordinate system
direction, and $v$ is a coupled variable. This term effectively sets the value of the
kernel variable to the value of the selected component of the gradient of a coupled
variable.

!syntax parameters /Kernels/GradientComponent

!syntax inputs /Kernels/GradientComponent

!syntax children /Kernels/GradientComponent

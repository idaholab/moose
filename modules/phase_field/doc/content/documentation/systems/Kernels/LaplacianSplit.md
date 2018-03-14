# LaplacianSplit

!syntax description /Kernels/LaplacianSplit

Sets the kernel variable $u$ to the Laplacian of a chosen variable $c$ (`c`).
Implements the residual

$$
u - \nabla^2c.
$$

This allows the construction of split formulations, where $u$ can be substituted
to reduce the order of a PDE.

!syntax parameters /Kernels/LaplacianSplit

!syntax inputs /Kernels/LaplacianSplit

!syntax children /Kernels/LaplacianSplit

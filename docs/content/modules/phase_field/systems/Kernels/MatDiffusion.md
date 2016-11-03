!devel /Kernels/MatDiffusion float=right width=auto margin=20px padding=20px background-color=#F8F8F8

# MatDiffusion
!description /Kernels/MatDiffusion

Implements the term

$$
\nabla D \nabla c
$$

where the diffusion coefficient $D$ is provided by the [function material](../../introduction/FunctionMaterials) specified in `D_name`. $D$ can depend on arbitrary non-linear variables. The complete Jacobian contributions are provided by the kernel.

!parameters /Kernels/MatDiffusion

!inputfiles /Kernels/MatDiffusion

!childobjects /Kernels/MatDiffusion

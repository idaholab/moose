# 2D MBB Beam with a PDE Filter

In this example we will go over using a PDE filter instead of a convolution type
filter (see [!cite](pde_filter)). For larger problems this method may scale better depending on processor
counts and filter radius size. Only new material not covered in the previous
example will be covered here [2D Topology Optimization with a Convolution Filter](2d_mbb.md).

First there is a new variable `Dc` that will be the filtered sensitivity.

!listing examples/optimization/2d_mbb_pde.i
         block=Variables id=var_block
         caption=MBB `Variables` block

The `AuxVariables` block `sensitivity` is now used as a source
term for the PDE filter. There is also now a `Dc_elem` variable that will be
used for the density update.

!listing examples/optimization/2d_mbb_pde.i
         block=AuxVariables id=aux_var_block
         caption=MBB `AuxVariables` block

In the Kernel block the filtering is done using a `FunctionDiffusion` kernel,
`Reaction` Kernel, and a `CoupledForce` kernel. The function coefficient ($\lambda$) in the
`FunctionDiffusion` kernel is related to the radius ($r_{min}$) of the
`RadialAverage` filter by the equation $\lambda = \frac{r_{min}^2}{12}$.

!listing examples/optimization/2d_mbb_pde.i
         block=Kernels id=kernel_var_block
         caption=MBB `Kernels` block

One advantage of using a PDE filter is that by applying boundary conditions
on the sensitivity variable on the boundary of the domain the filter will
prevent "sticking" of the material commonly seen in topology optimization. That
penalty condition is applied using the `ADRobinBC` where the `coef` controls how
much the sensitivity is penalized on the boundary.

!listing examples/optimization/2d_mbb_pde.i
         block=BCs id=bc_var_block
         caption=MBB `BCs` block

The `UserObjects` block now only contains the `DensityUpdate` object.

!listing examples/optimization/2d_mbb_pde.i
         block=UserObjects id=uo_var_block
         caption=MBB `UserObjects` block

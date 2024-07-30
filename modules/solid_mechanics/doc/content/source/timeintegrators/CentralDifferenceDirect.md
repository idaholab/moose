# CentralDifferenceDirect

!syntax description /Executioner/TimeIntegrator/CentralDifferenceDirect

## Overview

`CentralDifferenceDirect` applies a time integrator for central difference in which the acceleration used for the solution update is calculated directly from the residual forces.

\begin{equation}
    \begin{aligned}
        \mathbf{a}_n &= \mathbf{M}^{-1}\mathbf{F}_n \\
        \mathbf{v}_{n+\frac{1}{2}} &= \mathbf{v}_{n-\frac{1}{2}} + \frac{\Delta t_{n-1}+\Delta t_n}{2}\mathbf{a}_n \\
        \mathbf{d}_{n+1} &= \mathbf{d}_{n-1} + \Delta t \mathbf{v}_{n+\frac{1}{2}}
    \end{aligned}
\end{equation}

When using Dirichlet BCs, one must use the ([DirectDirichletBC](source/bcs/DirectDirichletBC.md),[DirectFunctionDirichletBC](source/bcs/DirectFunctionDirichletBC.md)) variations to enforce BC's properly.

Additionally, the time integrator must be used with `MassMatrix`, with a properly tagged mass matrix.

## Example Input File Syntax

An example input file is shown below:

```

[GlobalParams]
    displacements = 'disp_x disp_y'
[]

[Kernels]
    [DynamicSolidMechanics]
        displacements = 'disp_x disp_y'
    []
    [massmatrix]
        type = MassMatrix
        density = 1
        matrix_tags = 'system'
        variable = disp_x
    []
    [massmatrix_y]
        type = MassMatrix
        density = 1
        matrix_tags = 'system'
        variable = disp_y
    []
[]

[Executioner]
    type = Transient

    [TimeIntegrator]
        type = CentralDifferenceDirect
        mass_matrix_tag = 'system'
    []
[]

```

!! Describe and include an example of how to use the CentralDifferenceDirect object.

!syntax parameters /Executioner/TimeIntegrator/CentralDifferenceDirect

!syntax inputs /Executioner/TimeIntegrator/CentralDifferenceDirect

!syntax children /Executioner/TimeIntegrator/CentralDifferenceDirect

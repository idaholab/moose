# DirectCentralDifference

!syntax description /Executioner/TimeIntegrator/DirectCentralDifference

## Overview

`DirectCentralDifference` applies a time integrator for central difference in which the acceleration used for the solution update is calculated directly from the residual forces.

\begin{equation}
    \begin{aligned}
        \mathbf{a}_n &= \mathbf{M}^{-1}\mathbf{F}_n \\
        \mathbf{v}_{n+\frac{1}{2}} &= \mathbf{v}_{n-\frac{1}{2}} + \frac{\Delta t_{n-1}+\Delta t_n}{2}\mathbf{a}_n \\
        \mathbf{d}_{n+1} &= \mathbf{d}_{n-1} + \Delta t \mathbf{v}_{n+\frac{1}{2}}
    \end{aligned}
\end{equation}

The formulation assumes a constant acceleration between midpoints. An average between the old and current time step is used to increment midpoint velocity to account for changing time steps, which is the same [method used in Abaqus](https://classes.engineering.wustl.edu/2009/spring/mase5513/abaqus/docs/v6.6/books/gsx/default.htm?startat=ch03s02.html).

For example if,
\begin{equation}
    \begin{aligned}
         t_{n-1}&=1\\
         t_{n}&=2\\
         t_{n+1}&=2.5
    \end{aligned}
\end{equation}
then,
\begin{equation}
    \begin{aligned}
         t_{n-\frac{1}{2}}+\frac{\Delta t_{n-1} + \Delta t_{n}}{2} = 1.5+\frac{1+0.5}{2} = 2.25 = t_{n+\frac{1}{2}}
    \end{aligned}
\end{equation}

When using Dirichlet BCs, one must use the ([DirectDirichletBC](source/bcs/DirectDirichletBC.md),[DirectFunctionDirichletBC](source/bcs/DirectFunctionDirichletBC.md)) variations to enforce Dirichlet BC's properly.

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

!! Describe and include an example of how to use the DirectCentralDifference object.

!syntax parameters /Executioner/TimeIntegrator/DirectCentralDifference

!syntax inputs /Executioner/TimeIntegrator/DirectCentralDifference

!syntax children /Executioner/TimeIntegrator/DirectCentralDifference

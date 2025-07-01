# ExplicitMixedOrder

!syntax description /Executioner/TimeIntegrator/ExplicitMixedOrder

## Overview

`ExplicitMixedOrder` applies a time integrator for central difference in which the acceleration used for the solution update is calculated directly from the residual forces.

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

When using Dirichlet BCs, one must use the ([ExplicitDirichletBC](source/bcs/ExplicitDirichletBC.md),[ExplicitFunctionDirichletBC](source/bcs/ExplicitFunctionDirichletBC.md)) variations to enforce Dirichlet BC's properly.

Additionally, the time integrator must be used with `MassMatrix`, with a properly tagged mass matrix. If both second and first time derivatives of a variable are present, they need to be distinguished by a tagged mass matrix and another different tagged damping matrix.

## Example Input File Syntax

An example input file is shown below. The file solves the equations:
\begin{equation}
    \begin{aligned}
        \ddot{disp_x} + \dot{disp_x} = 0\\
        \dot{disp_y}
    \end{aligned} = 0
\end{equation}

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
    [dampingmatrix]
        type = MassMatrix
        density = 0.8
        matrix_tags = 'damping'
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
        damping_matrix_tag = 'damping'
        second_order_vars = disp_x
        first_order_vars = disp_y
    []
[]

```

!! Describe and include an example of how to use the ExplicitMixedOrder object.

!syntax parameters /Executioner/TimeIntegrator/ExplicitMixedOrder

!syntax inputs /Executioner/TimeIntegrator/ExplicitMixedOrder

!syntax children /Executioner/TimeIntegrator/ExplicitMixedOrder

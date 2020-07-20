# PBP

!syntax description /Preconditioning/PBP

## Overview

Physics based preconditioning (PBP) is an advanced concept used to more efficiently solve using JFNK. The idea is to create a preconditioning process that targets each physics individually. In this way you can create a more effective preconditioner, while also maintaining efficiency. This object allows you to dial up a preconditioning matrix and the operations to be done on the different blocks of that matrix on the fly from the input file.

The PBP works by partially inverting a preconditioning matrix (usually an approximation of the true Jacobian) by partially inverting each block row in a Block-Gauss-Seidel way.

!equation
\boldsymbol{R}(u,v) =
  \begin{bmatrix}
    \boldsymbol{R}_u
    \\
    \boldsymbol{R}_v
  \end{bmatrix}

!equation
\boldsymbol{M} \equiv
\begin{bmatrix}
  (\boldsymbol{R}_u)_u & \boldsymbol{0}
  \\
  (\boldsymbol{R}_v)_u & (\boldsymbol{R}_v)_v
\end{bmatrix} \approx \boldsymbol{R}'

!equation
\boldsymbol{M} \boldsymbol{q} = \boldsymbol{p} \quad \Rightarrow \quad
\left\{
\begin{array}{rcl}
(\boldsymbol{R}_u)_u \boldsymbol{q}_u &=& \boldsymbol{p}_u \\[6pt]
(\boldsymbol{R}_v)_v \boldsymbol{q}_v &=& \boldsymbol{p}_v - (\boldsymbol{R}_v)_u \boldsymbol{q}_u
\end{array}
\right.

## Example Input File Syntax

Set up a PBP object for a two variable system (consisting of variables "u" and "v").
Use ILU for the "u" block and AMG for "v".
Use the lower diagonal (v,u) block.
When using `type = PBP`, MOOSE will set `solve_type = JFNK` automatically.

```
[Preconditioning]
  active = 'myPBP'

  [myPBP]
    type = PBP
    solve_order = 'u v'
    preconditioner  = 'ILU AMG'
    off_diag_row    = 'v'
    off_diag_column = 'u'
  []
[]
```

!syntax parameters /Preconditioning/PBP

!syntax inputs /Preconditioning/PBP

!syntax children /Preconditioning/PBP

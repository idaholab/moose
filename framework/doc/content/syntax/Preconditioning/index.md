# Preconditioning System

## Overview

See [Steady.md] for more details on how preconditioning is used in solving nonlinear systems in MOOSE.
The `Preconditioning` block allows you to define which type of preconditioning matrix to build and what process to apply.
You can define multiple blocks with different names, allowing you to quickly switch out preconditioning options.
Within the sub-blocks you can also provide other options specific to that type of preconditioning matrix.
You can also override PETSc options here.
Only one block can be active at a time.

## Default Preconditioning Matrix

Consider the fully coupled system of equations:

!equation
\begin{aligned}
\nabla \cdot k(s,T) \nabla T  &= 0 \\
\nabla \cdot D(s,T) \nabla s  &= 0 ,
\end{aligned},

the fully coupled Jacobian is then approximated using a block-diagonal approach:

!equation
\boldsymbol{R}'(s,T) =
 \begin{bmatrix}
   (\boldsymbol{R}_T)_T & (\boldsymbol{R}_T)_s
   \\
   (\boldsymbol{R}_s)_T & (\boldsymbol{R}_s)_s
 \end{bmatrix}
 \approx
 \begin{bmatrix}
   (\boldsymbol{R}_T)_T & \boldsymbol{0}
   \\
   \boldsymbol{0}       & (\boldsymbol{R}_s)_s
 \end{bmatrix} .

Thus, for this example, the default preconditioning matrix is defined as:

!equation
\boldsymbol{M} \equiv
    \begin{bmatrix}
      (k(s,T) \nabla \phi_j, \nabla \psi_i) & \boldsymbol{0} \\
      \boldsymbol{0} & (D(s,T) \nabla \phi_j, \nabla\psi_i)
    \end{bmatrix} \approx \boldsymbol{R}'(s,T) .

## Example Input File Syntax

!listing!
[Preconditioning]
  active = 'my_prec'

  [my_prec]
    type = SMP
    # SMP Options Go Here!
    # Override PETSc Options Here!
  []

  [other_prec]
    type = PBP
    # PBP Options Go Here!
    # Override PETSc Options Here!
  []
[]
!listing-end!

!syntax list /Preconditioning objects=True actions=False subsystems=False

!syntax list /Preconditioning objects=False actions=False subsystems=True

!syntax list /Preconditioning objects=False actions=True subsystems=False

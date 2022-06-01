# Step 9: Mechanics id=step09

!---

## Mechanics

Compute the elastic and thermal strain if the tube is only allowed to expand along the axial (y)
direction.

!equation
\begin{aligned}
\nabla \cdot (\boldsymbol{\sigma} + \boldsymbol{\sigma}_0) + \boldsymbol{b} =& \boldsymbol{0} \;\mathrm{in}\;\Omega \\
\boldsymbol{u} =& \boldsymbol{g}\;\mathrm{in}\;\Gamma_{ \boldsymbol{g}} \\
\boldsymbol{\sigma} \cdot \boldsymbol{n}=&\boldsymbol{t}\;\mathrm{in}\;\Gamma_{ \boldsymbol{t}}
\end{aligned}

where $\boldsymbol{\sigma}$  is the Cauchy stress tensor, $\boldsymbol{\sigma}_0$
is an additional source of stress (such as pore pressure), $\boldsymbol{u}$ is
the displacement vector, $\boldsymbol{b}$ is the body force, $\boldsymbol{n}$ is
the unit normal to the boundary, $\boldsymbol{g}$ is the prescribed displacement
on the boundary and $\boldsymbol{t}$ is the prescribed traction on the boundary.

!!end-intro

!---

## PackedColumn.h

!listing step09_mechanics/include/materials/PackedColumn.h

!---

## PackedColumn.C

!listing step09_mechanics/src/materials/PackedColumn.C

!---

## Step 9: Input File

!listing step09_mechanics/problems/step9.i

!---

## Step 9: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step09_mechanics
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step9.i
```

!---

## Step 9: Results

!media darcy_thermo_mech/step09_result.mp4

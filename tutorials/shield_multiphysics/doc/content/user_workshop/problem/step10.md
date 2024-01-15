# Step 10: Mechanics id=step10

!---

## Mechanics

Compute the elastic and thermal strain if the shield is only held on the ground surface

!equation
\begin{aligned}
\nabla \cdot (\boldsymbol{\sigma} + \boldsymbol{\sigma}_0) + \boldsymbol{b} =& \boldsymbol{0} \;\mathrm{in}\;\Omega \\
\boldsymbol{u} =& \boldsymbol{g}\;\mathrm{in}\;\Gamma_{ \boldsymbol{g}} \\
\boldsymbol{\sigma} \cdot \boldsymbol{n}=&\boldsymbol{t}\;\mathrm{in}\;\Gamma_{ \boldsymbol{t}}
\end{aligned}

where $\boldsymbol{\sigma}$  is the Cauchy stress tensor, $\boldsymbol{\sigma}_0$
is an additional source of stress, $\boldsymbol{u}$ is
the displacement vector, $\boldsymbol{b}$ is the body force, $\boldsymbol{n}$ is
the unit normal to the boundary, $\boldsymbol{g}$ is the prescribed displacement
on the boundary and $\boldsymbol{t}$ is the prescribed traction on the boundary.

!!end-intro

!---

## Step 10: Input File

!listing step10_mechanics/inputs/step10.i

!---

## Step 10: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step10_mechanics
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step10.i
```

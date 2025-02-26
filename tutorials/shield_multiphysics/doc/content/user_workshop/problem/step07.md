# Step 7: Mechanics id=step07

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

## Step 7: Input File

!listing step07_mechanics/step7.i

!---

## Step 7: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step07_mechanics
moose-opt -i step7.i
```

!---

## Step 7: Result

Displacements are exaggerated to be visible.

!media results/step07.png

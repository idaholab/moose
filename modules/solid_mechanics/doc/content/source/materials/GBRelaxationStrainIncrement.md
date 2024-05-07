# GBRelaxationStrainIncrement

!syntax description /Materials/GBRelaxationStrainIncrement

The strain increment from the relaxation of the grain boundaries $\boldsymbol{\epsilon}_{inc}$ is computed from the time step, the tensor of the normal to the grain boundaries $\boldsymbol{N}_{gb}$, and a user-specified pre-factor Real-value material property $f$.

!equation
\boldsymbol{\epsilon}_{inc} = f * dt * \boldsymbol{N}_{gb}

!syntax parameters /Materials/GBRelaxationStrainIncrement

!syntax inputs /Materials/GBRelaxationStrainIncrement

!syntax children /Materials/GBRelaxationStrainIncrement

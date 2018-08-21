# AllenCahnElasticEnergyOffDiag

!syntax description /Kernels/AllenCahnElasticEnergyOffDiag

Calculates the off-diagonal term

\begin{equation}
\frac{\partial\left({L\frac{\delta E}{\delta c}}\right)}{\partial{\mathbf{u}}} = \frac{\partial\left({L\frac{\delta E}{\delta c}}\right)}{\partial{\boldsymbol{\epsilon}}}\frac{\partial{\boldsymbol{\epsilon}}}{\partial{\mathbf{u}}}
\end{equation}

$E$ is the elastic energy, $\boldsymbol{\epsilon}$ is the strain. The first term on the right is a material property called 'd2Fdcdstrain'.

## Example Input File Syntax

!listing modules/combined/test/tests/phase_field_fracture/crack2d_aniso.i
         block=Kernels/off_disp

!syntax parameters /Kernels/AllenCahnElasticEnergyOffDiag

!syntax inputs /Kernels/AllenCahnElasticEnergyOffDiag

!syntax children /Kernels/AllenCahnElasticEnergyOffDiag

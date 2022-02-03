# ComputeNeoHookeanStress

!syntax description /Materials/ComputeNeoHookeanStress

## Overview

This class provides a hyperelastic Neo-Hookean stress update
defining the 2nd Piola-Kirchhoff stress as
\begin{equation}
      S_{IJ} = \left(\lambda \log J - \mu\right) C^{-1}_{IJ} + \mu \delta_{IJ}
\end{equation}
with
\begin{equation}
      C_{IJ} = 2 E_{IJ} + \delta_{IJ}
\end{equation}
where $E_{KL}$ is the Green-Lagrange strain.
The model is then defined in terms of two parameters, $\lambda$ and $mu$.

For small strains, when `large_kinematics = false` the model instead 
returns the standard linear elastic response, taking the $\lambda$
and $\mu$ parameters to be the Lame parameters defining a small strain
theory elastic model.

[example] shows a typical material response under uniaxial stress.  The 
plot shows the results in terms of the Cauchy stress and logarithmic 
strain to illustrate that the large deformation version of the
model is in fact nonlinear.

!media tensor_mechanics/neohookean.png
       id=example
       style=width:50%;float:center;padding-top:1.5%;
       caption=Response of the Neo-Hookean model to uniaxial deformation, plotted as log strain versus Cauchy stress.

## Example Input File Syntax

The follow example configures a large deformation Neo-Hookean model with $\lambda=4000$ and $\mu=6700$.

!listing modules/tensor_mechanics/test/tests/lagrangian/materials/correctness/neohookean.i
         block=Materials

!syntax parameters /Materials/ComputeNeoHookeanStress

!syntax inputs /Materials/ComputeNeoHookeanStress

!syntax children /Materials/ComputeNeoHookeanStress

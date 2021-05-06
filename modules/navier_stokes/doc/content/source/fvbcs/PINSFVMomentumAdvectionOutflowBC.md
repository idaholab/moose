# PINSFVMomentumAdvectionOutflowBC

!syntax description /FVBCs/PINSFVMomentumAdvectionOutflowBC

## Overview

This object implements the $\rho \dfrac{1}{\epsilon} \left(\vec u_d \cdot\nabla\right)\vec u_d$ component
term of the incompressible porous media Navier Stokes momentum equation along a domain boundary,
with $u_d$ the interstitial or Darcy velocity.
It simultaneously requires that the normal gradient of each superficial velocity component at
the boundary be zero.

!syntax parameters /FVBCs/PINSFVMomentumAdvectionOutflowBC

!syntax inputs /FVBCs/PINSFVMomentumAdvectionOutflowBC

!syntax children /FVBCs/PINSFVMomentumAdvectionOutflowBC

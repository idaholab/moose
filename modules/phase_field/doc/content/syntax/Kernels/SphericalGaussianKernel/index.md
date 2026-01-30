# SphericalGaussianKernel System

Refer to [SphericalGaussianKernelAction.md]

## Overview

This action sets up [ADGrainGrowth.md], [ADACInterface.md], [ADTimeDerivative.md], [EpsilonModelEpsilonGradientKernel.md], [EpsilonModelMGradientKernel.md], [GammaModelGammaGradientKernel.md] kernels based on the selected model ($\epsilon$-Model or $\gamma$-Model).

## Example Input File Syntax

A bicrystal input file is available for the $\epsilon$-Model:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/epsilon_model_bicrystal.i

A bicrystal input file is available for the $\gamma$-Model:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/gamma_model_bicrystal.i


!syntax list /Kernels/SphericalGaussianKernel objects=True actions=False subsystems=False

!syntax list /Kernels/SphericalGaussianKernel objects=False actions=False subsystems=True

!syntax list /Kernels/SphericalGaussianKernel objects=False actions=True subsystems=False

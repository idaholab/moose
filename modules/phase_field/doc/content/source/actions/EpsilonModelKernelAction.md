# EpsilonModelKernelAction

$\epsilon$-Model presented in [!cite](YEO2024127508)

## Overview

Set up ACGrGrPoly, ACInterface, TimeDerivative, ACGBPoly, EpsilonModelKernel1stGauss, EpsilonModelKernel1stV2Gauss, EpsilonModelKernel2ndGauss, EpsilonModelKernel2ndV2Gauss kernels.

## Example Input File Syntax

A bicrystal input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/test/tests/SphericalGaussian5DAnisotropyBicrystal/BicrystalEpsAndMAnisoGauss.i

A tricrystal input file is available.

!listing modules/phase_field/examples/SphericalGaussian5DAnisotropyTricrystal/TricrystalEpsAndMAnisoGauss.i


!syntax description /Kernels/EpsilonModelKernel/EpsilonModelKernelAction

!syntax parameters /Kernels/EpsilonModelKernel/EpsilonModelKernelAction

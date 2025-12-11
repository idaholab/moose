# SphericalGaussianKernelAction

Set up $\epsilon$-Model and $\gamma$-Model presented in [!cite](YEO2024127508)

## Overview

Early studies suggested that specific grain boundaries significantly influence the overall properties of materials, implying that material performance could be improved through intentional grain boundary engineering. For instance, twin boundaries have been shown to markedly enhance material properties. Given that grain boundaries possess five degrees of freedom and the current methods to incorporate both misorientation and inclination dependencies are either lacking or difficult to implement, further research and advanced modeling techniques are necessary.

A previously developed Spherical-Gaussian Method [!cite](BAIR2021110126) is used to incorporate 5-D anisotropy in two phase field models originally published by [!cite](MOELANS2022110592), here named $\epsilon$ and $\gamma$ models. Quaternions, assigned to individual grains as orientations and used to compute quaternion misorientations at each grain boundary, drive the ongoing mesoscale changes. As a means of incorporating the effects of specific low energy boundaries, such as twin boundaries, local minima/specific grain boundaries are stored as minima library for use in the phase field models through a developed gaussian switch ([!cite](Yeo2022) and [!cite](YEO2024127508)).

The $\epsilon$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\kappa$, $m$, and $L$.  

The $\gamma$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\gamma$ and $L$.

A material object, [SphericalGaussianMaterial.md], that adds anisotropy to grain boundary energy and mobility using the Spherical-Gaussian Method, was created as well as the kernels necessary to drive grain growth.

This action sets up [ADGrainGrowth.md], [ADACInterface.md], [ADTimeDerivative.md], [EpsilonModelEpsilonGradientKernel.md], [EpsilonModelMGradientKernel.md], [GammaModelGammaGradientKernel.md] kernels based on the selected model.

## Example Input File Syntax

A bicrystal input file is available for the $\epsilon$-Model:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/epsilon_model_bicrystal.i

A tricrystal input file is available for the $\epsilon$-Model:

!listing modules/phase_field/examples/spherical_gaussian_tricrystal/epsilon_model_tricrystal.i

A bicrystal input file is available for the $\gamma$-Model:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/gamma_model_bicrystal.i

A tricrystal input file is available for the $\gamma$-Model:

!listing modules/phase_field/examples/spherical_gaussian_tricrystal/gamma_model_tricrystal.i


!syntax parameters /Kernels/SphericalGaussianKernel/SphericalGaussianKernelAction

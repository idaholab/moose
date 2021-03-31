# INSFVMixingLengthScalarDiffusion

This kernel uses a mixing-length model to compute the turbulent diffusion of a
passive scalar, $\nabla \cdot \overline{ c' \vec u'}$, which appears in
Reynolds-averaged conservation equations.

Example passive scalars include energy and trace chemicals. In the case of
energy, the turbulent Schmidt number parameter is actually referred to as the
turbulent Prandtl number.

!syntax parameters /FVKernels/INSFVMixingLengthScalarDiffusion

!syntax inputs /FVKernels/INSFVMixingLengthScalarDiffusion

!syntax children /FVKernels/INSFVMixingLengthScalarDiffusion

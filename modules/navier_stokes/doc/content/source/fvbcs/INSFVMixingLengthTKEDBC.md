# INSFVMixingLengthTKEDBC

!syntax description /Materials/INSFVMixingLengthTKEDBC

This object wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md),
to impose a precomputed value for the turbulent kinetic energy dissipation rate.

The value set for the turbulent kinetic energy is:

$$\epsilon = C_{\mu}^{0.75} \frac{k^{\frac{3}{2}}}{0.07 L}$$,

where:
- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $L$ is a characterisitc length, e.g., the diameter of a pipe, diameter of an inlet jet, or the side length of the lid-driven cavity.

!syntax parameters /FVBCs/INSFVMixingLengthTKEDBC

!syntax inputs /FVBCs/INSFVMixingLengthTKEDBC

!syntax children /FVBCs/INSFVMixingLengthTKEDBC

# FunctionGradientNeumannBC

`FunctionGradientNeumannBC` adds the weak form contribution:
$\langle -\psi_i, \vec{n} \cdot k \nabla f \rangle$ where $\vec{n}$ is the
unit normal vector at the boundary, $k$ is a coefficient (corresponding to
thermal conductivity or diffusivity), and $f$ is a function of space and time
that is specified in the input file through the parameter `exact_solution`. As
the parameter name suggests, the `FunctionGradientNeumannBC` object can be
useful for specifying a Neumann condition when the exact solution to the PDE is
known.

!syntax description /BCs/FunctionGradientNeumannBC

!syntax parameters /BCs/FunctionGradientNeumannBC

!syntax inputs /BCs/FunctionGradientNeumannBC

!syntax children /BCs/FunctionGradientNeumannBC

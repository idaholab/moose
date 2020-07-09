# INSADMomentumCoupledForce

This object adds a term to the incompressible momentum equation directly
proportional to a coupled vector variable `coupled_vector_var` or a vector
function `vector_function`. Positive coupled variable or function components represent
momentum sources in that component direction, e.g. if the x-component is positive then this
object imposes a momentum source in the +x direction.

!syntax parameters /Kernels/INSADMomentumCoupledForce

!syntax inputs /Kernels/INSADMomentumCoupledForce

!syntax children /Kernels/INSADMomentumCoupledForce

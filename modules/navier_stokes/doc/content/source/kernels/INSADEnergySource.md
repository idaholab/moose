# INSADEnergySource

This object adds a heat source to the incompressible energy equation in the form
of a function with name set through the `source_function` parameter. Note that
if the function evaluates positive, this kernel is a source term. If the
function evaluates negative, then this kernel is a sink term.

!syntax description /Kernels/INSADEnergySource

!syntax parameters /Kernels/INSADEnergySource

!syntax inputs /Kernels/INSADEnergySource

!syntax children /Kernels/INSADEnergySource

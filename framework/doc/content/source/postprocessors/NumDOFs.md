# NumDOFs

The NumDOFs postprocessor provides information about the number of degrees of freedom (DOFs) in the simulation. This postprocessor
is capable of providing the DOFs in either the Nonlinear system (the system of PDEs you are solving), the Auxiliary system, the
system containing only explicit calculations, or both together.

If you are using [Mesh Adaptivity](syntax/Adaptivity/index.md), the number of DOFs will likely change as your simulation progresses.
You might consider using `execute_on = initial timestep_end` in that case. Otherwise `execute_on = initial` should be sufficient.

When scaling your problem up to more processor cores, try not to spread your problem out too much. A good target should be around
20,000 DOFs in your Nonlinear System. Please see this [PETSc FAQ](http://www.mcs.anl.gov/petsc/documentation/faq.html#slowerparallel) for more information.

## Description and Syntax

!syntax description /Postprocessors/NumDOFs

!syntax parameters /Postprocessors/NumDOFs

!syntax inputs /Postprocessors/NumDOFs

!syntax children /Postprocessors/NumDOFs

!bibtex bibliography

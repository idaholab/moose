# FVMatAdvectionOutflowBC

## Description

`FVMatAdvectionOutflowBC` adds an identical type of residual as
[`FVMatAdvection`](/FVMatAdvection.md) but as it is imposed on a boundary it's
only added to one side of a face. As the name implies this BC should be imposed
at an outflow boundary as it simply advects either a material property
optionally specified with the `advected_quantity` parameter or the variable upon
which the BC is applied out of the domain with a velocity specified through the
`vel` parameter.

!syntax parameters /FVBCs/FVMatAdvectionOutflowBC

!syntax inputs /FVBCs/FVMatAdvectionOutflowBC

!syntax children /FVBCs/FVMatAdvectionOutflowBC

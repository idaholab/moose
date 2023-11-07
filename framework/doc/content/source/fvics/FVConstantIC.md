# FVConstantIC

!syntax description /FVICs/FVConstantIC

Sets a constant initial condition described by parameter [!param](/FVICs/FVConstantIC/value). It can be restricted to particular blocks using the [!param](/FVICs/FVConstantIC/block) parameter.

## Example input syntax

In this example, a blockwise constant initial condition is set for variable `u`.
Block 1 and 2 are set differently by two `FVConstantIC` objects.

!listing test/tests/fvics/constant_ic/subdomain_constant_ic.i block=FVICs

!syntax parameters /FVICs/FVConstantIC

!syntax inputs /FVICs/FVConstantIC

!syntax children /FVICs/FVConstantIC

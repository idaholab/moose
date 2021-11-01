# ConstantIC

!syntax description /ICs/ConstantIC

Sets a constant initial condition described by parameter [!param](/ICs/ConstantIC/value). It can be restricted to particular blocks and boundaries using the [!param](/ICs/ConstantIC/block) and [!param](/ICs/ConstantIC/boundary) parameters, respectively.

## Example input syntax

In this example, constant initial conditions are set for variable `u` and auxiliary array variable `u_aux`. The values of `u_aux` in block 1 and 2 are set differently by two `ConstantIC` objects.

!listing test/tests/ics/constant_ic/subdomain_constant_ic_test.i block=ICs

!syntax parameters /ICs/ConstantIC

!syntax inputs /ICs/ConstantIC

!syntax children /ICs/ConstantIC

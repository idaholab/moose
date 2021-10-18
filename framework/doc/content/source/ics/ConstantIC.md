# ConstantIC

!syntax description /ICs/ConstantIC

This constant value may be set for the whole variable domain, or restricted to particular blocks or boundaries.

## Example input syntax

In this example, constant initial conditions are set for variable `u` and auxiliary array variable `u_aux`. The values of `u_aux` in block 1 and 2 are set differently by two `ConstantIC` objects.

!listing test/tests/ics/constant_ic/subdomain_constant_ic_test.i block=ICs

!syntax parameters /ICs/ConstantIC

!syntax inputs /ICs/ConstantIC

!syntax children /ICs/ConstantIC

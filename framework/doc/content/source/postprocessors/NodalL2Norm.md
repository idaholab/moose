# NodalL2Norm

!syntax description /Postprocessors/NodalL2Norm

!equation
||u||_{L_2} = \sqrt{\sum_{\text{nodes n}} u(n)^2}

## Example input syntax

In this example, the L2 norm of the variable `saved` is computed on
block 0 at the end of every time step using a `NodalL2Norm` postprocessor.

!listing test/tests/misc/save_in/diag_save_in_soln_var_err_test.i block=Postprocessors

!syntax parameters /Postprocessors/NodalL2Norm

!syntax inputs /Postprocessors/NodalL2Norm

!syntax children /Postprocessors/NodalL2Norm

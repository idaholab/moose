# VariableInnerProduct

!syntax description /Postprocessors/VariableInnerProduct

The inner product computed is for the L2 norm:

!equation
\langle u, v \rangle = \int_\Omega u(x, t) v(x, t) d\Omega

where $u$ and $v$ are the `variable` and `second_variable` parameters and $\Omega$ is the integration domain, either the whole domain or the specified `blocks`.

## Example input syntax

In this input file, we compute the inner products $\langle f,g \rangle$ and $<f,f>$ of auxiliary variables. We verify that $<f,f>$ is equal to the L2 norm.

!listing test/tests/postprocessors/variable_inner_product/variable_inner_product.i block=Postprocessors

!syntax parameters /Postprocessors/VariableInnerProduct

!syntax inputs /Postprocessors/VariableInnerProduct

!syntax children /Postprocessors/VariableInnerProduct

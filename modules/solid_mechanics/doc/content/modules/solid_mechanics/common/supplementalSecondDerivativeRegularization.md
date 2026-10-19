!alert warning title=Second-derivative FE data required
This kernel evaluates second derivatives of the trial and test functions. Use an FE
family and order for which libMesh provides meaningful second derivatives on the selected
elements; the example inputs use `family = LAGRANGE` and `order = SECOND`. Global $C^1$
continuity is not required by this kernel. On elements whose shape functions are linear,
such as `TRI3` and `TET4`, the second derivatives vanish and the kernel adds nothing.
Bilinear elements such as `QUAD4` keep the mixed derivative, so `huhu` still acts on them,
while `lulu` does not.

The included tests and intended displacement-regularization use case run on $C^0$
quadratic Lagrange elements. This is non-conforming for a standalone biharmonic solve,
but the kernel is used as a regularization contribution rather than as a conforming
biharmonic discretization. The cantilever test adds the regularization to both
displacement components of a plane-strain elastic beam under a tip shear load and shows
that it stiffens the bending response by an amount that depends on the regularization type.

In one dimension HuHu and LuLu are identical, so `huhu_lulu` with the default LuLu factor
of 1 adds nothing. Use `huhu` or `lulu` for one-dimensional problems.

!alert warning title=Second-derivative FE data required
This kernel evaluates second derivatives of the trial and test functions. Use an FE
family and order for which libMesh provides meaningful second derivatives on the selected
elements; the example inputs use `family = LAGRANGE` and `order = SECOND`. Global $C^1$
continuity is not required by this kernel, but variables whose second derivatives vanish
will produce little or no regularization contribution.

The included tests and intended displacement-regularization use case run on $C^0$
quadratic Lagrange elements. This is non-conforming for a standalone biharmonic solve,
but the kernel is used as a regularization contribution rather than as a conforming
biharmonic discretization. The test inputs pair the regularization kernel with a
`Diffusion` kernel, which keeps the $C^0$ quadratic test problem well posed independent
of the regularization contribution being tested.

The `huhu_lulu` option is rejected in one dimension when the default LuLu factor is used,
because HuHu and LuLu are identical in 1D and the default factor makes the combined term
vanish identically. Set `lulu_factor` explicitly, or use `huhu` or `lulu` directly, for
one-dimensional problems.

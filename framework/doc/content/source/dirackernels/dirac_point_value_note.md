## Developer Note: Dirac Point Values and Duplicate Handling

Every DiracKernel combines Dirac points that are within a fuzzy-comparison tolerance of one
another into a single geometric point. This combining of nearby points is performed internally by
`DiracKernelInfo` and always occurs, regardless of the `drop_duplicate_points` setting. However,
the +values+ at those combined points are not always combined — that behavior depends on
`drop_duplicate_points`.

The `drop_duplicate_points` parameter controls what happens during residual and Jacobian assembly:

- `drop_duplicate_points = true` (+default+): The accumulated values of the combined points are
  +ignored+ and only a single point's contribution from `computeQpResidual()` is used.
  Because the default is `true`, only a single point's contribution will be applied unless
  `drop_duplicate_points` is explicitly set to `false`.

- `drop_duplicate_points = false`: The values passed to `addPoint()` for coincident points are
  +accumulated+ onto the single combined geometric point. During assembly, this accumulated value
  is used as a +multiplier+ on `computeQpResidual()`. This is needed when multiple sources may
  coincide and each must contribute its own value.

Because of this design, a leaf class's `computeQpResidual()` does not return the final residual
contribution directly. It returns a kernel term (typically involving only test functions) that is
then scaled by the stored point value during assembly in `DiracKernel::computeResidual()`.

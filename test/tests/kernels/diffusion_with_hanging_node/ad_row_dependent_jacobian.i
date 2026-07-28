!include ad_simple_diffusion.i

[Kernels]
  active = row_dependent
  [row_dependent]
    type = ADRowDependentTestKernel
    variable = u
  []
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Outputs]
  exodus := false
[]

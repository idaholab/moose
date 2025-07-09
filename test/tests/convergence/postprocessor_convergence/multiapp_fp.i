!include ../parent.i

[Convergence]
  [fp_conv]
    type = PostprocessorConvergence
    postprocessor = nl_res
    tolerance = 1e-2
    max_iterations = 15
  []
[]

[Postprocessors]
  [nl_res]
    type = Residual
    residual_type = COMPUTE
    execute_on = 'MULTIAPP_FIXED_POINT_CONVERGENCE'
  []
[]

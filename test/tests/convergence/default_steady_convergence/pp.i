!include base.i

[Convergence]
  [steady_conv]
    type = PostprocessorConvergence
    postprocessor = var_diff_norm
    tolerance = ${ss_tol}
  []
[]

[Postprocessors]
  [var_diff_norm]
    type = RelativeSolutionDifferenceNorm
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

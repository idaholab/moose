# See the included input file. This should converge after step 2.
!include ../../postprocessors/change_over_time/time_derivative.i

[Convergence]
  [test_conv]
    type = PostprocessorSteadyStateConvergence
    postprocessor = dfdt_pp
    tolerance = 0.6
  []
[]

[Postprocessors]
  [num_steps]
    type = NumTimeSteps
  []
[]

[Executioner]
  steady_state_detection = true
  steady_state_convergence = test_conv
[]

[Outputs]
  file_base := steady_state
  show = 'num_steps'
  execute_on = 'FINAL'
[]

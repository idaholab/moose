# The SimplePredictor of predictor_test.i, with its change of the initial guess limited by a
# damper. In the second step the prediction changes u by up to 0.5; the MaxIncrement damper limits
# that change to 0.1 at the quadrature points, which scales the prediction back. The damper does
# not act on the nonlinear updates.

!include predictor_test.i

[Dampers]
  [cap]
    type = MaxIncrement
    variable = u
    max_increment = 0.1
    apply_to = predictor
  []
[]

[Outputs]
  file_base = predictor_damped_out
[]

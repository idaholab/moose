!include sub.i

[Executioner]
  [TimeStepper]
    type = FixedPointIterationAdaptiveDT
    dt_initial = 0.1
    target_iterations = 6
    target_window = 0
    increase_factor = 2.0
    decrease_factor = 0.5
  []
[]

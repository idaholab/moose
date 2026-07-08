!include formatting.i

[GlobalParams]
  absolute_value_vector_tags := 'first_ref second_ref'
[]

[Problem]
  extra_tag_vectors := 'first_ref; second_ref'
  nl_sys_names = 'first second'
[]

[Variables]
  [a]
    solver_sys = first
  []
  [b]
    solver_sys = first
  []
  [c]
    solver_sys = first
  []
  [d]
    solver_sys = first
  []
  [verylongvariable_e]
    solver_sys = second
  []
  [variable_f]
    solver_sys = first
  []
  [g]
    solver_sys = second
  []
[]

[Convergence]
  [conv_one]
    reference_vector := 'first_ref'
    solver_sys = first
  []
  [conv_two]
    reference_vector := 'second_ref'
    solver_sys = second
  []
[]

[Executioner]
  nonlinear_convergence := 'conv_one conv_two'
[]

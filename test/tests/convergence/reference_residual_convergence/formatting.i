[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[GlobalParams]
  absolute_value_vector_tags = 'ref'
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Variables]
  [a]
  []
  [b]
  []
  [c]
  []
  [d]
  []
  [verylongvariable_e]
  []
  [variable_f]
  []
  [g]
  []
[]

[Kernels]
  [a_force]
    type = BodyForce
    variable = a
    value = 1e2
  []
  [b_force]
    type = BodyForce
    variable = b
    value = 1e3
  []
  [c_force]
    type = BodyForce
    variable = c
    value = 1e4
  []
  [d_force]
    type = BodyForce
    variable = d
    value = 1e5
  []
  [e_force]
    type = BodyForce
    variable = verylongvariable_e
    value = 1e6
  []
  [f_force]
    type = BodyForce
    variable = variable_f
    value = 1e7
  []
  [g_force]
    type = BodyForce
    variable = g
    value = 1e8
  []

  [a_dt]
    type = TimeDerivative
    variable = a
  []
  [b_dt]
    type = TimeDerivative
    variable = b
  []
  [c_dt]
    type = TimeDerivative
    variable = c
  []
  [d_dt]
    type = TimeDerivative
    variable = d
  []
  [e_dt]
    type = TimeDerivative
    variable = verylongvariable_e
  []
  [f_dt]
    type = TimeDerivative
    variable = variable_f
  []
  [g_dt]
    type = TimeDerivative
    variable = g
  []
[]

[BCs]
  [a_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = a
  []
  [b_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = b
  []
  [c_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = c
  []
  [d_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = d
  []
  [e_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = verylongvariable_e
  []
  [f_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = variable_f
  []
  [g_bc]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = g
  []
[]

[Convergence]
  [conv_one]
    type = ReferenceResidualConvergence
    reference_vector = 'ref'
    group_variables = 'a b variable_f; c d'
    converge_on = 'a b variable_f c d g'
  []
  [conv_two]
    type = ReferenceResidualConvergence
    reference_vector = 'ref'
    converge_on = 'verylongvariable_e g'
  []
  [combined]
    type = ParsedConvergence
    convergence_expression = 'conv_one & conv_two'
    symbol_names = 'conv_one conv_two'
    symbol_values = 'conv_one conv_two'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nonlinear_convergence = combined
  verbose = true
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [a_avg]
    type = ElementAverageValue
    variable = a
  []
  [b_avg]
    type = ElementAverageValue
    variable = b
  []
  [c_avg]
    type = ElementAverageValue
    variable = c
  []
  [d_avg]
    type = ElementAverageValue
    variable = d
  []
  [e_avg]
    type = ElementAverageValue
    variable = verylongvariable_e
  []
  [f_avg]
    type = ElementAverageValue
    variable = variable_f
  []
[]

[Mesh]
  file = ../../mesh/square_quad9.e
[]

[AuxVariables]
  [sent_nodal]
    [InitialCondition]
      type = FunctionIC
      function = '3 + 2*x*x + 3*y*y*y'
    []
  []
  [received_nodal]
    initial_condition = -1
  []
  [sent_elem]
    family = MONOMIAL
    order = CONSTANT
    [InitialCondition]
      type = FunctionIC
      function = '4 + 2*x*x + 3*y*y*y'
    []
  []
  [received_elem]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
  hide = 'sent_nodal sent_elem'
  execute_on = 'TIMESTEP_END'
[]

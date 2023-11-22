[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
[]

[AuxVariables]
  [received_vector]
    family = LAGRANGE_VEC
    order = FIRST
  []

  [expected_vector_x]
    family = LAGRANGE
    order = FIRST
  []

  [expected_vector_y]
    family = LAGRANGE
    order = FIRST
  []

  [expected_vector_z]
    family = LAGRANGE
    order = FIRST
  []

  [received_vector_x]
    family = LAGRANGE
    order = FIRST
  []

  [received_vector_y]
    family = LAGRANGE
    order = FIRST
  []

  [received_vector_z]
    family = LAGRANGE
    order = FIRST
  []
[]

[ICs]
  # Set the expected components. If everything works, the received vector components should match.
  [set_expected_vector_x]
    type = FunctionIC
    variable = expected_vector_x
    function = "100*x*x"
  []

  [set_expected_vector_y]
    type = FunctionIC
    variable = expected_vector_y
    function = "100*y*y"
  []

  [set_expected_vector_z]
    type = FunctionIC
    variable = expected_vector_z
    function = "100*z*z"
  []
[]

[AuxKernels]
  # Set the components from the received vector.
  [set_received_vector_x]
    type = VectorVariableComponentAux
    vector_variable = received_vector
    variable = received_vector_x
    component = 'x'
    execute_on = timestep_begin
  []

  [set_received_vector_y]
    type = VectorVariableComponentAux
    vector_variable = received_vector
    variable = received_vector_y
    component = 'y'
    execute_on = timestep_begin
  []

  [set_received_vector_z]
    type = VectorVariableComponentAux
    vector_variable = received_vector
    variable = received_vector_z
    component = 'z'
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  [ensure_something_happened]
    type = ElementAverageValue
    variable = received_vector_x
  []

  # Compare the received vector against the expected components.
  [l2_difference_x]
    type = ElementL2Difference
    variable = received_vector_x
    other_variable = expected_vector_x
  []

  [l2_difference_y]
    type = ElementL2Difference
    variable = received_vector_y
    other_variable = expected_vector_y
  []

  [l2_difference_z]
    type = ElementL2Difference
    variable = received_vector_z
    other_variable = expected_vector_z
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  csv = true
[]

[Problem]
  solve = false
[]

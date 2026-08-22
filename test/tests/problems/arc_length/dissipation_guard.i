[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [secant_stiffness]
    type = ParsedFunction
    expression = 'if(t < 0.45, 1.0, 0.45)'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [restoring]
    type = ADMatBodyForce
    variable = u
    material_property = restoring_force
    function = secant_stiffness
    value = -1
  []
  [load]
    type = BodyForce
    variable = u
    value = 1
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []
[]

[Materials]
  [linear]
    type = ADParsedMaterial
    property_name = restoring_force
    coupled_variables = 'u'
    expression = 'u'
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.6
[]

[Postprocessors]
  [u_mid]
    type = PointValue
    variable = u
    point = '0.5 0 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [lambda]
    type = ArcLengthLoadParameter
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'u_mid'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 0.1
  num_steps = 10
  dtmin = 1e-3
[]

[Outputs]
  csv = true
[]

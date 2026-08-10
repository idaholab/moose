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

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [restoring]
    type = ADMatBodyForce
    variable = u
    material_property = restoring_force
    value = -1
  []
  [load]
    type = BodyForce
    variable = u
    value = 1
    vector_tags = 'arc_length_load'
  []
[]

[Materials]
  [cubic]
    type = ADParsedMaterial
    property_name = restoring_force
    coupled_variables = 'u'
    expression = 'u * (u - 1) * (u - 2)'
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.25
  lambda_min = -1
  lambda_max = 1
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
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]

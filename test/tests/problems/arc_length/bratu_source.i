[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
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
  [source]
    type = ADMatBodyForce
    variable = u
    material_property = bratu_source
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []
[]

[BCs]
  [fixed]
    type = ADPenaltyDirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
    penalty = 1e6
  []
[]

[Materials]
  [bratu]
    type = ADParsedMaterial
    property_name = bratu_source
    coupled_variables = 'u'
    expression = 'exp(u)'
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.2
  lambda_max = 6
[]

[Postprocessors]
  [u_center]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [lambda]
    type = ArcLengthLoadParameter
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'u_center'
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

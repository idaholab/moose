[Mesh]
  type = GeneratedMesh
  dim = 3
  xmax = 1.5
  ymax = 1.5
  zmax = 1.2
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [np_layered_average]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [np_layered_average]
    type = SpatialUserObjectAux
    variable = np_layered_average
    execute_on = timestep_end
    user_object = npla
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [one]
    type = DirichletBC
    variable = u
    boundary = 'right back top'
    value = 1
  []
[]

[UserObjects]
  [npla]
    type = NearestPointLayeredAverage
    direction = y
    num_layers = 3
    variable = u
    points = '0.375 0.0 0.3
              1.125 0.0 0.3
              0.375 0.0 0.9
              1.125 0.0 0.9'
  []
[]

[VectorPostprocessors]
  # getting the points from the user object itself is here exactly equivalent to the points
  # provided in the 'spatial_manually_provided' vector postprocessor
  [spatial_from_uo]
    type = SpatialUserObjectVectorPostprocessor
    userobject = npla
  []
  [spatial_manually_provided]
    type = SpatialUserObjectVectorPostprocessor
    userobject = npla
    points = '0.375 0.25 0.3
              0.375 0.75 0.3
              0.375 1.25 0.3

              1.125 0.25 0.3
              1.125 0.75 0.3
              1.125 1.25 0.3

              0.375 0.25 0.9
              0.375 0.75 0.9
              0.375 1.25 0.9

              1.125 0.25 0.9
              1.125 0.75 0.9
              1.125 1.25 0.9'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

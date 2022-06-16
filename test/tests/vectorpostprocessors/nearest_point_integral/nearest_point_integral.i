[Mesh]
  type = GeneratedMesh
  dim = 3
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
    user_object = npi
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1.5
  []
  [one]
    type = DirichletBC
    variable = u
    boundary = 'right back top'
    value = 1
  []
[]

[VectorPostprocessors]
  [npi]
    type = NearestPointIntegralVariablePostprocessor
    variable = u
    points = '0.25 0.25 0.25
              0.75 0.25 0.25
              0.25 0.75 0.75
              0.75 0.75 0.75'
  []

  # getting the points from the user object itself is here exactly equivalent to the points
  # provided in the 'spatial_manually_provided' vector postprocessor
  [spatial_from_uo]
    type = SpatialUserObjectVectorPostprocessor
    userobject = npi
  []
  [spatial_manually_provided]
    type = SpatialUserObjectVectorPostprocessor
    userobject = npi
    points = '0.25 0.25 0.25
              0.75 0.25 0.25
              0.25 0.75 0.75
              0.75 0.75 0.75'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = final
[]

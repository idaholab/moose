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
  [./u]
  [../]
[]

[AuxVariables]
  [./np_layered_average]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./np_layered_average]
    type = SpatialUserObjectAux
    variable = np_layered_average
    execute_on = timestep_end
    user_object = npla
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./one]
    type = DirichletBC
    variable = u
    boundary = 'right back top'
    value = 1
  [../]
[]

[UserObjects]
  [./npla]
    type = NearestPointLayeredIntegral
    direction = y
    num_layers = 10
    variable = u
    points_file = points.txt
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 8
  xmax = 0.1
  ymax = 0.5
  coord_type = rz
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
[]

[AuxVariables]
  [./layered_average_value]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./axial_force]
    type = ParsedFunction
    value = 1000*y
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    function = axial_force
  [../]
[]

[AuxKernels]
  [./layered_aux]
    type = SpatialUserObjectAux
    variable = layered_average_value
    execute_on = timestep_end
    user_object = layered_average
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[UserObjects]
  [./layered_average]
    type = LayeredAverage
    variable = u
    direction = y
    num_layers = 4
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.001

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

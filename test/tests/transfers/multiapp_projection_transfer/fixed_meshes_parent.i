[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 5
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_sub]
  [../]
  [./elemental_from_sub]
    order = CONSTANT
    family = MONOMIAL
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
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.01
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  #
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.0 0.0 0'
    input_files = fixed_meshes_sub.i
  [../]
[]

[Transfers]
  [./from_sub]
    type = MultiAppProjectionTransfer
    from_multi_app = sub
    source_variable = u
    variable = from_sub
    fixed_meshes = true
  [../]
  [./elemental_from_sub]
    type = MultiAppProjectionTransfer
    from_multi_app = sub
    source_variable = u
    variable = elemental_from_sub
    fixed_meshes = true
  [../]
  [./to_sub]
    type = MultiAppProjectionTransfer
    to_multi_app = sub
    source_variable = u
    variable = from_parent
    fixed_meshes = true
  [../]
  [./elemental_to_sub]
    type = MultiAppProjectionTransfer
    to_multi_app = sub
    source_variable = u
    variable = elemental_from_parent
    fixed_meshes = true
  [../]
[]


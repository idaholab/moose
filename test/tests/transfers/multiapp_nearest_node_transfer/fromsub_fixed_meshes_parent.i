[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.01
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.1 0.45 0'
    input_files = fromsub_fixed_meshes_sub.i
  [../]
[]

[Transfers]
  # Note: it's not generally advised to use "fixed_meshes = true" with displaced
  # meshes.  We only do that for this test to make sure the test will fail if
  # "fixed_meshes" isn't working properly.
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = from_sub
    fixed_meshes = true
    displaced_source_mesh = true
  [../]
  [./elemental_from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = elemental_from_sub
    fixed_meshes = true
    displaced_source_mesh = true
  [../]
[]


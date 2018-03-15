[Mesh]
  type = FileMesh
  file = patterned_mesh_in.e
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = MatCoefDiffusion
    variable = u
    conductivity = conductivity
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  [./mat1]
    type = GenericConstantMaterial
    block = 1
    prop_names = conductivity
    prop_values = 100
  [../]
  [./mat2]
    type = GenericConstantMaterial
    block = 2
    prop_names = conductivity
    prop_values = 1e-4
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]


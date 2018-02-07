[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./diff_x]
    type = Diffusion
    variable = disp_x
  [../]
  [./diff_y]
    type = Diffusion
    variable = disp_y
  [../]
  [./diff_z]
    type = Diffusion
    variable = disp_z
  [../]
  [./conv_x]
    type = Convection
    variable = disp_x
    velocity = '2 0 0'
  [../]
  [./conv_y]
    type = Convection
    variable = disp_y
    velocity = '2 0 0'
  [../]
  [./conv_z]
    type = Convection
    variable = disp_z
    velocity = '2 0 0'
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 1
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  [../]
  [./right_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 1
  [../]
  [./left_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  [../]
  [./right_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [./out]
    type = Exodus
    output_dimension = 3
  [../]
[]

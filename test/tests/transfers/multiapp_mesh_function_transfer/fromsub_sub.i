[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = -.01
  xmax = 0.21
  ymin = -.01
  ymax = 0.21
  displacements = 'x_disp y_disp'
[]

[Variables]
  [./sub_u]
  [../]
[]

[AuxVariables]
  [./x_disp]
    initial_condition = 0.2
  [../]
  [./y_disp]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = sub_u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = sub_u
    boundary = left
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = sub_u
    boundary = right
    value = 4
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

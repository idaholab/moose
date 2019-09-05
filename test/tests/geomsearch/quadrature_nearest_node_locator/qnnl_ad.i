[Mesh]
  file = 2dcontact_collide.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./distance]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [disp_x][]
  [disp_y][]
[]

[Kernels]
  [./diff]
    type = ADDiffusion
    variable = u
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./distance]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

[]

[Outputs]
  exodus = true
  file_base = qnnl_ad
[]

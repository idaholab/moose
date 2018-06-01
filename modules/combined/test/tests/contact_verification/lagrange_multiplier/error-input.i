[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./lm]
  [../]
[]

[Constraints]
  [./lm]
    type = LMConstraint
    slave = 0
    master = 0
    variable = lm
    master_variable = disp_x
    disp_y = disp_y
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]

[Contact]
  [./leftright]
    master = 0
    slave = 0
    model = frictionless
    formulation = lagrange
    # penalty = 1e6
    system = constraint
  [../]
[]

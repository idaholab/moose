[Mesh]
  file = 2-lines.e
  displacements = 'disp_x'
  allow_renumbering = false
[]

[AuxVariables]
  [./disp_x]
  [../]
[]

[AuxKernels]
  [./disp_x_ak]
    type = ConstantAux
    variable = disp_x
    value = 1
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 3
  [../]
[]

[Constraints]
  [./c1]
    type = EqualValueNodalConstraint
    variable = u
    primary = 0
    secondary = 4
    penalty = 100000
    use_displaced_mesh = true
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

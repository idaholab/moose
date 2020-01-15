[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [./u_x]
  [../]
  [./u_y]
  [../]
[]

[Kernels]
  [./diff_x]
    type = CoefDiffusion
    variable = u_x
    coef = 0.1
  [../]
  [./diff_y]
    type = CoefDiffusion
    variable = u_y
    coef = 0.1
  [../]
[]

[NodalKernels]
  [./test_y]
    type = JacobianCheck
    variable = u_y
  [../]
  [./test_x]
    type = JacobianCheck
    variable = u_x
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = u_x
    preset = false
    boundary = left
    value = 0
  [../]
  [./right_x]
    type = DirichletBC
    variable = u_x
    preset = false
    boundary = right
    value = 1
  [../]
  [./left_y]
    type = DirichletBC
    variable = u_y
    preset = false
    boundary = left
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
  solve_type = NEWTON
#  petsc_options = '-snes_check_jacobian -snes_check_jacobian_view'
  nl_max_its = 1
  nl_abs_tol = 1e0
[]

[Outputs]
  exodus = true
[]

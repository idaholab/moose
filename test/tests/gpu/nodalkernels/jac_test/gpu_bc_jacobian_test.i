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

[GPUKernels]
  [./diff_x]
    type = GPUCoefDiffusion
    variable = u_x
    coef = 0.1
  [../]
  [./diff_y]
    type = GPUCoefDiffusion
    variable = u_y
    coef = 0.1
  [../]
[]

[GPUNodalKernels]
  [./test_y]
    type = GPUJacobianCheck
    variable = u_y
    boundary = top
  [../]
  [./test_x]
    type = GPUJacobianCheck
    variable = u_x
    boundary = top
  [../]
[]

[GPUBCs]
  [./left_x]
    type = GPUDirichletBC
    variable = u_x
    preset = false
    boundary = left
    value = 0
  [../]
  [./right_x]
    type = GPUDirichletBC
    variable = u_x
    preset = false
    boundary = right
    value = 1
  [../]
  [./left_y]
    type = GPUDirichletBC
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
# petsc_options = '-snes_check_jacobian -snes_check_jacobian_view'
  nl_max_its = 1
  nl_abs_tol = 1e0
[]

[Outputs]
  exodus = true
[]

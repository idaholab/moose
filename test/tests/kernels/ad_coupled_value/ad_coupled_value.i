[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  elem_type = quad9
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
  [./w]
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./diff_w]
    type = Diffusion
    variable = w
  [../]
  [./ad_coupled_value]
    type = ADCoupledValueTest
    variable = u
    v = v
  [../]
  [./ad_coupled_value_w]
    type = ADCoupledValueTest
    variable = u
    v = w
  [../]
  [./ad_coupled_value_x]
    type = ADCoupledValueTest
    variable = u
    # v = 2.0 (Using the default value)
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
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  [../]
  [./left_w]
    type = DirichletBC
    variable = w
    boundary = left
    value = 0
  [../]
  [./right_w]
    type = DirichletBC
    variable = w
    boundary = right
    value = 1
  [../]
[]

[Preconditioning]
  active = ''
  [./smp]
    type = SMP
  [../]
[]


[Executioner]
  type = Steady

  solve_type = 'Newton'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_tol = 1e-10
  nl_rel_tol = 1e-9
  nl_max_its = 1
[]

[Outputs]
  exodus = true
[]

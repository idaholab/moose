[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./diff]
    type = ADDiffusion
    variable = u
  [../]
  [./convection]
    type = ADCoupledConvection
    variable = u
    velocity_vector = v
  [../]
  [./diff_v]
    type = ADDiffusion
    variable = v
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
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'Newton'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_tol = 1e-10
  nl_rel_tol = 1e-9
  nl_max_its = 2
[]

[Outputs]
  exodus = true
[]

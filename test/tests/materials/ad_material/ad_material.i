[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
[]

[Kernels]
  [./diff]
    type = ADMatDiffusion
    variable = u
    prop_name = diffusivity
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
[]

[Materials]
  [./ad_coupled_mat]
    type = ADCoupledMaterial
    coupled_var = u
    block = 0
    mat_prop = diffusivity
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'Newton'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_tol = 1e-10
  nl_rel_tol = 1e-9
[]

[Outputs]
  exodus = true
[]

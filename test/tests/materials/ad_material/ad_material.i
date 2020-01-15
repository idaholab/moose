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
    type = ADMatDiffusionTest
    variable = u
    prop_to_use = 'AdAd'
    ad_mat_prop = ad_diffusivity
    regular_mat_prop = regular_diffusivity
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./ad_coupled_mat]
    type = ADCoupledMaterial
    coupled_var = u
    ad_mat_prop = ad_diffusivity
    regular_mat_prop = regular_diffusivity
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

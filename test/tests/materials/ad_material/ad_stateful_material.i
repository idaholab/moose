[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
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
    ad_mat_prop = 'diffusivity'
    regular_mat_prop = 'unused_diffusivity'
  [../]
[]

[Kernels]
  [./force]
    type = BodyForce
    variable = u
    value = 1
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
  [./constant_material]
    type = GenericConstantMaterial
    prop_names = 'unused_diffusivity'
    prop_values = '0'
  [../]
  [./ad_stateful]
    type = ADStatefulMaterial
    u = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  line_search = 'none'

  solve_type = 'Newton'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_tol = 1e-10
  nl_rel_tol = 1e-9
[]

[Outputs]
  [./exodus]
    type = Exodus
    show_material_properties = 'diffusivity'
  [../]
[]

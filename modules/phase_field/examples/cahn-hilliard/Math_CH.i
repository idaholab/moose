[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmax = 60
  ymax = 60
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = RandomIC
      min = -0.1
      max =  0.1
    [../]
  [../]
[]

[Kernels]
  [./c_dot]
    type = TimeDerivative
    variable = c
  [../]
  [./CHbulk]
    type = CHMath
    variable = c
  [../]
  [./CHint]
    type = CHInterface
    variable = c
    mob_name = M
    kappa_name = kappa_c
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names  = 'M   kappa_c'
    prop_values = '1.0 0.5'
  [../]
[]

[Postprocessors]
  [./top]
    type = SideIntegralVariablePostprocessor
    variable = c
    boundary = top
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  scheme = bdf2

  # Preconditioning using the additive Schwartz method and LU decomposition
  petsc_options_iname = '-pc_type -sub_ksp_type -sub_pc_type'
  petsc_options_value = 'asm      preonly       lu          '

  # Alternative preconditioning options using Hypre (algebraic multi-grid)
  #petsc_options_iname = '-pc_type -pc_hypre_type'
  #petsc_options_value = 'hypre    boomeramg'

  l_tol = 1e-4
  l_max_its = 30

  dt = 2.0
  end_time = 80.0
[]

[Outputs]
  exodus = true
  perf_graph = true
[]

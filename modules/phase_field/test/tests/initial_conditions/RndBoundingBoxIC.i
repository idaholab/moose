[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 8
  xmax = 50
  ymax = 25
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./InitialCondition]
    type = RndBoundingBoxIC
    x1 = 15.0
    x2 = 35.0
    y1 = 0.0
    y2 = 25.0
    mx_invalue = 1.0
    mn_invalue = 0.9
    mx_outvalue = -0.7
    mn_outvalue = -0.8
    variable = c
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]
  [./CHSolid]
    type = CHMath
    variable = c
    mob_name = M
  [../]
  [./CHInterface]
    type = CHInterface
    variable = c
    kappa_name = kappa_c
    mob_name = M
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
  [./constant]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1.0 1.0'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 15
  nl_max_its = 10

  start_time = 0.0
  num_steps = 4
  dt = 5.0
[]

[Outputs]
  exodus = true
[]

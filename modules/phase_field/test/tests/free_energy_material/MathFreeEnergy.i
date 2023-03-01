[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  xmax = 50
  ymax = 50
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./c]
    type = SmoothCircleIC
    variable = c
    x1 = 25.0
    y1 = 25.0
    radius = 6.0
    invalue = 1.0
    outvalue = -0.8
    int_width = 4.0
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]
  [./CHSolid]
    type = CahnHilliard
    variable = c
    mob_name = M
    f_name = F
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
  [./free_energy]
    type = MathFreeEnergy
    property_name = F
    c = c
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 20
  l_tol = 1.0e-5
  nl_max_its = 40
  nl_rel_tol = 5.0e-14

  start_time = 0.0
  num_steps = 1
  dt = 2.0
[]

[Outputs]
  execute_on = 'timestep_end'
  [./oversample]
    type = Exodus
    refinements = 2
  [../]
[]

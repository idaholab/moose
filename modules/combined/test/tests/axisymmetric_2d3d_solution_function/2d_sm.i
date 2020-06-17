[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = 2d.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./temp]
    initial_condition = 400
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hoop_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./temp_inner_func]
    type = PiecewiseLinear
    xy_data = '0 400
               1 350'
  [../]
  [./temp_outer_func]
    type = PiecewiseLinear
    xy_data = '0 400
               1 400'
  [../]
  [./press_func]
    type = PiecewiseLinear
    xy_data = '0 15
               1 15'
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
    temp = temp
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
  [./vonmises_stress]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises_stress
    quantity = vonmises
  [../]
  [./hoop_stress]
    type = MaterialTensorAux
    tensor = stress
    variable = hoop_stress
    quantity = hoop
    execute_on = timestep_end
  [../]
  [./hydrostatic_stress]
    type = MaterialTensorAux
    tensor = stress
    variable = hydrostatic_stress
    quantity = hydrostatic
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = '1'
    value = 0.0
  [../]

  [./Pressure]
    [./internal_pressure]
      boundary = '4'
      factor = 1.e6
      function = press_func
    [../]
  [../]

  [./t_in]
    type = FunctionDirichletBC
    variable = temp
    boundary = '4'
    function = temp_inner_func
  [../]

  [./t_out]
    type = FunctionDirichletBC
    variable = temp
    boundary = '2'
    function = temp_outer_func
  [../]
[]

[Constraints]
  [./disp_y]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    master = '65'
    secondary = '3'
    penalty = 1e18
  [../]
[]

[Materials]
  [./thermal1]
    type = HeatConductionMaterial
    block = '1'
    thermal_conductivity = 25.0
    specific_heat = 490.0
    temp = temp
  [../]

  [./solid_mechanics1]
    type = Elastic
    block = '1'
    disp_r = disp_x
    disp_z = disp_y
    temp = temp
    youngs_modulus = 193.05e9
    poissons_ratio = 0.3
    thermal_expansion = 13e-6
    stress_free_temperature = 295.00
    formulation = NonlinearRZ
  [../]

  [./density]
    type = Density
    block = '1'
    density = 8000.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
  line_search = 'none'
  l_max_its = 25
  nl_max_its = 20
  nl_rel_tol = 1e-9
  l_tol = 1e-2

  start_time = 0.0
  dt = 1
  end_time = 1
  dtmin = 1
[]

[Outputs]
  file_base = 2d_out
  exodus = true
  [./console]
    type = Console
    max_rows = 25
  [../]
[]

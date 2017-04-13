#
# Test to exercise the exponential stress release
#
# First x, then y, then z directions crack
#

[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = cracking_test.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]

  [./disp_x]
  [../]

  [./disp_y]
  [../]

  [./disp_z]
  [../]

[]

[AuxVariables]
  [./crack_1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_3]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

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

  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

[Functions]
  [./displx]
    type = PiecewiseLinear
#   x = '0 1'
#   y = '0 .0035'
    x = '0 1'
    y = '0 0.00175'
  [../]
  [./velocity_y]
    type = ParsedFunction
    value = 'if(t < 2, 0.00175, 0)'
  [../]
  [./velocity_z]
    type = ParsedFunction
    value = 0.00175
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]


[AuxKernels]
  [./crack_1]
    type = MaterialRealVectorValueAux
    variable = crack_1
    property = crack_flags
    component = 0
  [../]
  [./crack_2]
    type = MaterialRealVectorValueAux
    variable = crack_2
    property = crack_flags
    component = 1
  [../]
  [./crack_3]
    type = MaterialRealVectorValueAux
    variable = crack_3
    property = crack_flags
    component = 2
  [../]

  [./strain_xx]
    type = MaterialTensorAux
    variable = strain_xx
    tensor = total_strain
    index = 0
  [../]
  [./strain_yy]
    type = MaterialTensorAux
    variable = strain_yy
    tensor = total_strain
    index = 1
  [../]
  [./strain_zz]
    type = MaterialTensorAux
    variable = strain_zz
    tensor = total_strain
    index = 2
  [../]

  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    tensor = stress
    index = 0
  [../]

  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]

  [./stress_zz]
    type = MaterialTensorAux
    variable = stress_zz
    tensor = stress
    index = 2
  [../]

  [./stress_xy]
    type = MaterialTensorAux
    variable = stress_xy
    tensor = stress
    index = 3
  [../]

  [./stress_yz]
    type = MaterialTensorAux
    variable = stress_yz
    tensor = stress
    index = 4
  [../]

  [./stress_zx]
    type = MaterialTensorAux
    variable = stress_zx
    tensor = stress
    index = 5
  [../]
[]


[BCs]
  [./fix_x]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./move_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 4
    function = displx
  [../]

  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./move_y]
    type = PresetVelocity
    variable = disp_y
    boundary = 5
    function = velocity_y
#    time_periods = 'p2 p3'
  [../]

  [./fix_z]
    type = PresetBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]
  [./move_z]
    type = PresetVelocity
    variable = disp_z
    boundary = 6
    function = velocity_z
#    time_periods = 'p3'
  [../]
[]

[Materials]
  [./fred]
    type = Elastic
    block = 1
    youngs_modulus = 186.5e9
    poissons_ratio = .316
    cracking_stress = 119.3e6
    cracking_release = exponential

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    formulation = linear
  [../]
[]

#[Preconditioning]
#  [./SMP]
#    type = SMP
#    full = true
#  []
#[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'


  line_search = 'none'


  l_max_its = 100
  l_tol = 1e-6

  nl_max_its = 100
  nl_abs_tol = 1e-8
  #nl_rel_tol = 1e-3
  nl_rel_tol = 1e-6

  start_time = 0.0
  end_time = 3.0
#  dt = 0.005
  dt = 0.01
[]

[Controls]
  [./p1]
    type = TimePeriod
    start_time = 0.0
    end_time = 1.0
    disable_objects = 'move_y move_z'
    set_outside_of_range = false
    execute_on = 'initial timestep_begin'
  [../]

  [./p2]
    type = TimePeriod
    start_time = 1.0
    end_time = 2.0
    disable_objects = 'move_z'
    enable_objects = 'move_y'
    set_outside_of_range = false
    execute_on = 'initial timestep_begin'
  [../]

  [./p3]
    type = TimePeriod
    start_time = 2.0
    end_time = 3.0
    enable_objects = 'move_y move_z'
    set_outside_of_range = false
    execute_on = 'initial timestep_begin'
    set_sync_times = true
  [../]
[]

[Outputs]
  exodus = true
[]

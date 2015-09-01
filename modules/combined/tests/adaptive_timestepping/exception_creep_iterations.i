# 1x1x1 unit cube with uniform pressure on top face

[Mesh]
  file = 1x1x1_cube.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  active = 'disp_x disp_y disp_z temp'

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1000.0
  [../]
 []

[AuxVariables]
  active = 'stress_yy creep_strain_xx creep_strain_yy creep_strain_zz elastic_strain_yy'

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./creep_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear

    x = '0    1'
    y = '2e-3 2e-3'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Kernels]
  [./solid_x_ie]
    type = SolidMechImplicitEuler
    variable = disp_x
  [../]

  [./solid_y_ie]
    type = SolidMechImplicitEuler
    variable = disp_y
  [../]

  [./solid_z_ie]
    type = SolidMechImplicitEuler
    variable = disp_z
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]
[]


[AuxKernels]

  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]

  [./creep_strain_xx]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_xx
    index = 0
  [../]

  [./creep_strain_yy]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_yy
    index = 1
  [../]

  [./creep_strain_zz]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_zz
    index = 2
  [../]

  [./elastic_strain_yy]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_yy
    index = 1
  [../]

 []


[BCs]
  [./u_top_pull]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 5
    function = top_pull
  [../]

  [./u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./u_x_fix]
    type = DirichletBC
    variable = disp_x
    boundary = '3 5'
    value = 0.0
  [../]

  [./u_z_fix]
    type = DirichletBC
    variable = disp_z
    boundary = '3 5'
    value = 0.0
  [../]

  [./temp_top_fix]
    type = DirichletBC
    variable = temp
    boundary = 5
    value = 1000.0
  [../]

  [./temp_bottom_fix]
    type = DirichletBC
    variable = temp
    boundary = 3
    value = 1000.0
  [../]

[]

[Materials]

  [./creep]
    type = PowerLawCreep
    block = 1
    youngs_modulus = 2.e11
    poissons_ratio = .3
    coefficient = 1.0e-15
    exponent = 4
    activation_energy = 3.0e5
    tolerance = 1.e-5
    max_its = 15
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
    output_iteration_info = false
  [../]

  [./thermal]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 1.0
    thermal_conductivity = 100.
  [../]

  [./density]
    type = Density
    block = 1
    density = 1.0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]

[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  verbose = true
  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'


  line_search = 'none'


  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  l_tol = 1e-5
  start_time = 0.0
  end_time = 1.0
  num_steps = 100
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1.0
  [../]
[]
[Outputs]
  file_base = out_except_creep
  csv = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

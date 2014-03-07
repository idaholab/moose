#
# Plenum Pressure Test
#
# This test is designed to compute an internal pressure based on
#   p = n * R * T / V
# where
#   p is the pressure
#   n is the amount of material in the volume (moles)
#   R is the universal gas constant
#   T is the temperature
#   V is the volume
#
# The mesh is composed of one block (1) with an interior cavity of volume 8.
#   Block 2 sits in the cavity and has a volume of 1.  Thus, the total
#   initial volume is 7.
#
# The test includes a refabrication step in which the amount of gas is reset.
#   n = n0 + alpha*t
#   T = 70
#   V = 7
# with
#   alpha = n0
#   n0 = 1/R
#   R = 8.314472 J* K^(−1)*mol^(−1)
#
# So, p0 = n*R*T/V = 1/R * R * 70 / 7
#        = 10
# and p(1) = 20.
#
# The amount of gas is reset to n0 just after time 1.
# So, p(2) = 20.
#

[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
[]

[Mesh]#Comment
  file = plenum_pressure.e
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./displ_positive]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0'
  [../]
  [./displ_negative]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0'
  [../]
  [./temp1]
    type = PiecewiseLinear
    x = '0 2'
    y = '70 70'
  [../]
  [./material_input_function]
    type = PiecewiseLinear
    x = '0 2'
    y = '0 0.24054443866068703' # 2/R
  [../]
[]

[Variables]

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
  [../]

  [./material_input]
    order = FIRST
    family = LAGRANGE
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

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Kernels]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

  [./material_input_dummy]
    type = HeatConduction
    variable = material_input
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
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]

[] # AuxKernels


[BCs]

  [./no_x_exterior]
    type = DirichletBC
    variable = disp_x
    boundary = '7 8 100'
    value = 0.0
  [../]
  [./no_y_exterior]
    type = DirichletBC
    variable = disp_y
    boundary = '9 10 100'
    value = 0.0
  [../]
  [./no_z_exterior]
    type = DirichletBC
    variable = disp_z
    boundary = '11 12 100'
    value = 0.0
  [../]




  [./temperatureInterior]
    type = FunctionDirichletBC
    boundary = 100
    function = temp1
    variable = temp
  [../]
  [./MaterialInput]
    type = FunctionDirichletBC
    boundary = '100 13 14 15 16'
    function = material_input_function
    variable = material_input
  [../]


  [./PlenumPressure]
    [./1]
      boundary = 100
      initial_pressure = 10
      material_input = materialInput
      R = 8.314472
      temperature = aveTempInterior
      volume = internalVolume
      output = ppress
      output_initial_moles = initial_moles
      refab_time = 1.01
      refab_pressure = 10
      refab_temperature = 350
      refab_volume = 35
    [../]
  [../]

[] # BCs


[Materials]

  [./stiffStuff]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e1
    poissons_ratio = 0.0
    thermal_expansion = 0
  [../]

  [./stiffStuff2]
    type = Elastic
    block = 2

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.0
    thermal_expansion = 0
  [../]

  [./heatconduction]
    type = HeatConductionMaterial
    block = '1 2'
    thermal_conductivity = 1.0
    specific_heat = 1.0
  [../]

  [./density]
    type = Density
    block = '1 2'
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



  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-12

  l_max_its = 20

  start_time = 1.5
  dt = 0.5
  end_time = 2.0

  restart_file_base = plenum_pressure_refab_restart1_out_cp/0003
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 100
    execute_on = residual
  [../]

  [./aveTempInterior]
    type = SideAverageValue
    boundary = 100
    variable = temp
    execute_on = residual
  [../]
  [./materialInput]
    type = SideAverageValue
    boundary = '7 8 9 10 11 12'
    variable = material_input
    execute_on = residual
  [../]

[]

[Output]
  linear_residuals = true
  file_base = plenum_pressure_refab_out
  interval = 1
  output_initial = true
  elemental_as_nodal = true
  exodus = true
  perf_log = true
[]

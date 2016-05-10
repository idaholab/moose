#
# This is a test of the piece-wise linear strain hardening model using the
# small strain formulation.  This test exercises the temperature-dependent
# yield stress.
#
# Test procedure:
# 1. The element is pulled to and then beyond the yield stress for a given
# temperature.
# 2. The displacement is then constant while the temperature increases and
# the yield stress decreases.  This results in a lower stress with more
# plastic strain.
# 3. The temperature decreases beyond its original value giving a higher
# yield stress.  The displacement increases, causing increases stress to
# the new yield stress.
# 4. The temperature and yield stress are constant with increasing
# displacement giving a constant stress and more plastic strain.
#
# Plotting total_strain_yy on the x axis and stress_yy on the y axis shows
# the stress history in a clear way.
#
#  s |
#  t |            *****
#  r |           *
#  e |   *****  *
#  s |  *    * *
#  s | *     *
#    |*
#    +------------------
#           total strain
#

[Mesh]
  file = 1x1x1_cube.e
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
[]


[AuxVariables]

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./total_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]


[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '0 1     2    4    5    6'
    y = '0 0.025 0.05 0.05 0.06 0.085'
  [../]
  [./hf1]
    type = PiecewiseLinear
    x = '0.0  0.01 0.02 0.03 0.1'
    y = '5000 5030 5060 5090 5300'
  [../]
  [./hf2]
    type = PiecewiseLinear
    x = '0.0  0.01 0.02 0.03 0.1'
    y = '4000 4020 4040 4060 4200'
  [../]
  [./temp]
    type = PiecewiseLinear
    x = '0   1   2   3   4'
    y = '500 500 500 600 400'
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
  [./heat]
    type = HeatConduction
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

  [./total_strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = total_strain_yy
    index = 1
  [../]

  [./plastic_strain_xx]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_xx
    index = 0
  [../]

  [./plastic_strain_yy]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_yy
    index = 1
  [../]

  [./plastic_strain_zz]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_zz
    index = 2
  [../]

[]


[BCs]

  [./y_pull_function]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 5
    function = top_pull
  [../]

  [./x_bot]
    type = PresetBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./y_bot]
    type = PresetBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./z_bot]
    type = PresetBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

  [./temp]
    type = FunctionPresetBC
    variable = temp
    function = temp
    boundary = 4
  [../]
[]

[Postprocessors]
  [./stress_yy_el]
    type = ElementalVariableValue
    variable = stress_yy    
    elementid = 0
  [../]
[]

[Materials]
  [./vermont]
    type = SolidModel
    formulation = Linear
    block = 1
    youngs_modulus = 2e5
    poissons_ratio = .3
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    constitutive_model = kentucky
  [../]
  [./kentucky]
    type = IsotropicTempDepHardening
    block = 1
    temp_dep_hardening_functions = 'hf1 hf2'
    temp_dep_hardening_functions_temps = '300.0 800.0'
    relative_tolerance = 1e-25
    absolute_tolerance = 1e-5
    temp = temp
  [../]
  [./utah]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 1
    thermal_conductivity = 1
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'


  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 6
  dt = 0.1
[]

[Outputs]
  csv = true
  [./out]
    type = Exodus
  [../]
[]

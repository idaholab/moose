# Patch Test

# This test is designed to compute displacements from a thermal strain.

# The cube is displaced by 1e-6 units in x, 2e-6 in y, and 3e-6 in z.
#  The faces are sheared as well (1e-6, 2e-6, and 3e-6 for xy, yz, and
#  zx).  This gives a uniform strain/stress state for all six unique
#  tensor components.

# The temperature moves 100 degrees, and the coefficient of thermal
#  expansion is 1e-6.  Therefore, the strain (and the displacement
#  since this is a unit cube) is 1e-4.

[Mesh]
  file = thermal_strain_test.e
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./tempFunc]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '117.56 217.56'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]

  [./temp]
    initial_condition = 117.56
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
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 9
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 14
    value = 0
  [../]

  [./temp]
    type = FunctionDirichletBC
    variable = temp
    boundary = '10 12'
    function = tempFunc
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    bulk_modulus = 0.333333333333e6
    poissons_ratio = 0.0

    temp = temp
    thermal_expansion = 1e-6
  [../]
  [./stiffStuff2]
    type = Elastic
    block = 2

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    bulk_modulus = 0.333333333333e6
    lambda = 0.0

    temp = temp
    thermal_expansion = 1e-6
  [../]
  [./stiffStuff3]
    type = Elastic
    block = 3

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.0

    temp = temp
    thermal_expansion = 1e-6
  [../]
  [./stiffStuff4]
    type = Elastic
    block = 4

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.0

    temp = temp
    thermal_expansion = 1e-6
  [../]
  [./stiffStuff5]
    type = Elastic
    block = 5

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    lambda = 0.0

    temp = temp
    thermal_expansion = 1e-6
  [../]
  [./stiffStuff6]
    type = Elastic
    block = 6

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    shear_modulus = 5e5

    temp = temp
    thermal_expansion = 1e-6
  [../]
  [./stiffStuff7]
    type = Elastic
    block = 7

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    shear_modulus = 5e5
    poissons_ratio = 0.0

    temp = temp
    thermal_expansion = 1e-6
  [../]

  [./heat]
    type = HeatConductionMaterial
    block = '1 2 3 4 5 6 7'

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = Density
    block = '1 2 3 4 5 6 7'
    density = 1.0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  nl_abs_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 0.5
  num_steps = 2
  end_time = 1.0
[]

[Outputs]
  file_base = thermal_strain_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

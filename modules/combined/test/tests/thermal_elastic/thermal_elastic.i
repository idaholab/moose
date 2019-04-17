# Patch Test

# This test is designed to compute constant xx, yy, zz, xy, yz, and xz
#  stress on a set of irregular hexes.  The mesh is composed of one
#  block with seven elements.  The elements form a unit cube with one
#  internal element.  There is a nodeset for each exterior node.

# The cube is displaced by 1e-6 units in x, 2e-6 in y, and 3e-6 in z.
#  The faces are sheared as well (1e-6, 2e-6, and 3e-6 for xy, yz, and
#  zx).  This gives a uniform strain/stress state for all six unique
#  tensor components.  This displacement is again applied in the second
#  step.

# With Young's modulus at 1e6 and Poisson's ratio at 0, the shear
#  modulus is 5e5 (G=E/2/(1+nu)).  Therefore, for the mechanical strain,
#
#  stress xx = 1e6 * 1e-6 = 1
#  stress yy = 1e6 * 2e-6 = 2
#  stress zz = 1e6 * 3e-6 = 3
#  stress xy = 2 * 5e5 * 1e-6 / 2 = 0.5
#             (2 * G   * gamma_xy / 2 = 2 * G * epsilon_xy)
#  stress yz = 2 * 5e5 * 2e-6 / 2 = 1
#  stress zx = 2 * 5e5 * 3e-6 / 2 = 1.5

# Young's modulus is a function of temperature for this test.  The
#  temperature changes from 100 to 500.  The Young's modulus drops
#  due to that temperature change from 1e6 to 6e5.

# Poisson's ratio also is a function of temperature and changes from
#  0 to 0.25.

# At the end of the temperature ramp, E=6e5 and nu=0.25.  This gives
#  G=2.4e=5.  lambda=E*nu/(1+nu)/(1-2*nu)=2.4E5.  The final stress
#  is therefore

#  stress xx = 2.4e5 * 12e-6 + 2*2.4e5*2e-6 = 3.84
#  stress yy = 2.4e5 * 12e-6 + 2*2.4e5*4e-6 = 4.80
#  stress zz = 2.4e5 * 12e-6 + 2*2.4e5*6e-6 = 5.76
#  stress xy = 2 * 2.4e5 * 2e-6 / 2 = 0.48
#             (2 * G   * gamma_xy / 2 = 2 * G * epsilon_xy)
#  stress yz = 2 * 2.4e5 * 4e-6 / 2 = 0.96
#  stress xz = 2 * 2.4e5 * 6e-6 / 2 = 1.44

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = thermal_elastic.e
[]

[Functions]
  [./ramp1]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 2.'
    scale_factor = 1e-6
  [../]
  [./ramp2]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 2.'
    scale_factor = 2e-6
  [../]
  [./ramp3]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 2.'
    scale_factor = 3e-6
  [../]
  [./ramp4]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 2.'
    scale_factor = 4e-6
  [../]
  [./ramp6]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 2.'
    scale_factor = 6e-6
  [../]
  [./tempFunc]
    type = PiecewiseLinear
    x = '0     1     2'
    y = '100.0 100.0 500.0'
  [../]
[]

[Variables]
  [./temp]
    initial_condition = 100.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz'
    strain = FINITE
  [../]
[]

[Kernels]
  [./heat]
    type = Diffusion
    variable = temp
  [../]
[]

[BCs]
  [./node1_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./node1_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = ramp2
  [../]
  [./node1_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 1
    function = ramp3
  [../]

  [./node2_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 2
    function = ramp1
  [../]
  [./node2_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = ramp2
  [../]
  [./node2_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 2
    function = ramp6
  [../]

  [./node3_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 3
    function = ramp1
  [../]
  [./node3_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./node3_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 3
    function = ramp3
  [../]

  [./node4_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./node4_y]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
  [./node4_z]
    type = DirichletBC
    variable = disp_z
    boundary = 4
    value = 0.0
  [../]

  [./node5_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 5
    function = ramp1
  [../]
  [./node5_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 5
    function = ramp4
  [../]
  [./node5_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 5
    function = ramp3
  [../]

  [./node6_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 6
    function = ramp2
  [../]
  [./node6_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 6
    function = ramp4
  [../]
  [./node6_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 6
    function = ramp6
  [../]

  [./node7_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 7
    function = ramp2
  [../]
  [./node7_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 7
    function = ramp2
  [../]
  [./node7_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 7
    function = ramp3
  [../]

  [./node8_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 8
    function = ramp1
  [../]
  [./node8_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 8
    function = ramp2
  [../]
  [./node8_z]
    type = DirichletBC
    variable = disp_z
    boundary = 8
    value = 0.0
  [../]

  [./temp]
    type = FunctionDirichletBC
    variable = temp
    boundary = '10 12'
    function = tempFunc
  [../]
[]

[Materials]
  [./youngs_modulus]
    type = PiecewiseLinearInterpolationMaterial
    x = '100 500'
    y = '1e6 6e5'
    property = youngs_modulus
    variable = temp
  [../]
  [./poissons_ratio]
    type = PiecewiseLinearInterpolationMaterial
    x = '100 500'
    y = '0   0.25'
    property = poissons_ratio
    variable = temp
  [../]

  [./elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    args = temp
    youngs_modulus = youngs_modulus
    poissons_ratio = poissons_ratio
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-9

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]

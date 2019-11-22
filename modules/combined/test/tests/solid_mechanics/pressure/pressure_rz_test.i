#
# Pressure Test
#
# This test is taken from the Abaqus verification manual:
#   "1.3.4 Axisymmetric solid elements"
#
# The two lower nodes are not allowed to translate in the z direction.
# Step 1:
#   Pressure of 1000 is applied on each face.
# Step 2:
#   Step 1 load plus a pressure on the vertical faces that varies from
#   0 to 1000 from top to bottom.
#
# Solution:
# Step 1:
#    Stress xx, yy, zz = -1000
#    Stress xy = 0
# Step 2:
#    Stress xx, zz = -1500
#    Stress yy = -1000
#    Stress xy = 0

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = pressure_rz_test.e
[] # Mesh

[Functions]
  [./constant]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 1e3
  [../]
  [./vary]
    type = ParsedFunction
    value = 'if(t <= 1, 1000 , 1000+1000*(1-y))'
  [../]
[] # Functions

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = SMALL
    additional_generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  []
[]

[BCs]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./Pressure]
    [./Pressure1]
      boundary = '3 4'
      function = constant
    [../]
    [./Pressure2]
      boundary = '1 2'
      function = vary
    [../]
  [../]
[] # BCs

[Materials]
  [./constant]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./constant_stress]
    type = ComputeLinearElasticStress
    block = '1'
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-12

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 2
  end_time = 2.0
[] # Executioner

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs

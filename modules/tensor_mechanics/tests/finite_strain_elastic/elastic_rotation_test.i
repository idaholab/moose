#
# Rotation Test
#
# This test is designed to compute a uniaxial stress and then follow that
# stress as the mesh is rotated 90 degrees.
#
# The mesh is composed of one block with a single element.  The nodal
# displacements in the x and y directions are prescribed.  Poisson's
# ratio is zero.
#
[Mesh]
  type = FileMesh
  file = rotation_test.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./x_200]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, delta*t, (1.0+delta)*cos(pi/2*(t-t0)) - 1.0)'
  [../]
  [./y_200]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, (1.0+delta)*sin(pi/2*(t-t0)))'
  [../]
  [./x_300]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, delta*t, (1.0+delta)*cos(pi/2.0*(t-t0)) - sin(pi/2.0*(t-t0)) - 1.0)'
  [../]
  [./y_300]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, cos(pi/2.0*(t-t0)) + (1+delta)*sin(pi/2.0*(t-t0)) - 1.0)'
  [../]
  [./x_400]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, -sin(pi/2.0*(t-t0)))'
  [../]
  [./y_400]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, cos(pi/2.0*(t-t0)) - 1.0)'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    #generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[BCs]
  # BCs
  [./no_x]
    type = PresetBC
    variable = disp_x
    boundary = 100
    value = 0.0
  [../]
  [./no_y]
    type = PresetBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]
  [./x_200]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 200
    function = x_200
  [../]
  [./y_200]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 200
    function = y_200
  [../]
  [./x_300]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 300
    function = x_300
  [../]
  [./y_300]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 300
    function = y_300
  [../]
  [./x_400]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 400
    function = x_400
  [../]
  [./y_400]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 400
    function = y_400
  [../]
  [./no_z]
    type = PresetBC
    variable = disp_z
    boundary = '100 200 300 400'
    value = 0.0
  [../]
[]

[Materials]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '10.0e6  0.0   0.0 10.0e6  0.0  10.0e6 5e6 5e6 5e6'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type '
  petsc_options_value = lu
  nl_rel_tol = 1e-30
  nl_abs_tol = 1e-20
  l_max_its = 20
  start_time = 0.0
  dt = 0.01
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]

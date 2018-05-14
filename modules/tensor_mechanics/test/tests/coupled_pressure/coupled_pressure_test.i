#
# Pressure Test
#
# This test is designed to compute pressure loads on three faces of a unit cube.
# The pressure is computed as an auxiliary variable. It should give the same result
# as pressure_test.i
#
# The mesh is composed of one block with a single element.  Symmetry bcs are
# applied to the faces opposite the pressures.  Poisson's ratio is zero,
# which makes it trivial to check displacements.
#


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = FileMesh
  file = pressure_test.e
[]

[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 1.0
  [../]
  [./zeroRamp]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 0. 1.'
    scale_factor = 2.0
  [../]
  [./rampUnramp]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 0.'
    scale_factor = 10.0
  [../]
[]

[AuxVariables]
  [./pressure_1]
  [../]
  [./pressure_2]
  [../]
  [./pressure_3]
  [../]
[]

[AuxKernels]
  [./side1_pressure_ak]
    type = FunctionAux
    variable = pressure_1
    boundary = 1
    function = rampConstant
  [../]
  [./side2_pressure_ak]
    type = FunctionAux
    variable = pressure_2
    boundary = 2
    function = zeroRamp
  [../]
  [./side3_pressure_ak]
    type = FunctionAux
    variable = pressure_3
    boundary = 3
    function = rampUnramp
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = SMALL
        add_variables = true
      [../]
    [../]
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 5
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 6
    value = 0.0
  [../]
  [./CoupledPressure]
    [./Side1]
      boundary = '1'
      pressure = pressure_1
      displacements = 'disp_x disp_y disp_z'
    [../]
    [./Side2]
      boundary = '2'
      pressure = pressure_2
      displacements = 'disp_x disp_y disp_z'
    [../]
  [../]

  [./side3_x]
    type = CoupledPressureBC
    variable = 'disp_x'
    boundary = '3'
    pressure = pressure_3
    component = 0
  [../]
  [./side3_y]
    type = CoupledPressureBC
    variable = 'disp_y'
    boundary = '3'
    pressure = pressure_3
    component = 1
  [../]
  [./side3_z]
    type = CoupledPressureBC
    variable = 'disp_z'
    boundary = '3'
    pressure = pressure_3
    component = 2
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 2
  end_time = 2.0
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

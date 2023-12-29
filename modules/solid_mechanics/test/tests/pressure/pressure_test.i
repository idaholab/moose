#
# Pressure Test
#
# This test is designed to compute pressure loads on three faces of a unit cube.
#
# The mesh is composed of one block with a single element.  Symmetry bcs are
# applied to the faces opposite the pressures.  Poisson's ratio is zero,
# which makes it trivial to check displacements.
#


[Mesh]
  type = FileMesh
  file = pressure_test.e
  displacements = 'disp_x disp_y disp_z'
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
    scale_factor = 1.0
  [../]
  [./rampUnramp]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 0.'
    scale_factor = 10.0
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
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
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
  [./Pressure]
    [./Side1]
      boundary = 1
      function = rampConstant
      displacements = 'disp_x disp_y disp_z'
    [../]
    [./Side2]
      boundary = 2
      function = zeroRamp
      displacements = 'disp_x disp_y disp_z'
      factor = 2.0
    [../]
    [./Side3]
      boundary = 3
      function = rampUnramp
      displacements = 'disp_x disp_y disp_z'
    [../]
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = 1
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    block = 1
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 1
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

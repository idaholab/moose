###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of a central difference with a
# direct calculation of acceleration.
#
# Testing that the first and second time derivatives
# are calculated correctly using the Direct Central Difference
# method

# Testing the accuracy of the timestep averaging method within
# the Direct Central Difference method
###########################################################

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 1
  ny = 1
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Functions]
  #Mid-step velocities
  #0 0.00625 0.015 0.0075 0.25 0
  #Accelerations
  #0.025 0.01944 -0.01 0.48 -2.17
  [forcing_fn]
    type = PiecewiseLinear
    x = '0.0 0.1 0.5    1.0  2.0   2.01 2.23'
    y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
  []
[]

[Kernels]
  [DynamicSolidMechanics]
    displacements = 'disp_x disp_y'
  []
  [massmatrix]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_x
  []
  [massmatrix_y]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_y
  []
[]

[Materials]
  [elasticity_tensor_block_one]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e1
    poissons_ratio = 0.0
  []

  [strain_block]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y'
    implicit = false
  []
  [stress_block]
    type = ComputeFiniteStrainElasticStress
    implicit = false
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 5
  []
[]

[BCs]
  [left_x]
    type = ExplicitFunctionDirichletBC
    variable = disp_x
    boundary = 'left'
    function = forcing_fn
  []
  [right_x]
    type = ExplicitFunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = forcing_fn
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    second_order_vars = 'disp_x disp_y'
  []
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '0.0 0.1 0.5 1.0 2.0 2.01 2.23'
  []
  start_time = 0.0
  num_steps = 6
  dt = 0.1
[]

[Postprocessors]
  [udot]
    type = ElementAverageTimeDerivative
    variable = disp_x
  []
  [udotdot]
    type = ElementAverageSecondTimeDerivative
    variable = disp_x
  []
  [u]
    type = ElementAverageValue
    variable = disp_x
  []
[]

[Outputs]
  exodus = true
[]

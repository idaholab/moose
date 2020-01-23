# ---------------------------------------------------------------------------
# This test is designed to verify the variable elasticity tensor functionality in the
# ComputeFiniteStrainElasticStress class with the elasticity_tensor_has_changed flag
# by varying the young's modulus with temperature. A constant strain is applied
# to the mesh in this case, and the stress varies with the changing elastic constants.
#
# Geometry: A single element cube in symmetry boundary conditions and pulled
#           at a constant displacement to create a constant strain in the x-direction.
#
# Temperature:  The temperature varies from 400K to 700K in this simulation by
#           100K each time step. The temperature is held constant in the last
#           timestep to ensure that the elasticity tensor components are constant
#           under constant temperature.
#
# Results: Because Poisson's ratio is set to zero, only the stress along the x
#          axis is non-zero.  The stress changes with temperature.
#
#    Temperature(K)   strain_{xx}(m/m)     Young's Modulus(Pa)   stress_{xx}(Pa)
#          400              0.001             10.0e6               1.0e4
#          500              0.001             10.0e6               1.0e4
#          600              0.001              9.94e6              9.94e3
#          700              0.001              9.93e6              9.93e3
#
#    The tensor mechanics results align exactly with the analytical results above
#    when this test is run with ComputeIncrementalSmallStrain.  When the test is
#    run with ComputeFiniteStrain, a 0.05% discrepancy between the analytical
#    strains and the simulation strain results is observed, and this discrepancy
#    is carried over into the calculation of the elastic stress.
#-------------------------------------------------------------------------

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./temp]
    initial_condition = 400
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./temperature_function]
    type = PiecewiseLinear
    x = '1       4'
    y = '400   700'
  [../]
[]

[Kernels]
  [./heat]
    type = Diffusion
    variable = temp
  [../]

  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]


[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]

 [./elastic_strain_xx]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./u_left_fix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./u_back_fix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./u_pull_right]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.001
  [../]

  [./temp_bc_1]
    type = FunctionDirichletBC
    variable = temp
    preset = false
    boundary = '1 2 3 4'
    function = temperature_function
  [../]
[]

[Materials]
  [./youngs_modulus]
    type = PiecewiseLinearInterpolationMaterial
    xy_data = '0          10e+6
               599.9999   10e+6
               600        9.94e+6
               99900      10e3'
    property = youngs_modulus
    variable = temp
  [../]
  [./elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    args = temp
    youngs_modulus = youngs_modulus
    poissons_ratio = 0.0
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient

  end_time = 5
[]

[Postprocessors]
  [./elastic_strain_xx]
    type = ElementAverageValue
    variable = elastic_strain_xx
  [../]
  [./elastic_stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  [../]

  [./temp]
    type = AverageNodalVariableValue
    variable = temp
  [../]
[]

[Outputs]
  exodus = true
[]

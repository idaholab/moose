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
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy plastic_strain_xx plastic_strain_yy plastic_strain_zz'
    use_automatic_differentiation = true
  [../]
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '0 1     2    4    5    6'
    y = '0 0.025 0.05 0.05 0.06 0.085'
  [../]
  [./yield]
    type = PiecewiseLinear
    x = '400 500 600'
    y = '6e3 5e3 4e3'
  [../]
  [./temp]
    type = PiecewiseLinear
    x = '0   1   2   3   4'
    y = '500 500 500 600 400'
  [../]
[]

[Kernels]
  [./heat]
    type = ADHeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  [../]
  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./temp]
    type = FunctionDirichletBC
    variable = temp
    function = temp
    boundary = left
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 2.0e5
    poissons_ratio = 0.3
  [../]
  [./creep_plas]
    type = ADComputeMultipleInelasticStress
    block = 0
    inelastic_models = 'plasticity'
    max_iterations = 50
    absolute_tolerance = 1e-05
  [../]
  [./plasticity]
    type = ADIsotropicPlasticityStressUpdate
    block = 0
    hardening_constant = 0
    yield_stress_function = yield
    temperature = temp
  [../]
  [./heat_conduction]
    type = ADHeatConductionMaterial
    block = 0
    specific_heat = 1
    thermal_conductivity = 1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

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
  exodus = true
[]

# This problem consists of two beams with different prescribed temperatures on
# the top of the top beam and the bottom of the bottom beam.  The top beam is
# fixed against vertical displacement on the top surface, and the bottom beam
# bends downward due to thermal expansion.

# This is a test of the effectiveness of the Jacobian terms coupling temperature
# and displacement for thermal contact. The Jacobian is not exactly correct,
# but is close enough that this challenging problem converges in a small number
# of nonlinear iterations using NEWTON.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [./msh]
    type = FileMeshGenerator
    file = two_blocks.e
  []
[]

[Variables]
  [./temp]
  [../]
[]

[Kernels]
  [./heat]
    type = ADHeatConduction
    variable = temp
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = FINITE
    add_variables = true
    eigenstrain_names = thermal_expansion
    generate_output = 'stress_xx stress_yy stress_zz stress_yz stress_xz stress_xy'
    use_automatic_differentiation = true
  [../]
[]

[Contact]
  [./mechanical]
    primary = 4
    secondary = 5
    formulation = kinematic
    tangential_tolerance = 1e-1
    penalty = 1e10
  [../]
[]

[ThermalContact]
  [./thermal]
    type = GapHeatTransfer
    variable = temp
    primary = 4
    secondary = 5
    emissivity_primary = 0
    emissivity_secondary = 0
    gap_conductivity = 1e4
    quadrature = true
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = 7
    value = 0
  [../]
  [./top_temp]
    type = DirichletBC
    variable = temp
    boundary = 7
    value = 1000.0
  [../]
  [./bot_temp]
    type = DirichletBC
    variable = temp
    boundary = 6
    value = 500.0
  [../]
[]

[Materials]
  [./density]
    type = Density
    density = 100
  [../]
  [./temp]
    type = ADHeatConductionMaterial
    thermal_conductivity = 1e5
    specific_heat = 620.0
  [../]
  [./Elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
  [../]
  [./thermal_eigenstrain]
    type = ADComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-5
    stress_free_temperature = 500
    temperature = temp
    eigenstrain_name = thermal_expansion
  [../]
  [./stress]
    type = ADComputeFiniteStrainElasticStress
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
[]

[Executioner]
  automatic_scaling = true
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
  nl_max_its = 15
  l_tol = 1e-10
  l_max_its = 50
  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  num_steps = 1
  line_search = none
[]

#
# This test is Example 3 from "A Consistent Formulation for the Integration
#   of Combined Plasticity and Creep" by P. Duxbury, et al., Int J Numerical
#   Methods in Engineering, Vol. 37, pp. 1277-1295, 1994.
#
# The problem is a one-dimensional bar which is loaded from yield to a value of twice
#   the initial yield stress and then unloaded to return to the original stress. The
#   bar must harden to the required yield stress during the load ramp, with no
#   further yielding during unloading. The initial yield stress (sigma_0) is prescribed
#   as 20 with a plastic strain hardening of 100. The mesh is a 1x1x1 cube with symmetry
#   boundary conditions on three planes to provide a uniaxial stress field.
#   The temperature is held constant at 1000.
#
#  In the PowerLawCreep model, the creep strain rate is defined by:
#
#   edot = A(sigma)**n * exp(-Q/(RT)) * t**m
#
#   The creep law specified in the paper, however, defines the creep strain rate as:
#
#   edot = Ao * mo * (sigma)**n * t**(mo-1)
#      with the creep parameters given by
#         Ao = 1e-7
#         mo = 0.5
#         n  = 5
#
#   thus, input parameters for the test were specified as:
#         A = Ao * mo = 1e-7 * 0.5 = 0.5e-7
#         m = mo-1 = -0.5
#         n = 5
#         Q = 0
#
#   The variation of load P with time is:
#       P = 20 + 20t      0 < t < 1
#       P = 40 - 40(t-1)  1 < t 1.5
#
#  The analytic solution for total strain during the loading period 0 < t < 1 is:
#
#    e_tot = (sigma_0 + 20*t)/E + 0.2*t + A * t**0.5  * sigma_0**n * [ 1 + (5/3)*t +
#               + 2*t**2 + (10/7)*t**3 + (5/9)**t**4 + (1/11)*t**5 }
#
#    and during the unloading period 1 < t < 1.5:
#
#    e_tot = (sigma_1 - 40*(t-1))/E + 0.2 + (4672/693) * A * sigma_0**n +
#               A * sigma_0**n * [ t**0.5 * ( 32 - (80/3)*t + 16*t**2 - (40/7)*t**3
#                                  + (10/9)*t**4 - (1/11)*t**5 ) - (11531/693) ]
#
#         where sigma_1 is the stress at time t = 1.
#
#  Assuming a Young's modulus (E) of 1000 and using the parameters defined above:
#
#    e_tot(1) = 2.39734
#    e_tot(1.5) = 3.16813
#
#
#   The numerically computed solution is:
#
#    e_tot(1) = 2.39718         (~0.006% error)
#    e_tot(1.5) = 3.15555       (~0.40% error)
#
#
#   Note that this test is not a completely correct representation of the analytical problem
#     since the code does not set an initial condition for the stress.  Currently (21 Feb 2012),
#     no capability exists for setting the initial condition of stress.  Until
#     that is available, some error will be inherent in the solution. This error has been
#     minimized by using a very small initial time increment.
#
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = 1x1x1_cube.e
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

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1000.0
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./total_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '  0   1   1.5'
    y = '-20 -40   -20'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
  [../]

  [./elastic_strain_yy]
    type = RankTwoAux
    variable = elastic_strain_yy
    rank_two_tensor = elastic_strain
    index_i = 1
    index_j = 1
  [../]

  [./plastic_strain_yy]
    type = RankTwoAux
    variable = plastic_strain_yy
    rank_two_tensor = plastic_strain
    index_i = 1
    index_j = 1
  [../]

  [./creep_strain_yy]
    type = RankTwoAux
    variable = creep_strain_yy
    rank_two_tensor = creep_strain
    index_i = 1
    index_j = 1
  [../]

  [./total_strain_yy]
    type = RankTwoAux
    variable = total_strain_yy
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
  [../]
[]

[BCs]
  [./u_top_pull]
    type = Pressure
    variable = disp_y
    component = 1
    boundary = 5
    factor = 1
    function = top_pull
  [../]

  [./u_bottom_fix]
    type = PresetBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./u_yz_fix]
    type = PresetBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./u_xy_fix]
    type = PresetBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

  [./temp_top_fix]
    type = PresetBC
    variable = temp
    boundary = 5
    value = 1000.0
  [../]
  [./temp_bottom_fix]
    type = PresetBC
    variable = temp
    boundary = 3
    value = 1000.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  [../]

  [./strain]
    type = ComputeFiniteStrain
    block = 1
  [../]

  [./creep_plas]
    type = ComputeReturnMappingStress
    block = 1
    return_mapping_models = 'creep plas'
    absolute_tolerance = 1e-8
    output_iteration_info = false
    max_iterations = 50
  [../]

  [./creep]
    type = PowerLawCreepStressUpdate
    block = 1
    coefficient = 0.5e-7
    n_exponent = 5
    m_exponent = -0.5
    activation_energy = 0
    relative_tolerance = 1e-5
    absolute_tolerance = 1e-20
    max_iterations = 30
    temperature =  temp
  [../]

  [./plas]
    type = IsotropicPlasticityStressUpdate
    block = 1
    hardening_constant = 100
    yield_stress = 20
    temperature = temp
    max_iterations = 30
    relative_tolerance = 1e-5
    absolute_tolerance = 1e-20
  [../]

  [./thermal]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 1.0
    thermal_conductivity = 100.
  [../]
  [./density]
    type = Density
    block = 1
    density = 1
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Postprocessors]
  [./elem_plastic_strain_yy]
    type = ElementalVariableValue
    variable = plastic_strain_yy
    elementid = 0
  [../]

  [./elem_creep_strain_yy]
    type = ElementalVariableValue
    variable = creep_strain_yy
    elementid = 0
  [../]

  [./elem_elastic_strain_yy]
    type = ElementalVariableValue
    variable = elastic_strain_yy
    elementid = 0
  [../]

  [./elem_total_strain_yy]
    type = ElementalVariableValue
    variable = total_strain_yy
    elementid = 0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101           asm      lu'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 6
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
  l_tol = 1e-5
  start_time = 0.0
  end_time = 1.4999999999

  dt = 0.001
  [./TimeStepper]
    type = FunctionDT
    time_t  = '0        0.5    1.0    1.5'
    time_dt = '0.015  0.015  0.005  0.005'
  [../]
[]

[Postprocessors]
  [./timestep]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
[]

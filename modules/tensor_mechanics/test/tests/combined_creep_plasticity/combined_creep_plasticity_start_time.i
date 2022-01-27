#
# This test is Example 2 from "A Consistent Formulation for the Integration
#   of Combined Plasticity and Creep" by P. Duxbury, et al., Int J Numerical
#   Methods in Engineering, Vol. 37, pp. 1277-1295, 1994.
#
# The problem is a one-dimensional bar which is loaded from yield to a value of twice
#   the initial yield stress and then unloaded to return to the original stress. The
#   bar must harden to the required yield stress during the load ramp, with no
#   further yielding during unloading. The initial yield stress (sigma_0) is prescribed
#   as 20 with a plastic strain hardening of 100. The mesh is a 1x1x1 cube with symmetry
#   boundary conditions on three planes to provide a uniaxial stress field.
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
#               + 2*t**2 + (10/7)*t**3 + (5/9)**t**4 + (1/11)*t**5 ]
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

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy elastic_strain_yy creep_strain_yy plastic_strain_yy'
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '  10   11   11.5'
    y = '-20 -40   -20'
  [../]

  [./dts]
    type = PiecewiseLinear
    x = '10        10.5    11.0    11.5'
    y = '0.015  0.015  0.005  0.005'
  [../]
[]

[BCs]
  [./u_top_pull]
    type = Pressure
    variable = disp_y
    boundary = top
    factor = 1
    function = top_pull
  [../]
  [./u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./u_yz_fix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./u_xy_fix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  [../]
  [./creep_plas]
    type = ComputeMultipleInelasticStress
    block = 0
    tangent_operator = elastic
    inelastic_models = 'creep plas'
    max_iterations = 50
    absolute_tolerance = 1e-05
    combined_inelastic_strain_weights = '0.0 1.0'
  [../]
  [./creep]
    type = PowerLawCreepStressUpdate
    block = 0
    coefficient = 0.5e-7
    n_exponent = 5
    m_exponent = -0.5
    activation_energy = 0
    start_time = 10
  [../]
  [./plas]
    type = IsotropicPlasticityStressUpdate
    block = 0
    hardening_constant = 100
    yield_stress = 20
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 6
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
  l_tol = 1e-5
  start_time = 10.0
  end_time = 11.5

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  exodus = true
[]

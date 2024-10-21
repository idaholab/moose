# This test simulates shear test. The shear load is applied in one direction.
#
# -------------------
# ANALYTICAL SOLUTION
# -------------------
# https://mooseframework.inl.gov/source/materials/HillCreepStressUpdate.html
# q = [F(S22-S33)^2 + G(S33-S11)^2 + H(S11-S22)^2 + 2L(S23)^2 + 2M(S13)^2 + 2N(S12)^2]^0.5
# S12 = 10 Pa; other compoenents of stress are zero since it is a uniaxial test
# F=0.5 G=0.25 H=0.3866 L=1.6413 M=1.6413 N=1.2731 (as used in this test)
# Substituting the values of stress components and F, G, H, L, M and N we obtain
# q = 15.9568 Pa
#
# Equivalent_creep_strain_rate = A(q)^n (power law creep rate used in this test)
# Substituting A=1e-16 and n=9, and q as calculated above, we obtain
# Equivalent_creep_strain_rate = 6.7068e-06
#
# The 12 (xy) component of creep_strain_tensor is calculated as below
# creep_strain_tensor_12 = (Equivalent_creep_strain_rate / q) * 2.0 * N * S12 * time_increment
# Substituting the values and time_increment as 0.001 we obtain the analytical solution.
#
#                               MOOSE               Analytical
#  creep_strain_tensor_12     1.070870e-09         1.07019e-09
#
# -----------------------------------------
# PYTHON SCRIPT FOR THE ANALYTICAL SOLUTION
# -----------------------------------------
# import math
# F=0.5; G=0.25; H=0.3866; L=1.6413; M=1.6413; N=1.2731
# S11=0; S22=0; S33=0; S23=0; S13=0; S12=10
# q = math.sqrt(F*(S22-S33)**2 + G*(S33-S11)**2 + H*(S11-S22)**2 + 2*L*(S23)**2 + 2*M*(S13)**2 + 2*N*(S12)**2)
# print(q)
# A=1e-16; n=9; time=0.0001
# equivalent_creep_strain_rate = A*(q**n)
# print(equivalent_creep_strain_rate)
# equivalent_creep_strain_rate_12=(equivalent_creep_strain_rate / q) * 2.0 * N * S12 * time
# print(equivalent_creep_strain_rate_12)

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0.0
    ymin = 0.0
    zmin = 0.0

    xmax = 1.0
    ymax = 1.0
    zmax = 1.0
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[AuxVariables]
  [hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [creep_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xy
    index_i = 0
    index_j = 1
  []
  [sigma_xy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = FINITE
    generate_output = 'elastic_strain_xy stress_xy'
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 700
    poissons_ratio = 0.0
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_creep_two"
    max_iterations = 50
    absolute_tolerance = 1e-16
  []

  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5 0.25 0.3866 1.6413 1.6413 1.2731"
  []

  [trial_creep_two]
    type = ADHillCreepStressUpdate
    coefficient = 1e-16
    n_exponent = 9
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 0.00003
    absolute_tolerance = 1e-20
    relative_tolerance = 1e-20
    # Force it to not use integration error
    max_integration_error = 100.0
    anisotropic_elasticity = true
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  [no_disp_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []

  [shear_load]
    type = ADNeumannBC
    variable = disp_x
    boundary = top
    value = 10
  []

  [no_disp_y_top]
    type = ADDirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  []

[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -mat_mffd_err'
  petsc_options_value = 'lu     superlu_dist                    1e-5'

  nl_rel_tol = 1.0e-14
  nl_abs_tol = 1.0e-14
  l_max_its = 10
  num_steps = 5
  dt = 1.0e-4
  start_time = 0
  automatic_scaling = true
[]

[Postprocessors]
  [matl_ts_min]
    type = MaterialTimeStepPostprocessor
  []
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  []
  [max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [dt]
    type = TimestepSize
  []
  [num_lin]
    type = NumLinearIterations
    outputs = console
  []
  [num_nonlin]
    type = NumNonlinearIterations
    outputs = console
  []
  [creep_strain_xy]
    type = ElementAverageValue
    variable = creep_strain_xy
    execute_on = 'TIMESTEP_END'
  []
  [elastic_strain_xy]
    type = ElementAverageValue
    variable = elastic_strain_xy
    execute_on = 'TIMESTEP_END'
  []
  [sigma_xy]
    type = ElementAverageValue
    variable = stress_xy
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  exodus = false
  perf_graph = true
  # unnecessary output variables
  hide = 'matl_ts_min max_disp_x max_disp_y max_hydro dt num_lin num_nonlin'
[]

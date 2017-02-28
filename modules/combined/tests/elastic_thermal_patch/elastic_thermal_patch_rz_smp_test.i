#
# This problem is modified from the Abaqus verification manual:
#   "1.5.4 Patch test for axisymmetric elements"
# The original stress solution is given as:
#   xx = yy = zz = 2000
#   xy = 400
#
# Here, E=1e6 and nu=0.25.
# However, with a +100 degree change in temperature and a coefficient
#   of thermal expansion of 1e-6, the solution becomes:
#   xx = yy = zz = 1800
#   xy = 400
#   since
#   E*(1-nu)/(1+nu)/(1-2*nu)*(1+2*nu/(1-nu))*(1e-3-1e-4) = 1800
#
# Also,
#
#   dSrr   dSrz   Srr-Stt
#   ---- + ---- + ------- + br = 0
#    dr     dz       r
#
# and
#
#   dSrz   Srz   dSzz
#   ---- + --- + ---- + bz = 0
#    dr     r     dz
#
# where
#   Srr = stress in rr
#   Szz = stress in zz
#   Stt = stress in theta-theta
#   Srz = stress in rz
#   br  = body force in r direction
#   bz  = body force in z direction
#
# This test is meant to exercise the Jacobian.  To that end, the body
# force has been turned off.  This makes the results differ slightly
# from the original values, but requires a correct Jacobian for minimal
# iterations.  Iteration plotting is turned on to ensure that the
# number of iterations needed does not increase.

[GlobalParams]
  temperature = temp
  volumetric_locking_correction = true
  order = FIRST
  family = LAGRANGE
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = elastic_thermal_patch_rz_test.e
[]

[Functions]
  [./ur]
    type = ParsedFunction
    value = '1e-3*x'
  [../]
  [./uz]
    type = ParsedFunction
    value = '1e-3*(x+y)'
  [../]
  [./body]
    type = ParsedFunction
    value = '-400/x'
  [../]
  [./temp]
    type = ParsedFunction
    value = '117.56+100*t'
  [../]
[]

[Variables]
  [./disp_x]
  [../]

  [./disp_y]
  [../]

  [./temp]
    initial_condition = 117.56
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
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
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
  [../]
[]

[BCs]
  [./ur]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 10
    function = ur
  [../]
  [./uz]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 10
    function = uz
  [../]

  [./temp]
    type = FunctionPresetBC
    variable = temp
    boundary = 10
    function = temp
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 666666.6666666667
    poissons_ratio = 0.25
  [../]

  [./strain]
    type = ComputeAxisymmetricRZIncrementalStrain
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]

  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-6
    stress_free_temperature = 117.56
    incremental_form = true
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeStrainIncrementBasedStress
  [../]

  [./heat]
    type = HeatConductionMaterial
    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
    disp_r = disp_x
    disp_z = disp_y
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'
  line_search = 'none'

  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-12
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[]

[Outputs]
  file_base = elastic_thermal_patch_rz_smp_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[]

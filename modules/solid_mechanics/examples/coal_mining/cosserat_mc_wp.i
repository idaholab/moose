# Strata deformation and fracturing around a coal mine
#
# A 2D geometry is used that simulates a transverse section of
# the coal mine.  The model is actually 3D, but the "x"
# dimension is only 10m long, meshed with 1 element, and
# there is no "x" displacement.  The mine is 300m deep
# and just the roof is studied (0<=z<=300).  The model sits
# between 0<=y<=450.  The excavation sits in 0<=y<=150.  This
# is a "half model": the boundary conditions are such that
# the model simulates an excavation sitting in -150<=y<=150
# inside a model of the region -450<=y<=450.  The
# excavation height is 3m (ie, the excavation lies within
# 0<=z<=3).  Mining is simulated by moving the excavation's
# roof down, until disp_z=-3 at t=1.
# Time is meaningless in this example
# as quasi-static solutions are sought at each timestep, but
# the number of timesteps controls the resolution of the
# process.
#
# The boundary conditions are:
#  - disp_x = 0 everywhere
#  - disp_y = 0 at y=0 and y=450
#  - disp_z = 0 for y>150
#  - disp_z = -3 at maximum, for 0<=y<=150.  See excav function.
# That is, rollers on the sides, free at top, and prescribed at bottom.
#
# The small strain formulation is used.
#
# All stresses are measured in MPa.  The initial stress is consistent with
# the weight force from density 2500 kg/m^3, ie, stress_zz = -0.025*(300-z) MPa
# where gravity = 10 m.s^-2 = 1E-5 MPa m^2/kg.  The maximum and minimum
# principal horizontal stresses are assumed to be equal to 0.8*stress_zz.
#
# Material properties:
# Young's modulus = 8 GPa
# Poisson's ratio = 0.25
# Cosserat layer thickness = 1 m
# Cosserat-joint normal stiffness = large
# Cosserat-joint shear stiffness = 1 GPa
# MC cohesion = 3 MPa
# MC friction angle = 37 deg
# MC dilation angle = 8 deg
# MC tensile strength = 1 MPa
# MC compressive strength = 100 MPa, varying down to 1 MPa when tensile strain = 1
# WeakPlane cohesion = 0.1 MPa
# WeakPlane friction angle = 30 deg
# WeakPlane dilation angle = 10 deg
# WeakPlane tensile strength = 0.1 MPa
# WeakPlane compressive strength = 100 MPa softening to 1 MPa at strain = 1
#
[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    xmin = -5
    xmax = 5
    nz = 40
    zmin = 0
    zmax = 400.0
    bias_z = 1.1
    ny = 30 # make this a multiple of 3, so y=150 is at a node
    ymin = 0
    ymax = 450
  []
  [left]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 11
    normal = '0 -1 0'
    input = generated_mesh
  []
  [right]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 12
    normal = '0 1 0'
    input = left
  []
  [front]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 13
    normal = '-1 0 0'
    input = right
  []
  [back]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 14
    normal = '1 0 0'
    input = front
  []
  [top]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 15
    normal = '0 0 1'
    input = back
  []
  [bottom]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 16
    normal = '0 0 -1'
    input = top
  []
  [excav]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '-5 0 0'
    top_right = '5 150 3'
    input = bottom
  []
  [roof]
    type = SideSetsBetweenSubdomainsGenerator
    new_boundary = 21
    primary_block = 0
    paired_block = 1
    input = excav
  []
  [hole]
    type = BlockDeletionGenerator
    block = 1
    input = roof
  []
[]

[GlobalParams]
  block = 0
  perform_finite_strain_rotations = false
  displacements = 'disp_x disp_y disp_z'
  Cosserat_rotations = 'wc_x wc_y wc_z'
[]

[Variables]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./wc_x]
  [../]
[]

[Kernels]
  [./cy_elastic]
    type = CosseratStressDivergenceTensors
    use_displaced_mesh = false
    variable = disp_y
    component = 1
  [../]
  [./cz_elastic]
    type = CosseratStressDivergenceTensors
    use_displaced_mesh = false
    variable = disp_z
    component = 2
  [../]
  [./x_couple]
    type = StressDivergenceTensors
    use_displaced_mesh = false
    variable = wc_x
    displacements = 'wc_x wc_y wc_z'
    component = 0
    base_name = couple
  [../]
  [./x_moment]
    type = MomentBalancing
    use_displaced_mesh = false
    variable = wc_x
    component = 0
  [../]
  [./gravity]
    type = Gravity
    use_displaced_mesh = false
    variable = disp_z
    value = -10E-6
  [../]
[]


[AuxVariables]
  [./disp_x]
  [../]
  [./wc_y]
  [../]
  [./wc_z]
  [../]
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
  [./mc_shear]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mc_tensile]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./wp_shear]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./wp_tensile]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./wp_shear_f]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./wp_tensile_f]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mc_shear_f]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mc_tensile_f]
    order = CONSTANT
    family = MONOMIAL
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
  [./mc_shear]
    type = MaterialStdVectorAux
    index = 0
    property = mc_plastic_internal_parameter
    variable = mc_shear
  [../]
  [./mc_tensile]
    type = MaterialStdVectorAux
    index = 1
    property = mc_plastic_internal_parameter
    variable = mc_tensile
  [../]
  [./wp_shear]
    type = MaterialStdVectorAux
    index = 0
    property = wp_plastic_internal_parameter
    variable = wp_shear
  [../]
  [./wp_tensile]
    type = MaterialStdVectorAux
    index = 1
    property = wp_plastic_internal_parameter
    variable = wp_tensile
  [../]
  [./mc_shear_f]
    type = MaterialStdVectorAux
    index = 6
    property = mc_plastic_yield_function
    variable = mc_shear_f
  [../]
  [./mc_tensile_f]
    type = MaterialStdVectorAux
    index = 0
    property = mc_plastic_yield_function
    variable = mc_tensile_f
  [../]
  [./wp_shear_f]
    type = MaterialStdVectorAux
    index = 0
    property = wp_plastic_yield_function
    variable = wp_shear_f
  [../]
  [./wp_tensile_f]
    type = MaterialStdVectorAux
    index = 1
    property = wp_plastic_yield_function
    variable = wp_tensile_f
  [../]
[]



[BCs]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = '11 12 16 21' # note addition of 16 and 21
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = '16'
    value = 0.0
  [../]
  [./no_wc_x]
    type = DirichletBC
    variable = wc_x
    boundary = '11 12'
    value = 0.0
  [../]
  [./roof]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 21
    function = excav_sideways
  [../]
[]

[Functions]
  [./ini_xx]
    type = ParsedFunction
    expression = '-0.8*2500*10E-6*(400-z)'
  [../]
  [./ini_zz]
    type = ParsedFunction
    expression = '-2500*10E-6*(400-z)'
  [../]
  [./excav_sideways]
    type = ParsedFunction
    symbol_names = 'end_t ymin ymax  e_h  closure_dist'
    symbol_values = '1.0   0    150.0 -3.0 15.0'
    expression = 'e_h*max(min((min(t/end_t,1)*(ymax-ymin)+ymin-y)/closure_dist,1),0)'
  [../]
  [./excav_downwards]
    type = ParsedFunction
    symbol_names = 'end_t ymin ymax  e_h  closure_dist'
    symbol_values = '1.0   0    150.0 -3.0 15.0'
    expression = 'e_h*min(t/end_t,1)*max(min(((ymax-ymin)+ymin-y)/closure_dist,1),0)'
  [../]
[]

[UserObjects]
  [./mc_coh_strong_harden]
    type = TensorMechanicsHardeningExponential
    value_0 = 2.99 # MPa
    value_residual = 3.01 # MPa
    rate = 1.0
  [../]
  [./mc_fric]
    type = TensorMechanicsHardeningConstant
    value = 0.65 # 37deg
  [../]
  [./mc_dil]
    type = TensorMechanicsHardeningConstant
    value = 0.15 # 8deg
  [../]

  [./mc_tensile_str_strong_harden]
    type = TensorMechanicsHardeningExponential
    value_0 = 1.0 # MPa
    value_residual = 1.0 # MPa
    rate = 1.0
  [../]
  [./mc_compressive_str]
    type = TensorMechanicsHardeningCubic
    value_0 = 100 # Large!
    value_residual = 100
    internal_limit = 0.1
  [../]

  [./wp_coh_harden]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.1
    value_residual = 0.1
    internal_limit = 10
  [../]
  [./wp_tan_fric]
    type = TensorMechanicsHardeningConstant
    value = 0.36 # 20deg
  [../]
  [./wp_tan_dil]
    type = TensorMechanicsHardeningConstant
    value = 0.18 # 10deg
  [../]

  [./wp_tensile_str_harden]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.1
    value_residual = 0.1
    internal_limit = 10
  [../]
  [./wp_compressive_str_soften]
    type = TensorMechanicsHardeningCubic
    value_0 = 100
    value_residual = 1
    internal_limit = 1.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 8E3 # MPa
    poisson = 0.25
    layer_thickness = 1.0
    joint_normal_stiffness = 1E9 # huge
    joint_shear_stiffness = 1E3
  [../]

  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = 'ini_xx 0 0  0 ini_xx 0  0 0 ini_zz'
    eigenstrain_name = ini_stress
  [../]

  [./stress]
    type = ComputeMultipleInelasticCosseratStress
    block = 0
    inelastic_models = 'mc wp'
    cycle_models = true
    relative_tolerance = 2.0
    absolute_tolerance = 1E6
    max_iterations = 1
    tangent_operator = nonlinear
    perform_finite_strain_rotations = false
  [../]
  [./mc]
    type = CappedMohrCoulombCosseratStressUpdate
    block = 0
    warn_about_precision_loss = false
    host_youngs_modulus = 8E3
    host_poissons_ratio = 0.25
    base_name = mc
    tensile_strength = mc_tensile_str_strong_harden
    compressive_strength = mc_compressive_str
    cohesion = mc_coh_strong_harden
    friction_angle = mc_fric
    dilation_angle = mc_dil
    max_NR_iterations = 10000
    smoothing_tol = 0.1 # MPa  # Must be linked to cohesion
    yield_function_tol = 1E-9 # MPa.  this is essentially the lowest possible without lots of precision loss
    perfect_guess = true
    min_step_size = 1.0
  [../]
  [./wp]
    type = CappedWeakPlaneCosseratStressUpdate
    block = 0
    warn_about_precision_loss = false
    base_name = wp
    cohesion = wp_coh_harden
    tan_friction_angle = wp_tan_fric
    tan_dilation_angle = wp_tan_dil
    tensile_strength = wp_tensile_str_harden
    compressive_strength = wp_compressive_str_soften
    max_NR_iterations = 10000
    tip_smoother = 0.1
    smoothing_tol = 0.1 # MPa  # Note, this must be tied to cohesion, otherwise get no possible return at cone apex
    yield_function_tol = 1E-11 # MPa.  this is essentially the lowest possible without lots of precision loss
    perfect_guess = true
    min_step_size = 1.0
  [../]


  [./density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 2500
  [../]
[]

[Postprocessors]
  [./subsidence]
    type = PointValue
    point = '0 0 400'
    variable = disp_z
    use_displaced_mesh = false
  [../]
[]
[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'

  line_search = bt

  nl_abs_tol = 1e-3
  nl_rel_tol = 1e-5

  l_max_its = 30
  nl_max_its = 1000

  start_time = 0.0
  dt = 0.2
  end_time = 0.2

[]

[Outputs]
  file_base = cosserat_mc_wp
  interval = 1
  print_linear_residuals = false
  csv = true
  exodus = true
  [./console]
    type = Console
    output_linear = false
  [../]
[]

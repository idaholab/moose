# Uses an Abbo et al smoothed version of Mohr-Coulomb (via TensorMechanicsPlasticMohrCoulomb and ComputeMultiPlasticityStress) to simulate the following problem.
# A cubical block is notched around its equator.
# All of its outer surfaces have roller BCs, but the notched region is free to move as needed
# The block is initialised with a high hydrostatic tensile stress
# Without the notch, the BCs do not allow contraction of the block, and this stress configuration is admissible
# With the notch, however, the interior parts of the block are free to move in order to relieve stress, and this causes plastic failure
# The top surface is then pulled upwards (the bottom is fixed because of the roller BCs)
# This causes more failure
[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 9
    ny = 9
    nz = 9
    xmin = 0
    xmax = 0.1
    ymin = 0
    ymax = 0.1
    zmin = 0
    zmax = 0.1
  []
  [block_to_remove_xmin]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '-0.01 -0.01 0.045'
    top_right = '0.01 0.11 0.055'
    location = INSIDE
    block_id = 1
    input = generated_mesh
  []
  [block_to_remove_xmax]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.09 -0.01 0.045'
    top_right = '0.11 0.11 0.055'
    location = INSIDE
    block_id = 1
    input = block_to_remove_xmin
  []
  [block_to_remove_ymin]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '-0.01 -0.01 0.045'
    top_right = '0.11 0.01 0.055'
    location = INSIDE
    block_id = 1
    input = block_to_remove_xmax
  []
  [block_to_remove_ymax]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '-0.01 0.09 0.045'
    top_right = '0.11 0.11 0.055'
    location = INSIDE
    block_id = 1
    input = block_to_remove_ymin
  []
  [remove_block]
    type = BlockDeletionGenerator
    block = 1
    input = block_to_remove_ymax
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    incremental = true
    generate_output = 'max_principal_stress mid_principal_stress min_principal_stress stress_zz'
    eigenstrain_names = ini_stress
  [../]
[]

[Postprocessors]
  [./uz]
    type = PointValue
    point = '0 0 0.1'
    use_displaced_mesh = false
    variable = disp_z
  [../]
  [./s_zz]
    type = ElementAverageValue
    use_displaced_mesh = false
    variable = stress_zz
  [../]
  [./num_res]
    type = NumResidualEvaluations
  [../]
  [./nr_its] # num_iters is the average number of NR iterations encountered per element in this timestep
    type = ElementAverageValue
    variable = num_iters
  [../]
  [./max_nr_its] # num_iters is the average number of NR iterations encountered in the element in this timestep, so we must get max(max_nr_its) to obtain the max number of iterations
    type = ElementExtremeValue
    variable = num_iters
  [../]
  [./runtime]
    type = PerfGraphData
    data_type = TOTAL
    section_name = 'Root'
  [../]
[]

[BCs]
  # back=zmin, front=zmax, bottom=ymin, top=ymax, left=xmin, right=xmax
  [./xmin_xzero]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./xmax_xzero]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  [../]
  [./ymin_yzero]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./ymax_yzero]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  [../]
  [./zmin_zzero]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = '0'
  [../]
  [./zmax_disp]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '1E-6*max(t,0)'
  [../]
[]

[AuxVariables]
  [./mc_int]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./num_iters]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./mc_int_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = mc_int
  [../]
  [./num_iters_auxk]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = num_iters
  [../]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_yield_function
    variable = yield_fcn
  [../]
[]

[UserObjects]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 5E6
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 35
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 10
    convert_to_radians = true
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulomb
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    mc_tip_smoother = 0.02E6
    mc_edge_smoother = 29
    yield_function_tolerance = 1E-5
    internal_constraint_tolerance = 1E-11
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 16E9
    poissons_ratio = 0.25
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-11
    plastic_models = mc
    max_NR_iterations = 1000
    debug_fspb = crash
  [../]
  [./strain_from_initial_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '6E6 0 0  0 6E6 0  0 0 6E6'
    eigenstrain_name = ini_stress
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  start_time = -1
  end_time = 10
  dt = 1
  solve_type = NEWTON
  type = Transient

  l_tol = 1E-2
  nl_abs_tol = 1E-5
  nl_rel_tol = 1E-7
  l_max_its = 200
  nl_max_its = 400

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
[]


[Outputs]
  file_base = biaxial_abbo
  perf_graph = true
  exodus = false
  csv = true
[]

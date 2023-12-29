# Part of the bottom (minimum z) is pulled down by a Preset displacement
# This causes tensile failure in the elements immediately above.
# Because only the bottom row of elements ever fail, and because these
# fail in the first nonlinear step, Moose correctly converges in
# 1 nonlinear step, despite this problem being inelastic.
# (If the problem had lower cohesion, then the top row would also
# fail, but in the second nonlinear step, and so the simulation
# would require at least two nonlinear steps.)
[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 2
    xmin = -10
    xmax = 10
    ymin = -10
    ymax = 10
    zmin = -100
    zmax = 0
  []
  [bottomz_middle]
    type = BoundingBoxNodeSetGenerator
    new_boundary = bottomz_middle
    bottom_left = '-1 -15 -105'
    top_right = '1 15 -95'
    input = generated_mesh
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[BCs]
  [./no_x2]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  [../]
  [./no_x1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./no_y1]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./no_y2]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  [../]

  [./z_fixed_sides_xmin]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  [../]
  [./z_fixed_sides_xmax]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0
  [../]

  [./bottomz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = bottomz_middle
    function = -1
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_shear]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_tensile]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_compressive]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl_shear]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl_tensile]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ls]
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
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./strainp_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_xx
    index_i = 0
    index_j = 0
  [../]
  [./strainp_xy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_xy
    index_i = 0
    index_j = 1
  [../]
  [./strainp_xz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_xz
    index_i = 0
    index_j = 2
  [../]
  [./strainp_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_yy
    index_i = 1
    index_j = 1
  [../]
  [./strainp_yz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_yz
    index_i = 1
    index_j = 2
  [../]
  [./strainp_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_zz
    index_i = 2
    index_j = 2
  [../]
  [./straint_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_xx
    index_i = 0
    index_j = 0
  [../]
  [./straint_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_xy
    index_i = 0
    index_j = 1
  [../]
  [./straint_xz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_xz
    index_i = 0
    index_j = 2
  [../]
  [./straint_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_yy
    index_i = 1
    index_j = 1
  [../]
  [./straint_yz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_yz
    index_i = 1
    index_j = 2
  [../]
  [./straint_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_zz
    index_i = 2
    index_j = 2
  [../]
  [./f_shear]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f_shear
  [../]
  [./f_tensile]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f_tensile
  [../]
  [./f_compressive]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f_compressive
  [../]
  [./intnl_shear]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = intnl_shear
  [../]
  [./intnl_tensile]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = intnl_tensile
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./ls]
    type = MaterialRealAux
    property = plastic_linesearch_needed
    variable = ls
  [../]
[]

[UserObjects]
  [./coh_irrelevant]
    type = TensorMechanicsHardeningCubic
    value_0 = 1E60
    value_residual = 1E60
    internal_limit = 0.01E8
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.5
    value_residual = 0.2
    internal_limit = 0.01E8
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.166666666667
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningConstant
    value = 0
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = 1E80
    value_residual = 1E80
    internal_limit = 0.01
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '6.4E9 6.4E9'  # young 16MPa, Poisson 0.25
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = stress
    tangent_operator = nonlinear
    perform_finite_strain_rotations = false
  [../]
  [./stress]
    type = CappedWeakPlaneStressUpdate
    cohesion = coh_irrelevant
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    max_NR_iterations = 1
    tip_smoother = 0
    smoothing_tol = 0
    yield_function_tol = 1E-2
    perfect_guess = true
    min_step_size = 1
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -snes_linesearch_monitor'
    petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
    petsc_options_value = ' asm      2              lu            gmres     200'
  [../]
[]

[Executioner]
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason'
  line_search = bt
  nl_abs_tol = 1E1
  nl_rel_tol = 1e-5
  l_tol = 1E-10

  l_max_its = 100
  nl_max_its = 100

  end_time = 1.0
  dt = 1.0
  type = Transient
[]

[Outputs]
  file_base = pull_and_shear_1step
  exodus = true
[]

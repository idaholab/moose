# Apply an initial stress that should be
# exactly that caused by gravity, and then
# do a transient step to check that nothing
# happens
# TODO: currently this has no div(moment_stress)
# contriution to the Kernels.  This is because
# there is no way in MOOSE of calculating
# moment stresses and applying initial stresses.
# This will become possible after issue #7243 is
# resolved.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 10
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -10
  zmax = 0
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  Cosserat_rotations = 'wc_x wc_y wc_z'
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./wc_x]
  [../]
  [./wc_y]
  [../]
  [./wc_z]
  [../]
[]

[Kernels]
  [./cx_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./cy_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./cz_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./x_moment]
    type = MomentBalancing
    variable = wc_x
    component = 0
  [../]
  [./y_moment]
    type = MomentBalancing
    variable = wc_y
    component = 1
  [../]
  [./z_moment]
    type = MomentBalancing
    variable = wc_z
    component = 2
  [../]
  [./weight]
    type = BodyForce
    variable = disp_z
    value = -0.5 # this is density*gravity
  [../]
[]


[BCs]
  # back = zmin
  # front = zmax
  # bottom = ymin
  # top = ymax
  # left = xmin
  # right = xmax
  [./x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0
  [../]
  [./y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom top'
    value = 0
  [../]
  [./z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
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
[]


[Functions]
  [./weight]
    type = ParsedFunction
    expression = '0.5*z' # initial stress that should result from the weight force
  [../]
  [./kxx]
    type = ParsedFunction
    expression = '0.4*z' # some arbitrary xx and yy stress that should not affect the result
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeCosseratElasticityTensor
    B_ijkl = '1.1 0.6 0.6' # In Forest notation this is alpha=1.1 (this is unimportant), beta=gamma=0.6.
    fill_method_bending = 'general_isotropic'
    fill_method = symmetric_isotropic
    E_ijkl = '0.4 0.4' # young = 1, poisson = 0.25
  [../]
  [./strain]
    type = ComputeCosseratSmallStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = 'kxx 0 0  0 kxx 0  0 0 weight'
    eigenstrain_name = ini_stress
  [../]
  [./stress]
    type = ComputeCosseratLinearElasticStress
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]


[Executioner]
  end_time = 1.0
  dt = 1.0
  solve_type = NEWTON
  type = Transient

  nl_abs_tol = 1E-8
  nl_rel_tol = 1E-12
  l_tol = 1E-3
  l_max_its = 200
  nl_max_its = 400

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
[]



[Outputs]
  file_base = gravity_cosserat
  exodus = true
[]

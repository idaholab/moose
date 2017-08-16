# Apply an initial stress that should be
# exactly that caused by gravity, and then
# do a transient step to check that nothing
# happens

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
    displacements = 'disp_x disp_y disp_z'
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
    type = PresetBC
    variable = disp_x
    boundary = 'left right'
    value = 0
  [../]
  [./y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom top'
    value = 0
  [../]
  [./z]
    type = PresetBC
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
  [./yield_fcn]
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
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
  [../]
[]


[Functions]
  [./weight]
    type = ParsedFunction
    value = '0.5*z' # initial stress that should result from the weight force
  [../]
  [./kxx]
    type = ParsedFunction
    value = '0.4*z' # some arbitrary xx and yy stress that should not affect the result
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1E6
  [../]
  [./mc]
    type = TensorMechanicsPlasticTensile
    tensile_strength = ts
    yield_function_tolerance = 1E-6
    tensile_tip_smoother = 1.0
    internal_constraint_tolerance = 1E-5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0.4 0.4' # young = 1, poisson = 0.25
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    initial_stress = 'kxx 0 0  0 kxx 0  0 0 weight'

    # the rest of this stuff is irrelevant for this test
    ep_plastic_tolerance = 1E-5
    plastic_models = mc
    debug_fspb = crash
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
  file_base = gravity
  exodus = true
[]

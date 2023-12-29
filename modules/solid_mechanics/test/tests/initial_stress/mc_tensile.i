# In this example, an initial stress is applied that
# is inadmissible, and the return-map algorithm must be
# used to return to the yield surface before any other
# computations can be carried out.
# In this case, the return-map algorithm must subdivide
# the initial stress, otherwise it does not converge.
# This test is testing that subdivision process.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
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
[]


[BCs]
  [./bottomx]
    type = DirichletBC
    variable = disp_x
    boundary = 'back'
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    variable = disp_y
    boundary = 'back'
    value = 0.0
  [../]
  [./bottomz]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  [../]
  [./topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front'
    function = '2*t-1'
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front'
    function = 't-1'
  [../]
  [./topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front'
    function = 't-1'
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
  [./iter]
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
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./s_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
    outputs = console
  [../]
[]

[UserObjects]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 1E5
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 60
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 5
    convert_to_radians = true
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulomb
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    mc_tip_smoother = 4.0
    mc_edge_smoother = 25
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]

  [./str]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./pt]
    type = TensorMechanicsPlasticTensile
    tensile_strength = str
    yield_function_tolerance = 1E-3
    tensile_tip_smoother = 0.05
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '8E6 4E6 -18E6 4E6 -40E6 -2E6 -18E6 -2E6 -34E6'
    eigenstrain_name = ini_stress
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-9
    plastic_models = 'pt mc'
    deactivation_scheme = safe
    max_NR_iterations = 100
    min_stepsize = 0.1
  [../]
[]


[Executioner]
  end_time = 2
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = mc_tensile
  exodus = false
  [./csv]
    type = CSV
    [../]
[]

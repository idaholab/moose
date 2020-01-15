# Using Cosserat with large layer thickness, so this should reduce to standard
# Using CappedMohrCoulombCosserat with tensile failure only
# A single unit element is stretched in a complicated way that
# the trial stress is
#        1.51515             0.8        0.666667
#            0.8        -3.74545    -1.85037e-17
#            0.7    -1.66533e-17        -1.27273
# with symmetric part
#        1.51515             0.8        0.6833
#            0.8        -3.74545    -1.85037e-17
#            0.6833  -1.66533e-17      -1.27273
#
# This has eigenvalues
# la = {-3.86844, 1.78368, -1.41827}
# and eigenvectors
#
# {0.15183, -0.987598, -0.03997},
# {-0.966321, -0.139815, -0.216044},
# {-0.207777, -0.0714259, 0.975565}}
#
# The tensile strength is 0.5 and Young=1 and Poisson=0.25,
# with E_0000/E_0011 = nu / (1 - nu) = 0.333333
# Using smoothing_tol=0.01, the return-map algorithm should
# return to stress_I = 0.5, which is a reduction of 1.28368, so
# stress_II = -1.41827 - 1.28368 * 0.33333 = -1.846
# stress_III = -3.86844 - 1.28368 * 0.33333 = -4.296
#
# The final stress symmetric stress is
#
# {0.29, 0.69, 0.51},
# {0.69, -4.19, -0.03},
# {0.51, -0.03, -1.74}
#
# and a final unsymmetric stress of
#
# {0.29, 0.69, 0.49},
# {0.69, -4.19, -0.03},
# {0.52, -0.03, -1.74}

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
  [./x_couple]
    type = StressDivergenceTensors
    variable = wc_x
    displacements = 'wc_x wc_y wc_z'
    component = 0
    base_name = couple
  [../]
  [./y_couple]
    type = StressDivergenceTensors
    variable = wc_y
    displacements = 'wc_x wc_y wc_z'
    component = 1
    base_name = couple
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
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '3*x-y+z'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '3*x-4*y'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = 'x-2*z'
  [../]
  [./wc_x]
    type = DirichletBC
    variable = wc_x
    boundary = 'front back'
    value = 0.0
  [../]
  [./wc_y]
    type = DirichletBC
    variable = wc_y
    boundary = 'front back'
    value = 0.0
  [../]
[]

[AuxVariables]
  [./wc_z]
  [../]
  [./stress_I]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_II]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_III]
    order = CONSTANT
    family = MONOMIAL
  [../]
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
  [./stress_yx]
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
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_I]
    type = RankTwoScalarAux
    scalar_type = MaxPrincipal
    rank_two_tensor = stress
    variable = stress_I
    selected_qp = 0
  [../]
  [./stress_II]
    type = RankTwoScalarAux
    scalar_type = MidPrincipal
    rank_two_tensor = stress
    variable = stress_II
    selected_qp = 0
  [../]
  [./stress_III]
    type = RankTwoScalarAux
    scalar_type = MinPrincipal
    rank_two_tensor = stress
    variable = stress_III
    selected_qp = 0
  [../]
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
  [./stress_yx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yx
    index_i = 1
    index_j = 0
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
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
  [../]
  [./stress_zy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zy
    index_i = 2
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./f0_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f0
  [../]
  [./f1_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f1
  [../]
  [./f2_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f2
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./intnl_auxk]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = intnl
  [../]
[]

[Postprocessors]
  [./s_I]
    type = PointValue
    point = '0 0 0'
    variable = stress_I
  [../]
  [./s_II]
    type = PointValue
    point = '0 0 0'
    variable = stress_II
  [../]
  [./s_III]
    type = PointValue
    point = '0 0 0'
    variable = stress_III
  [../]
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
  [./s_yx]
    type = PointValue
    point = '0 0 0'
    variable = stress_yx
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
  [./s_zx]
    type = PointValue
    point = '0 0 0'
    variable = stress_zx
  [../]
  [./s_zy]
    type = PointValue
    point = '0 0 0'
    variable = stress_zy
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./f0]
    type = PointValue
    point = '0 0 0'
    variable = f0
  [../]
  [./f1]
    type = PointValue
    point = '0 0 0'
    variable = f1
  [../]
  [./f2]
    type = PointValue
    point = '0 0 0'
    variable = f2
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  [../]
  [./intnl]
    type = PointValue
    point = '0 0 0'
    variable = intnl
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1E6
  [../]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 1E6
  [../]
  [./ang]
    type = TensorMechanicsHardeningConstant
    value = 30
    convert_to_radians = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 1
    poisson = 0.25
    layer_thickness = 1.0
    joint_normal_stiffness = 1.0
    joint_shear_stiffness = 2.0
  [../]
  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
  [../]
  [./tensile]
    type = CappedMohrCoulombCosseratStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = ang
    dilation_angle = ang
    smoothing_tol = 0.001
    yield_function_tol = 1.0E-12
    host_youngs_modulus = 1.0
    host_poissons_ratio = 0.25
  [../]
  [./stress]
    type = ComputeMultipleInelasticCosseratStress
    inelastic_models = tensile
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  nl_abs_tol = 1E-10
  type = Transient
[]


[Outputs]
  file_base = small_deform9_cosserat
  csv = true
[]

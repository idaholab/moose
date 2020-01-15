# Plastic deformation.  Layered Cosserat with parameters:
# Young = 1.0
# Poisson = 0.2
# layer_thickness = 0.1
# joint_normal_stiffness = 0.25
# joint_shear_stiffness = 0.2
# These give the following nonzero components of the elasticity tensor:
# E_0000 = E_1111 = 1.043195
# E_0011 = E_1100 = 0.260799
# E_2222 = 0.02445
# E_0022 = E_1122 = E_2200 = E_2211 = 0.006112
# G = E_0101 = E_0110 = E_1001 = E_1010 = 0.416667
# Gt = E_0202 = E_0220 = E_2002 = E_1212 = E_1221 = E_2112 = 0.019084
# E_2020 = E_2121 = 0.217875
# They give the following nonzero components of the bending rigidity tensor:
# D = 8.68056E-5
# B_0101 = B_1010 = 7.92021E-4
# B_0110 = B_1001 = -1.584E-4
#
# Applying the following deformation to the zmax surface of a unit cube:
# disp_x = 8*t
# disp_y = 6*t
# disp_z = -t
# omega_x = omega_y = omega_z = 0
# yields the following strains:
# strain_xz = 8*t
# strain_yz = 6*t
# strain_zz = -t
# and all other components, and the curvature, are zero.
# The nonzero components of stress are therefore:
# stress_xx = stress_yy = -0.006112*t
# stress_xz = stress_zx = 0.152671*t
# stress_yz = stress_zy = 0.114504*t
# stress_zz = -0.0244499*t
# The moment stress is zero.
# So q = 0.19084*t and p = -0.0244*t.
#
# With large cohesion, but compressive strength = 0.0244499, the
# system is elastic up to t=1.  After that time
# stress_zz = -0.0244499 (for t>=1)
# and
# stress_xx = stress_yy = -0.006112 (for t>=1), since the
# elastic trial increment is exactly canelled by the Poisson's
# contribution from the return to the yield surface.
# The plastic strains are zero for t<=1, but for larger times:
# plastic_strain_zz = - (t - 1)  (for t>=1)
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
  [./bottomx]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    variable = disp_y
    boundary = back
    value = 0.0
  [../]
  [./bottomz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]

  [./topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = front
    function = 8*t
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = front
    function = 6*t
  [../]
  [./topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = -t
  [../]
[]

[AuxVariables]
  [./wc_z]
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
  [./couple_stress_xx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_xy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_xz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_yx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_yy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_yz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_zx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_zy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./couple_stress_zz]
    family = MONOMIAL
    order = CONSTANT
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
  [./strainp_yx]
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
  [./strainp_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_zy]
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
  [./straint_yx]
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
  [./straint_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./straint_zy]
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
  [./couple_stress_xx]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./couple_stress_xy]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./couple_stress_xz]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./couple_stress_yx]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_yx
    index_i = 1
    index_j = 0
  [../]
  [./couple_stress_yy]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./couple_stress_yz]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./couple_stress_zx]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_zx
    index_i = 2
    index_j = 0
  [../]
  [./couple_stress_zy]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_zy
    index_i = 2
    index_j = 1
  [../]
  [./couple_stress_zz]
    type = RankTwoAux
    rank_two_tensor = couple_stress
    variable = couple_stress_zz
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
  [./strainp_yx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_yx
    index_i = 1
    index_j = 0
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
  [./strainp_zx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_zx
    index_i = 2
    index_j = 0
  [../]
  [./strainp_zy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_zy
    index_i = 2
    index_j = 1
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
  [./straint_yx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_yx
    index_i = 1
    index_j = 0
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
  [./straint_zx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_zx
    index_i = 2
    index_j = 0
  [../]
  [./straint_zy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_zy
    index_i = 2
    index_j = 1
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
  [./c_s_xx]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_xx
  [../]
  [./c_s_xy]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_xy
  [../]
  [./c_s_xz]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_xz
  [../]
  [./c_s_yx]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_yx
  [../]
  [./c_s_yy]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_yy
  [../]
  [./c_s_yz]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_yz
  [../]
  [./c_s_zx]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_zx
  [../]
  [./c_s_zy]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_zy
  [../]
  [./c_s_zz]
    type = PointValue
    point = '0 0 0'
    variable = couple_stress_zz
  [../]
  [./strainp_xx]
    type = PointValue
    point = '0 0 0'
    variable = strainp_xx
  [../]
  [./strainp_xy]
    type = PointValue
    point = '0 0 0'
    variable = strainp_xy
  [../]
  [./strainp_xz]
    type = PointValue
    point = '0 0 0'
    variable = strainp_xz
  [../]
  [./strainp_yx]
    type = PointValue
    point = '0 0 0'
    variable = strainp_yx
  [../]
  [./strainp_yy]
    type = PointValue
    point = '0 0 0'
    variable = strainp_yy
  [../]
  [./strainp_yz]
    type = PointValue
    point = '0 0 0'
    variable = strainp_yz
  [../]
  [./strainp_zx]
    type = PointValue
    point = '0 0 0'
    variable = strainp_zx
  [../]
  [./strainp_zy]
    type = PointValue
    point = '0 0 0'
    variable = strainp_zy
  [../]
  [./strainp_zz]
    type = PointValue
    point = '0 0 0'
    variable = strainp_zz
  [../]
  [./straint_xx]
    type = PointValue
    point = '0 0 0'
    variable = straint_xx
  [../]
  [./straint_xy]
    type = PointValue
    point = '0 0 0'
    variable = straint_xy
  [../]
  [./straint_xz]
    type = PointValue
    point = '0 0 0'
    variable = straint_xz
  [../]
  [./straint_yx]
    type = PointValue
    point = '0 0 0'
    variable = straint_yx
  [../]
  [./straint_yy]
    type = PointValue
    point = '0 0 0'
    variable = straint_yy
  [../]
  [./straint_yz]
    type = PointValue
    point = '0 0 0'
    variable = straint_yz
  [../]
  [./straint_zx]
    type = PointValue
    point = '0 0 0'
    variable = straint_zx
  [../]
  [./straint_zy]
    type = PointValue
    point = '0 0 0'
    variable = straint_zy
  [../]
  [./straint_zz]
    type = PointValue
    point = '0 0 0'
    variable = straint_zz
  [../]
  [./f_shear]
    type = PointValue
    point = '0 0 0'
    variable = f_shear
  [../]
  [./f_tensile]
    type = PointValue
    point = '0 0 0'
    variable = f_tensile
  [../]
  [./f_compressive]
    type = PointValue
    point = '0 0 0'
    variable = f_compressive
  [../]
  [./intnl_shear]
    type = PointValue
    point = '0 0 0'
    variable = intnl_shear
  [../]
  [./intnl_tensile]
    type = PointValue
    point = '0 0 0'
    variable = intnl_tensile
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  [../]
  [./ls]
    type = PointValue
    point = '0 0 0'
    variable = ls
  [../]
[]


[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 30
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.1111077
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningConstant
    value = 40
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 0.024449878
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 1.0
    poisson = 0.2
    layer_thickness = 0.1
    joint_normal_stiffness = 0.25
    joint_shear_stiffness = 0.2
  [../]
  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
  [../]
  [./admissible]
    type = ComputeMultipleInelasticCosseratStress
    inelastic_models = stress
    perform_finite_strain_rotations = false
  [../]
  [./stress]
    type = CappedWeakPlaneCosseratStressUpdate
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0
    smoothing_tol = 1
    yield_function_tol = 1E-5
  [../]
[]

[Executioner]
  nl_abs_tol = 1E-14
  end_time = 3
  dt = 1
  type = Transient
[]

[Outputs]
  file_base = small_deform_cosserat2
  csv = true
[]


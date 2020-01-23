# checking for small deformation
# A single element is stretched by 1E-6m in x,y and z directions.
# stress_zz = Youngs Modulus*Strain = 2E6*1E-6 = 2 Pa
# wpt_tensile_strength is set to 1Pa
# Then the final stress should return to the yeild surface and its value should be 1pa.

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
    type = DirichletBC
    variable = disp_x
    boundary = front
    value = 0E-6
  [../]
  [./topy]
    type = DirichletBC
    variable = disp_y
    boundary = front
    value = 0E-6
  [../]
  [./topz]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 1E-6
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
[]

[UserObjects]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 100
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
    mc_tip_smoother = 4
    mc_edge_smoother = 25
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
  [./str]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./wpt]
    type = TensorMechanicsPlasticWeakPlaneTensile
    tensile_strength = str
    yield_function_tolerance = 1E-6
    internal_constraint_tolerance = 1E-5
  [../]
[]

[Materials]
  [./mc]
    type = FiniteStrainMultiPlasticity
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
    ep_plastic_tolerance = 1E-9
    plastic_models = 'mc wpt'
    debug_fspb = crash
    debug_jac_at_stress = '10 0 0 0 10 0 0 0 10'
    debug_jac_at_pm = 1
    debug_jac_at_intnl = 1
    debug_stress_change = 1E-5
    debug_pm_change = 1E-6
    debug_intnl_change = 1E-6
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = mc_wpt_1
  exodus = false
  [./csv]
    type = CSV
    [../]
[]

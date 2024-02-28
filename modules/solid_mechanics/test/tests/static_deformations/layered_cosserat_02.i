# apply shears and Cosserat rotations and observe the stresses and moment-stresses
# with
# young = 0.7
# poisson = 0.2
# layer_thickness = 0.1
# joint_normal_stiffness = 0.25
# joint_shear_stiffness = 0.2
# then
# a0000 = 0.730681
# a0011 = 0.18267
# a2222 = 0.0244221
# a0022 = 0.006055
# a0101 = 0.291667
# a66 = 0.018717
# a77 = 0.155192
# b0110 = 0.000534
# b0101 = 0.000107
# and with
# u_x = y + 2*z
# u_y = x -1.5*z
# u_z = 1.1*x - 2.2*y
# wc_x = 0.5
# wc_y = 0.8
# then
# strain_xx = 0
# strain_xy = 1
# strain_xz = 2 - 0.8 = 1.2
# strain_yx = 1
# strain_yy = 0
# strain_yz = -1.5 + 0.5 = -1
# strain_zx = 1.1 + 0.8 = 1.9
# strain_zy = -2.2 - 0.5 = -2.7
# strain_zz = 0
# so that
# stress_xy = a0101*(1+1) = 0.583333
# stress_xz = a66*1.2 + a66*1.9 = 0.058021
# stress_yx = a0101*(1+1) = 0.583333
# stress_yz = a66*(-1) + a66*(-2.7) = -0.06925
# old stress_zx = a77*1.2 + a66*1.9 = 0.221793
# old stress_zy = a77*(-1) + a66*(-2.7) = -0.205728
# stress_zx = a66*1.2 + a77*1.9 = 0.317325
# stress_zy = a66*(-1) + a77*(-2.7) = -0.437735
# and all others zero
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  ymax = 1
  nz = 1
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
  # zmin is called back
  # zmax is called front
  # ymin is called bottom
  # ymax is called top
  # xmin is called left
  # xmax is called right
  [./strain_xx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left right'
    function = 'y+2*z'
  [../]
  [./strain_yy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'bottom top'
    function = 'x-1.5*z'
  [../]
  [./strain_zz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'back front'
    function = '1.1*x-2.2*y'
  [../]
  [./wc_x]
    type = FunctionDirichletBC
    variable = wc_x
    boundary = 'left right'
    function = 0.5
  [../]
  [./wc_y]
    type = FunctionDirichletBC
    variable = wc_y
    boundary = 'left right'
    function = 0.8
  [../]
[]

[AuxVariables]
  [./wc_z]
  [../]
  [./stress_xx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_xy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_xz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_yx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_yy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_yz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_zx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_zy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./stress_zz]
    family = MONOMIAL
    order = CONSTANT
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
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 0.7
    poisson = 0.2
    layer_thickness = 0.1
    joint_normal_stiffness = 0.25
    joint_shear_stiffness = 0.2
  [../]
  [./strain]
    type = ComputeCosseratSmallStrain
  [../]
  [./stress]
    type = ComputeCosseratLinearElasticStress
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol'
    petsc_options_value = 'gmres asm lu 1E-10 1E-14 10 1E-15 1E-10'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = layered_cosserat_02
  csv = true
[]

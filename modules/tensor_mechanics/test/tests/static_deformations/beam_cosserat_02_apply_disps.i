# Beam bending.
# Displacements are applied to a beam and stresses and moment-stresses
# are measured.  Note that since these quantities are averaged over
# elements, to get a good agreement with the analytical solution the
# number of elements (nz) should be increased.  Using nx=10
# and nz=10 yields roughly 1% error.
# The displacements applied are a pure-bend around the y axis
# with an additional displacement in the y direction so that
# the result (below) will end up being plane stress (stress_yy=0):
# u_x = Axz
# u_y = Dzy
# u_z = -(A/2)x^2 + (D/2)(z^2-y^2)
# wc_x = -Dy
# wc_y = Ax
# wc_z = 0
# Here A and D are arbitrary constants.
# This results in strains being symmetric, and the only
# nonzero ones are
# ep_xx = Az
# ep_yy = Dz
# ep_zz = Dz
# kappa_xy = -D
# kappa_yx = A
# Then choosing D = -poisson*A gives, for layered Cosserat:
# stress_xx = EAz
# m_yx = (1-poisson^2)*A*B = (1/12)EAh^2 (last equality for joint_shear_stiffness=0)
# where h is the layer thickness.  All other stress and moment-stress
# components are zero.
# The test uses: E=1.2, poisson=0.3, A=1.11E-2, h=2
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  xmax = 10
  ny = 1
  nz = 10
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
  # zmin is called back
  # zmax is called front
  # ymin is called bottom
  # ymax is called top
  # xmin is called left
  # xmax is called right
  [./clamp_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'left right top bottom front back'
    function = '-1.11E-2*x*x/2-0.3*(z*z-y*y)/2.0*1.11E-2'
  [../]
  [./clamp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left right top bottom front back'
    function = '-0.3*z*y*1.11E-2'
  [../]
  [./clamp_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left right top bottom front back'
    function = '1.11E-2*x*z'
  [../]
  [./clamp_wc_x]
    type = FunctionDirichletBC
    variable = wc_x
    boundary = 'left right top bottom front back'
    function = '0.3*y*1.11E-2'
  [../]
  [./clamp_wc_y]
    type = FunctionDirichletBC
    variable = wc_y
    boundary = 'left right top bottom front back'
    function = '1.11E-2*x'
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

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 1.2
    poisson = 0.3
    layer_thickness = 2.0
    joint_normal_stiffness = 1E16
    joint_shear_stiffness = 1E-15
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
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres asm lu 1E-10 1E-14 10 1E-15 1E-10 NONZERO'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = beam_cosserat_02_apply_disps
  exodus = true
[]

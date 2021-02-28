# Beam bending.
# One end is clamped and the other end is subjected to a stress
# and micromechanical moment that will induce bending.
# The stress that will induce bending around the y axis is
# stress_xx = EAz
# This implies a micromechanical moment-stress of
# m_yx = (1/12)EAh^2 for joint_shear_stiffness=0.
# For joint_shear_stiffness!=0, the micromechanical moment-stress
# is
# m_yx = (1/12)EAa^2 G/(ak_s + G)
# All other stresses and moment stresses are assumed to be zero.
# With joint_shear_stiffness=0, and introducing D=-poisson*A, the
# nonzero strains are
# ep_xx = Az
# ep_yy = Dz
# ep_zz = Dz
# kappa_xy = -D
# kappa_yx = A
# This means the displacements are:
# u_x = Axz
# u_y = Dzy
# u_z = -(A/2)x^2 + (D/2)(z^2-y^2)
# wc_x = -Dy
# wc_y = Ax
# wc_z = 0
# This is bending of a bar around the y axis, in plane stress
# (stress_yy=0).  Displacements at the left-hand (x=0) are applied
# according to the above formulae; wc_x and wc_y are applied throughout
# the bar; and stress_xx is applied at the right-hand end (x=10).
# The displacements are measured and
# compared with the above formulae.
# The test uses: E=1.2, poisson=0.3, A=1.11E-2, h=2, ks=0.1, so
# stress_xx = 1.332E-2*z
# m_yx = 0.2379E-2
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 1
  nz = 10
  xmin = 0
  xmax = 10
  ymin = -1
  ymax = 1
  zmin = -0.5
  zmax = 0.5
[]

[GlobalParams]
  #use_displaced_mesh = false
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
    boundary = left
    function = '-0.3*(z*z-y*y)/2.0*1.11E-2'
  [../]
  [./clamp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = left
    function = '-0.3*z*y*1.11E-2'
  [../]
  [./clamp_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./end_stress]
    type = FunctionNeumannBC
    boundary = right
    function = z*1.2*1.11E-2
    variable = disp_x
  [../]
  [./fix_wc_x]
    type = FunctionDirichletBC
    variable = wc_x
    boundary = 'left' # right top bottom front back'
    function = '0.3*y*1.11E-2'
  [../]
  [./fix_wc_y]
    type = FunctionDirichletBC
    variable = wc_y
    boundary = 'left' # right top bottom front back'
    function = '1.11E-2*x'
  [../]
  [./end_moment]
    type = VectorNeumannBC
    boundary = right
    variable = wc_y
    vector_value = '2.3785714286E-3 0 0'
  [../]
[]


[AuxVariables]
  [./wc_z]
  [../]
  [./strain_xx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_xy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_xz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_yx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_yy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_yz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_zx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_zy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_zz]
    family = MONOMIAL
    order = CONSTANT
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
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xy
    index_i = 0
    index_j = 1
  [../]
  [./strain_xz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xz
    index_i = 0
    index_j = 2
  [../]
  [./strain_yx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yx
    index_i = 1
    index_j = 0
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
  [./strain_yz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yz
    index_i = 1
    index_j = 2
  [../]
  [./strain_zx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zx
    index_i = 2
    index_j = 0
  [../]
  [./strain_zy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zy
    index_i = 2
    index_j = 1
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
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

[VectorPostprocessors]
  [./soln]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    sort_by = x
    variable = 'disp_x disp_y disp_z stress_xx stress_xy stress_xz stress_yx stress_yy stress_yz stress_zx stress_zy stress_zz wc_x wc_y wc_z couple_stress_xx couple_stress_xy couple_stress_xz couple_stress_yx couple_stress_yy couple_stress_yz couple_stress_zx couple_stress_zy couple_stress_zz'
    start_point = '0 0 0.5'
    end_point = '10 0 0.5'
    num_points = 11
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 1.2
    poisson = 0.3
    layer_thickness = 2.0
    joint_normal_stiffness = 1E16
    joint_shear_stiffness = 0.1
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
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol -ksp_max_it -sub_pc_factor_shift_type -pc_asm_overlap -ksp_gmres_restart'
    petsc_options_value = 'gmres asm lu 1E-11 1E-11 10 1E-15 1E-10 100 NONZERO 2 100'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = beam_cosserat_02_apply_stress
  exodus = true
  csv = true
[]

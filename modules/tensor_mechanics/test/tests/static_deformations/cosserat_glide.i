# Example taken from Appendix A of
# S Forest "Mechanics of Cosserat media An introduction".  Available from http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.154.4476&rep=rep1&type=pdf
#
# Analytically, the displacements are
# wc_z = B sinh(w_e y)
# disp_x = (2 mu_c B / w_e / (mu + mu_c)) (1 - cosh(w_e y))
# with w_e^2 = 2 mu mu_c / be / (mu + mu_c)
# and B = arbitrary integration constant
#
# Also, the only nonzero stresses are
# m_zy = 2 B be w_e cosh(w_e y)
# si_yx = -4 mu mu_c/(mu + mu_c) B sinh(w_e y)
#
# MOOSE gives these stress components correctly.
# However, it also gives a seemingly non-zero si_xy
# component.  Upon increasing the resolution of the
# mesh (ny=10000, for example), the stress components
# are seen to limit correctly to the above forumlae
#
# I use mu = 2, mu_c = 3, be = 0.6, so w_e = 2
# Also i use B = 1, so at y = 1
# wc_z = 3.626860407847
# disp_x = -1.65731741465
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 100
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
  [./z_couple]
    type = StressDivergenceTensors
    variable = wc_z
    displacements = 'wc_x wc_y wc_z'
    component = 2
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
  [./z_moment]
    type = MomentBalancing
    variable = wc_z
    component = 2
  [../]
[]

[BCs]
  # zmin is called back
  # zmax is called front
  # ymin is called bottom
  # ymax is called top
  # xmin is called left
  # xmax is called right
  [./disp_x_zero_at_y_zero]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  [../]
  [./disp_x_fixed_at_y_max]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = -1.65731741465
  [../]
  [./no_dispy]
    type = DirichletBC
    variable = disp_y
    boundary = 'back front bottom top left right'
    value = 0
  [../]
  [./no_dispz]
    type = DirichletBC
    variable = disp_z
    boundary = 'back front bottom top left right'
    value = 0
  [../]
  [./no_wc_x]
    type = DirichletBC
    variable = wc_x
    boundary = 'back front bottom top left right'
    value = 0
  [../]
  [./no_wc_y]
    type = DirichletBC
    variable = wc_y
    boundary = 'back front bottom top left right'
    value = 0
  [../]
  [./wc_z_zero_at_y_zero]
    type = DirichletBC
    variable = wc_z
    boundary = bottom
    value = 0
  [../]
  [./wc_z_fixed_at_y_max]
    type = DirichletBC
    variable = wc_z
    boundary = top
    value = 3.626860407847
  [../]
[]

[AuxVariables]
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
    type = ComputeCosseratElasticityTensor
    B_ijkl = '1.1 0.6 0.6' # In Forest notation this is alpha=1.1 (this is unimportant), beta=gamma=0.6.
    fill_method_bending = 'general_isotropic'
    E_ijkl = '1 2 3' # In Forest notation this is lambda=1 (this is unimportant), mu=2, mu_c=3
    fill_method = 'general_isotropic'
  [../]
  [./strain]
    type = ComputeCosseratSmallStrain
  [../]
  [./stress]
    type = ComputeCosseratLinearElasticStress
  [../]
[]

[VectorPostprocessors]
  [./soln]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    sort_by = y
    variable = 'disp_x wc_z stress_yx couple_stress_zy'
    start_point = '0 0 0'
    end_point = '0 1 0'
    num_points = 11
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
  file_base = cosserat_glide_out
  exodus = true
  csv = true
[]

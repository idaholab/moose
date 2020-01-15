# This is identical to vol_expansion.i, but uses the PoroMechanics action
#
# Apply an increasing porepressure, with zero mechanical forces,
# and observe the corresponding volumetric expansion
#
# P = t
# With the Biot coefficient being 2.0, the effective stresses should be
# stress_xx = stress_yy = stress_zz = 2t
# With bulk modulus = 1 then should have
# vol_strain = strain_xx + strain_yy + strain_zz = 2t.
# I use a single element lying 0<=x<=1, 0<=y<=1 and 0<=z<=1, and
# fix the left, bottom and back boundaries appropriately,
# so at the point x=y=z=1, the displacements should be
# disp_x = disp_y = disp_z = 2t/3 (small strain physics is used)
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./p]
  [../]
[]

[BCs]
  [./p]
    type = FunctionDirichletBC
    boundary = 'bottom top'
    variable = p
    function = t
  [../]
  [./xmin]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0
  [../]
  [./ymin]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  [../]
  [./zmin]
    type = DirichletBC
    boundary = back
    variable = disp_z
    value = 0
  [../]
[]


[Kernels]
  [./PoroMechanics]
    porepressure = p
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./unimportant_p]
    type = Diffusion
    variable = p
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
  [./corner_x]
    type = PointValue
    point = '1 1 1'
    variable = disp_x
  [../]
  [./corner_y]
    type = PointValue
    point = '1 1 1'
    variable = disp_y
  [../]
  [./corner_z]
    type = PointValue
    point = '1 1 1'
    variable = disp_z
  [../]
[]


[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    # bulk modulus = 1, poisson ratio = 0.2
    C_ijkl = '0.5 0.75'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./biot]
    type = GenericConstantMaterial
    prop_names = biot_coefficient
    prop_values = 2.0
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol'
    petsc_options_value = 'gmres bjacobi 1E-10 1E-10 10 1E-15 1E-10'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  dt = 0.1
  end_time = 1
[]

[Outputs]
  file_base = vol_expansion_action
  exodus = true
[]

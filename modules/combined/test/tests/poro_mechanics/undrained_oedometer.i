# An undrained oedometer test on a saturated poroelastic sample.
#
# The sample is a single unit element, with roller BCs on the sides
# and bottom.  A constant displacement is applied to the top: disp_z = -0.01*t.
# There is no fluid flow.
#
# Under these conditions
# porepressure = -(Biot coefficient)*(Biot modulus)*disp_z/L
# stress_xx = (bulk - 2*shear/3)*disp_z/L (remember this is effective stress)
# stress_zz = (bulk + 4*shear/3)*disp_z/L (remember this is effective stress)
# where L is the height of the sample (L=1 in this test)
#
# Parameters:
# Biot coefficient = 0.3
# Porosity = 0.1
# Bulk modulus = 2
# Shear modulus = 1.5
# fluid bulk modulus = 1/0.3 = 3.333333
# 1/Biot modulus = (1 - 0.3)*(0.3 - 0.1)/2 + 0.1*0.3 = 0.1. BiotModulus = 10
#
# Desired output:
# zdisp = -0.01*t
# p0 = 0.03*t
# stress_xx = stress_yy = -0.01*t
# stress_zz = -0.04*t

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
  porepressure = porepressure
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./porepressure]
  [../]
[]

[BCs]
  [./confinex]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  [../]
  [./confiney]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
  [../]
  [./basefixed]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = back
  [../]
  [./top_velocity]
    type = FunctionDirichletBC
    variable = disp_z
    function = -0.01*t
    boundary = front
  [../]
[]


[Kernels]
  [./grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./poro_x]
    type = PoroMechanicsCoupling
    variable = disp_x
    component = 0
  [../]
  [./poro_y]
    type = PoroMechanicsCoupling
    variable = disp_y
    component = 1
  [../]
  [./poro_z]
    type = PoroMechanicsCoupling
    variable = disp_z
    component = 2
  [../]
  [./poro_timederiv]
    type = PoroFullSatTimeDerivative
    variable = porepressure
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



[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./poro_material]
    type = PoroFullSatMaterial
    porosity0 = 0.1
    biot_coefficient = 0.3
    solid_bulk_compliance = 0.5
    fluid_bulk_compliance = 0.3
    constant_porosity = true
  [../]
[]

[Postprocessors]
  [./p0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = porepressure
  [../]
  [./zdisp]
    type = PointValue
    outputs = csv
    point = '0 0 0.5'
    variable = disp_z
  [../]
  [./stress_xx]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./stress_yy]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./stress_zz]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_zz
  [../]

[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-14 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 10
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = undrained_oedometer
  [./csv]
    type = CSV
  [../]
[]

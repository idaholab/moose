# Poroelastic response of a borehole.
#
# LOWRES VERSION: this version does not give perfect agreement with the analytical solution
#
# A fully-saturated medium contains a fluid with a homogeneous porepressure,
# but an anisitropic insitu stress.  A infinitely-long borehole aligned with
# the $$z$$ axis is instanteously excavated.  The borehole boundary is
# stress-free and allowed to freely drain.  This problem is analysed using
# plane-strain conditions (no $$z$$ displacement).
#
# The solution in Laplace space is found in E Detournay and AHD Cheng "Poroelastic response of a borehole in a non-hydrostatic stress field".  International Journal of Rock Mechanics and Mining Sciences and Geomechanics Abstracts 25 (1988) 171-182.  In the small-time limit, the Laplace transforms may be performed.  There is one typo in the paper.  Equation (A4)'s final term should be -(a/r)\sqrt(4ct/(a^2\pi)), and not +(a/r)\sqrt(4ct/(a^2\pi)).
#
# Because realistic parameters are chosen (below),
# the residual for porepressure is much smaller than
# the residuals for the displacements.  Therefore the
# scaling parameter is chosen.  Also note that the
# insitu stresses are effective stresses, not total
# stresses, but the solution in the above paper is
# expressed in terms of total stresses.
#
# Here are the problem's parameters, and their values:
# Borehole radius.  a = 1
# Rock's Lame lambda.  la = 0.5E9
# Rock's Lame mu, which is also the Rock's shear modulus.  mu = G = 1.5E9
# Rock bulk modulus.  K = la + 2*mu/3 = 1.5E9
# Drained Poisson ratio.  nu = (3K - 2G)/(6K + 2G) = 0.125
# Rock bulk compliance.  1/K = 0.66666666E-9
# Fluid bulk modulus.  Kf = 0.7171315E9
# Fluid bulk compliance.  1/Kf = 1.39444444E-9
# Rock initial porosity.  phi0 = 0.3
# Biot coefficient.  alpha = 0.65
# Biot modulus.  M = 1/(phi0/Kf + (alpha - phi0)(1 - alpha)/K) = 2E9
# Undrained bulk modulus. Ku = K + alpha^2*M = 2.345E9
# Undrained Poisson ratio.  nuu = (3Ku - 2G)/(6Ku + 2G) = 0.2364
# Skempton coefficient.  B = alpha*M/Ku = 0.554
# Fluid mobility (rock permeability/fluid viscosity).  k = 1E-12

[Mesh]
  type = FileMesh
  file = borehole_lowres_input.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  porepressure = porepressure
  block = 1
[]

[GlobalParams]
  volumetric_locking_correction=true
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./porepressure]
    scaling = 1E9  # Notice the scaling, to make porepressure's kernels roughly of same magnitude as disp's kernels
  [../]
[]

[ICs]
  [./initial_p]
    type = ConstantIC
    variable = porepressure
    value = 1E6
  [../]
[]

[BCs]
  [./fixed_outer_x]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = outer
  [../]
  [./fixed_outer_y]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = outer
  [../]
  [./plane_strain]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'zmin zmax'
  [../]
  [./borehole_wall]
    type = DirichletBC
    variable = porepressure
    value = 0
    boundary = bh_wall
  [../]
[]



[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./tot_yy]
    type = ParsedAux
    coupled_variables = 'stress_yy porepressure'
    execute_on = timestep_end
    variable = tot_yy
    expression = 'stress_yy-0.65*porepressure'
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
  [./darcy_flow]
    type = CoefDiffusion
    variable = porepressure
    coef = 1E-12
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '0.5E9 1.5E9'
    # bulk modulus is lambda + 2*mu/3 = 0.5 + 2*1.5/3 = 1.5E9
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '-1.35E6 0 0  0 -3.35E6 0  0 0 0' # remember this is the effective stress
    eigenstrain_name = ini_stress
  [../]
  [./no_plasticity]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./poro_material]
    type = PoroFullSatMaterial
    porosity0 = 0.3
    biot_coefficient = 0.65
    solid_bulk_compliance = 0.6666666666667E-9
    fluid_bulk_compliance = 1.3944444444444E-9
    constant_porosity = false
  [../]
[]

[Postprocessors]
  [./p00]
    type = PointValue
    variable = porepressure
    point = '1.00 0 0'
    outputs = csv_p
  [../]
  [./p01]
    type = PointValue
    variable = porepressure
    point = '1.01 0 0'
    outputs = csv_p
  [../]
  [./p02]
    type = PointValue
    variable = porepressure
    point = '1.02 0 0'
    outputs = csv_p
  [../]
  [./p03]
    type = PointValue
    variable = porepressure
    point = '1.03 0 0'
    outputs = csv_p
  [../]
  [./p04]
    type = PointValue
    variable = porepressure
    point = '1.04 0 0'
    outputs = csv_p
  [../]
  [./p05]
    type = PointValue
    variable = porepressure
    point = '1.05 0 0'
    outputs = csv_p
  [../]
  [./p06]
    type = PointValue
    variable = porepressure
    point = '1.06 0 0'
    outputs = csv_p
  [../]
  [./p07]
    type = PointValue
    variable = porepressure
    point = '1.07 0 0'
    outputs = csv_p
  [../]
  [./p08]
    type = PointValue
    variable = porepressure
    point = '1.08 0 0'
    outputs = csv_p
  [../]
  [./p09]
    type = PointValue
    variable = porepressure
    point = '1.09 0 0'
    outputs = csv_p
  [../]
  [./p10]
    type = PointValue
    variable = porepressure
    point = '1.10 0 0'
    outputs = csv_p
  [../]
  [./p11]
    type = PointValue
    variable = porepressure
    point = '1.11 0 0'
    outputs = csv_p
  [../]
  [./p12]
    type = PointValue
    variable = porepressure
    point = '1.12 0 0'
    outputs = csv_p
  [../]
  [./p13]
    type = PointValue
    variable = porepressure
    point = '1.13 0 0'
    outputs = csv_p
  [../]
  [./p14]
    type = PointValue
    variable = porepressure
    point = '1.14 0 0'
    outputs = csv_p
  [../]
  [./p15]
    type = PointValue
    variable = porepressure
    point = '1.15 0 0'
    outputs = csv_p
  [../]
  [./p16]
    type = PointValue
    variable = porepressure
    point = '1.16 0 0'
    outputs = csv_p
  [../]
  [./p17]
    type = PointValue
    variable = porepressure
    point = '1.17 0 0'
    outputs = csv_p
  [../]
  [./p18]
    type = PointValue
    variable = porepressure
    point = '1.18 0 0'
    outputs = csv_p
  [../]
  [./p19]
    type = PointValue
    variable = porepressure
    point = '1.19 0 0'
    outputs = csv_p
  [../]
  [./p20]
    type = PointValue
    variable = porepressure
    point = '1.20 0 0'
    outputs = csv_p
  [../]
  [./p21]
    type = PointValue
    variable = porepressure
    point = '1.21 0 0'
    outputs = csv_p
  [../]
  [./p22]
    type = PointValue
    variable = porepressure
    point = '1.22 0 0'
    outputs = csv_p
  [../]
  [./p23]
    type = PointValue
    variable = porepressure
    point = '1.23 0 0'
    outputs = csv_p
  [../]
  [./p24]
    type = PointValue
    variable = porepressure
    point = '1.24 0 0'
    outputs = csv_p
  [../]
  [./p25]
    type = PointValue
    variable = porepressure
    point = '1.25 0 0'
    outputs = csv_p
  [../]

  [./s00]
    type = PointValue
    variable = disp_x
    point = '1.00 0 0'
    outputs = csv_s
  [../]
  [./s01]
    type = PointValue
    variable = disp_x
    point = '1.01 0 0'
    outputs = csv_s
  [../]
  [./s02]
    type = PointValue
    variable = disp_x
    point = '1.02 0 0'
    outputs = csv_s
  [../]
  [./s03]
    type = PointValue
    variable = disp_x
    point = '1.03 0 0'
    outputs = csv_s
  [../]
  [./s04]
    type = PointValue
    variable = disp_x
    point = '1.04 0 0'
    outputs = csv_s
  [../]
  [./s05]
    type = PointValue
    variable = disp_x
    point = '1.05 0 0'
    outputs = csv_s
  [../]
  [./s06]
    type = PointValue
    variable = disp_x
    point = '1.06 0 0'
    outputs = csv_s
  [../]
  [./s07]
    type = PointValue
    variable = disp_x
    point = '1.07 0 0'
    outputs = csv_s
  [../]
  [./s08]
    type = PointValue
    variable = disp_x
    point = '1.08 0 0'
    outputs = csv_s
  [../]
  [./s09]
    type = PointValue
    variable = disp_x
    point = '1.09 0 0'
    outputs = csv_s
  [../]
  [./s10]
    type = PointValue
    variable = disp_x
    point = '1.10 0 0'
    outputs = csv_s
  [../]
  [./s11]
    type = PointValue
    variable = disp_x
    point = '1.11 0 0'
    outputs = csv_s
  [../]
  [./s12]
    type = PointValue
    variable = disp_x
    point = '1.12 0 0'
    outputs = csv_s
  [../]
  [./s13]
    type = PointValue
    variable = disp_x
    point = '1.13 0 0'
    outputs = csv_s
  [../]
  [./s14]
    type = PointValue
    variable = disp_x
    point = '1.14 0 0'
    outputs = csv_s
  [../]
  [./s15]
    type = PointValue
    variable = disp_x
    point = '1.15 0 0'
    outputs = csv_s
  [../]
  [./s16]
    type = PointValue
    variable = disp_x
    point = '1.16 0 0'
    outputs = csv_s
  [../]
  [./s17]
    type = PointValue
    variable = disp_x
    point = '1.17 0 0'
    outputs = csv_s
  [../]
  [./s18]
    type = PointValue
    variable = disp_x
    point = '1.18 0 0'
    outputs = csv_s
  [../]
  [./s19]
    type = PointValue
    variable = disp_x
    point = '1.19 0 0'
    outputs = csv_s
  [../]
  [./s20]
    type = PointValue
    variable = disp_x
    point = '1.20 0 0'
    outputs = csv_s
  [../]
  [./s21]
    type = PointValue
    variable = disp_x
    point = '1.21 0 0'
    outputs = csv_s
  [../]
  [./s22]
    type = PointValue
    variable = disp_x
    point = '1.22 0 0'
    outputs = csv_s
  [../]
  [./s23]
    type = PointValue
    variable = disp_x
    point = '1.23 0 0'
    outputs = csv_s
  [../]
  [./s24]
    type = PointValue
    variable = disp_x
    point = '1.24 0 0'
    outputs = csv_s
  [../]
  [./s25]
    type = PointValue
    variable = disp_x
    point = '1.25 0 0'
    outputs = csv_s
  [../]

  [./t00]
    type = PointValue
    variable = tot_yy
    point = '1.00 0 0'
    outputs = csv_t
  [../]
  [./t01]
    type = PointValue
    variable = tot_yy
    point = '1.01 0 0'
    outputs = csv_t
  [../]
  [./t02]
    type = PointValue
    variable = tot_yy
    point = '1.02 0 0'
    outputs = csv_t
  [../]
  [./t03]
    type = PointValue
    variable = tot_yy
    point = '1.03 0 0'
    outputs = csv_t
  [../]
  [./t04]
    type = PointValue
    variable = tot_yy
    point = '1.04 0 0'
    outputs = csv_t
  [../]
  [./t05]
    type = PointValue
    variable = tot_yy
    point = '1.05 0 0'
    outputs = csv_t
  [../]
  [./t06]
    type = PointValue
    variable = tot_yy
    point = '1.06 0 0'
    outputs = csv_t
  [../]
  [./t07]
    type = PointValue
    variable = tot_yy
    point = '1.07 0 0'
    outputs = csv_t
  [../]
  [./t08]
    type = PointValue
    variable = tot_yy
    point = '1.08 0 0'
    outputs = csv_t
  [../]
  [./t09]
    type = PointValue
    variable = tot_yy
    point = '1.09 0 0'
    outputs = csv_t
  [../]
  [./t10]
    type = PointValue
    variable = tot_yy
    point = '1.10 0 0'
    outputs = csv_t
  [../]
  [./t11]
    type = PointValue
    variable = tot_yy
    point = '1.11 0 0'
    outputs = csv_t
  [../]
  [./t12]
    type = PointValue
    variable = tot_yy
    point = '1.12 0 0'
    outputs = csv_t
  [../]
  [./t13]
    type = PointValue
    variable = tot_yy
    point = '1.13 0 0'
    outputs = csv_t
  [../]
  [./t14]
    type = PointValue
    variable = tot_yy
    point = '1.14 0 0'
    outputs = csv_t
  [../]
  [./t15]
    type = PointValue
    variable = tot_yy
    point = '1.15 0 0'
    outputs = csv_t
  [../]
  [./t16]
    type = PointValue
    variable = tot_yy
    point = '1.16 0 0'
    outputs = csv_t
  [../]
  [./t17]
    type = PointValue
    variable = tot_yy
    point = '1.17 0 0'
    outputs = csv_t
  [../]
  [./t18]
    type = PointValue
    variable = tot_yy
    point = '1.18 0 0'
    outputs = csv_t
  [../]
  [./t19]
    type = PointValue
    variable = tot_yy
    point = '1.19 0 0'
    outputs = csv_t
  [../]
  [./t20]
    type = PointValue
    variable = tot_yy
    point = '1.20 0 0'
    outputs = csv_t
  [../]
  [./t21]
    type = PointValue
    variable = tot_yy
    point = '1.21 0 0'
    outputs = csv_t
  [../]
  [./t22]
    type = PointValue
    variable = tot_yy
    point = '1.22 0 0'
    outputs = csv_t
  [../]
  [./t23]
    type = PointValue
    variable = tot_yy
    point = '1.23 0 0'
    outputs = csv_t
  [../]
  [./t24]
    type = PointValue
    variable = tot_yy
    point = '1.24 0 0'
    outputs = csv_t
  [../]
  [./t25]
    type = PointValue
    variable = tot_yy
    point = '1.25 0 0'
    outputs = csv_t
  [../]

  [./dt]
    type = FunctionValuePostprocessor
    outputs = console
    function = 2*t
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_monitor -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres asm 1E0 1E-10 200 500 lu NONZERO'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 0.3
  dt = 0.3
  #[./TimeStepper]
  #  type = PostprocessorDT
  #  postprocessor = dt
  #  dt = 0.003
  #[../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = borehole_lowres
  exodus = true
  sync_times = '0.003 0.3'
  [./csv_p]
    file_base = borehole_lowres_p
    type = CSV
  [../]
  [./csv_s]
    file_base = borehole_lowres_s
    type = CSV
  [../]
  [./csv_t]
    file_base = borehole_lowres_t
    type = CSV
  [../]
[]

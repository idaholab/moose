# Mandel's problem of consolodation of a drained medium
#
# A sample is in plane strain.
# -a <= x <= a
# -b <= y <= b
# It is squashed with constant force by impermeable, frictionless plattens on its top and bottom surfaces (at y=+/-b)
# Fluid is allowed to leak out from its sides (at x=+/-a)
# The porepressure within the sample is monitored.
#
# As is common in the literature, this is simulated by
# considering the quarter-sample, 0<=x<=a and 0<=y<=b, with
# impermeable, roller BCs at x=0 and y=0 and y=b.
# Porepressure is fixed at zero on x=a.
# Porepressure and displacement are initialised to zero.
# Then the top (y=b) is moved downwards with prescribed velocity,
# so that the total force that is inducing this downwards velocity
# is fixed.  The velocity is worked out by solving Mandel's problem
# analytically, and the total force is monitored in the simulation
# to check that it indeed remains constant.
#
# Here are the problem's parameters, and their values:
# Soil width.  a = 1
# Soil height.  b = 0.1
# Soil's Lame lambda.  la = 0.5
# Soil's Lame mu, which is also the Soil's shear modulus.  mu = G = 0.75
# Soil bulk modulus.  K = la + 2*mu/3 = 1
# Drained Poisson ratio.  nu = (3K - 2G)/(6K + 2G) = 0.2
# Soil bulk compliance.  1/K = 1
# Fluid bulk modulus.  Kf = 8
# Fluid bulk compliance.  1/Kf = 0.125
# Soil initial porosity.  phi0 = 0.1
# Biot coefficient.  alpha = 0.6
# Biot modulus.  M = 1/(phi0/Kf + (alpha - phi0)(1 - alpha)/K) = 4.705882
# Undrained bulk modulus. Ku = K + alpha^2*M = 2.694118
# Undrained Poisson ratio.  nuu = (3Ku - 2G)/(6Ku + 2G) = 0.372627
# Skempton coefficient.  B = alpha*M/Ku = 1.048035
# Fluid mobility (soil permeability/fluid viscosity).  k = 1.5
# Consolidation coefficient.  c = 2*k*B^2*G*(1-nu)*(1+nuu)^2/9/(1-nuu)/(nuu-nu) = 3.821656
# Normal stress on top.  F = 1
#
# The solution for porepressure and displacements is given in
# AHD Cheng and E Detournay "A direct boundary element method for plane strain poroelasticity" International Journal of Numerical and Analytical Methods in Geomechanics 12 (1988) 551-572.
# The solution involves complicated infinite series, so I shall not write it here

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 1
  nz = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 0.1
  zmin = 0
  zmax = 1
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
  [./roller_xmin]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left'
  [../]
  [./roller_ymin]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'bottom'
  [../]
  [./plane_strain]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'back front'
  [../]
  [./xmax_drained]
    type = DirichletBC
    variable = porepressure
    value = 0
    boundary = right
  [../]
  [./top_velocity]
    type = FunctionDirichletBC
    variable = disp_y
    function = top_velocity
    boundary = top
  [../]
[]

[Functions]
  [./top_velocity]
    type = PiecewiseLinear
    x = '0 0.002 0.006   0.014   0.03    0.046   0.062   0.078   0.094   0.11    0.126   0.142   0.158   0.174   0.19 0.206 0.222 0.238 0.254 0.27 0.286 0.302 0.318 0.334 0.35 0.366 0.382 0.398 0.414 0.43 0.446 0.462 0.478 0.494 0.51 0.526 0.542 0.558 0.574 0.59 0.606 0.622 0.638 0.654 0.67 0.686 0.702'
    y = '-0.041824842    -0.042730269    -0.043412712    -0.04428867     -0.045509181    -0.04645965     -0.047268246 -0.047974749      -0.048597109     -0.0491467  -0.049632388     -0.050061697      -0.050441198     -0.050776675     -0.051073238      -0.0513354 -0.051567152      -0.051772022     -0.051953128 -0.052113227 -0.052254754 -0.052379865 -0.052490464 -0.052588233 -0.052674662 -0.052751065 -0.052818606 -0.052878312 -0.052931093 -0.052977751 -0.053018997 -0.053055459 -0.053087691 -0.053116185 -0.053141373 -0.05316364 -0.053183324 -0.053200724 -0.053216106 -0.053229704 -0.053241725 -0.053252351 -0.053261745 -0.053270049 -0.053277389 -0.053283879 -0.053289615'
  [../]
[]


[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_force]
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
  [./tot_force]
    type = ParsedAux
    coupled_variables = 'stress_yy porepressure'
    execute_on = timestep_end
    variable = tot_force
    expression = '-stress_yy+0.6*porepressure'
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
    coef = 1.5
  [../]
[]


[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '0.5 0.75'
    # bulk modulus is lambda + 2*mu/3 = 0.5 + 2*0.75/3 = 1
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
    biot_coefficient = 0.6
    solid_bulk_compliance = 1
    fluid_bulk_compliance = 0.125
    constant_porosity = true
  [../]
[]

[Postprocessors]
  [./p0]
    type = PointValue
    outputs = csv
    point = '0.0 0 0'
    variable = porepressure
  [../]
  [./p1]
    type = PointValue
    outputs = csv
    point = '0.1 0 0'
    variable = porepressure
  [../]
  [./p2]
    type = PointValue
    outputs = csv
    point = '0.2 0 0'
    variable = porepressure
  [../]
  [./p3]
    type = PointValue
    outputs = csv
    point = '0.3 0 0'
    variable = porepressure
  [../]
  [./p4]
    type = PointValue
    outputs = csv
    point = '0.4 0 0'
    variable = porepressure
  [../]
  [./p5]
    type = PointValue
    outputs = csv
    point = '0.5 0 0'
    variable = porepressure
  [../]
  [./p6]
    type = PointValue
    outputs = csv
    point = '0.6 0 0'
    variable = porepressure
  [../]
  [./p7]
    type = PointValue
    outputs = csv
    point = '0.7 0 0'
    variable = porepressure
  [../]
  [./p8]
    type = PointValue
    outputs = csv
    point = '0.8 0 0'
    variable = porepressure
  [../]
  [./p9]
    type = PointValue
    outputs = csv
    point = '0.9 0 0'
    variable = porepressure
  [../]
  [./p99]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = porepressure
  [../]
  [./xdisp]
    type = PointValue
    outputs = csv
    point = '1 0.1 0'
    variable = disp_x
  [../]
  [./ydisp]
    type = PointValue
    outputs = csv
    point = '1 0.1 0'
    variable = disp_y
  [../]
  [./total_downwards_force]
     type = ElementAverageValue
     outputs = csv
     variable = tot_force
  [../]
  [./dt]
    type = FunctionValuePostprocessor
    outputs = console
    function = if(0.15*t<0.01,0.15*t,0.01)
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
  end_time = 0.7
  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = dt
    dt = 0.001
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = mandel
  [./csv]
    interval = 3
    type = CSV
  [../]
[]

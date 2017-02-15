############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  rho  = rho
  rhou = rhou
  rhoe = rhoe
  slope_reconstruction = rslope
  slope_limiting = lslope
  fluid_properties = fp
  implicit = false
[]
############################################################
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 400 # 400 | 800 for ref. soln.
[]
############################################################
[Problem]
  kernel_coverage_check = false
[]
############################################################
[Functions]
  [./ic_rho]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '1.0 7.0'
  [../]

  [./ic_rhou]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '0.0 -7.0
               1.0  7.0'
  [../]

  [./ic_rhoe]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '1.0 4.0'
  [../]
[]
############################################################
[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 0.71428571428571428571
    [../]
  [../]
[]
############################################################
[UserObjects]

  [./rslope]
    type = CNSFVSlopeReconstructionOneD
    execute_on = 'linear'
  [../]

  [./lslope]
    type = CNSFVSlopeLimitingOneD
    execute_on = 'linear'
    scheme = 'minmod' #none | minmod | mc | superbee
  [../]

  [./riemann]
    type = CNSFVHLLCInternalSideFlux
    execute_on = 'linear'
  [../]

  [./bc]
    type = CNSFVFreeOutflowBoundaryFlux
    execute_on = 'linear'
  [../]
[]
############################################################
[Variables]
  [./rho]
  [../]

  [./rhou]
  [../]

  [./rhoe]
  [../]
[]
############################################################
[AuxVariables]
  [./mach]
  [../]

  [./pres]
  [../]
[]
############################################################
[ICs]
  [./rho_ic]
    type = FunctionIC
    variable = 'rho'
    function = ic_rho
  [../]

  [./rhou_ic]
    type = FunctionIC
    variable = 'rhou'
    function = ic_rhou
  [../]

  [./rhoe_ic]
    type = FunctionIC
    variable = 'rhoe'
    function = ic_rhoe
  [../]

  [./mach_ic]
    type = CNSFVMachIC
    variable = 'mach'
  [../]

  [./pres_ic]
    type = CNSFVPressureIC
    variable = 'pres'
  [../]
[]
############################################################
[Kernels]
  #### Time derivative of mass
  [./time_rho]
    implicit = true
    type = TimeDerivative
    variable = rho
  [../]

  #### Time derivative of momentum in x-direction
  [./time_rhou]
    implicit = true
    type = TimeDerivative
    variable = rhou
  [../]

  #### Time derivative of total energy
  [./time_rhoe]
    implicit = true
    type = TimeDerivative
    variable = rhoe
  [../]
[]
############################################################
[DGKernels]
  #### Mass conservation eqn
  [./mass]
    type = CNSFVKernel
    variable = rho
    component = 'mass'
    flux = riemann
  [../]

  #### Momentum balance eqn in x-direction
  [./momx]
    type = CNSFVKernel
    variable = rhou
    component = 'x-momentum'
    flux = riemann
  [../]

  #### Total energy conservation eqn
  [./etot]
    type = CNSFVKernel
    variable = rhoe
    component = 'total-energy'
    flux = riemann
  [../]
[]
############################################################
[AuxKernels]
  [./mach]
    type = CNSFVMachAux
    variable = mach
  [../]

  [./pres]
    type = CNSFVPressureAux
    variable = pres
  [../]
[]
############################################################
[BCs]
  [./mass]
    type = CNSFVBC
    boundary = 'left right'
    variable = rho
    component = 'mass'
    flux = bc
  [../]

  [./momx]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhou
    component = 'x-momentum'
    flux = bc
  [../]

  [./etot]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhoe
    component = 'total-energy'
    flux = bc
  [../]
[]
############################################################
[Materials]
  [./cnsfv]
    type = CNSFVMaterial
    block = 0
  [../]
[]
############################################################
[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ExplicitMidpoint
  [../]
  solve_type = 'LINEAR'

  l_tol = 1e-4
  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  start_time = 0.0
  #### final time is 0.6 sec
  num_steps = 4 # 4 | 3000 for 400 cells | 6000 for 800 cells
  dt = 2e-4 # 2e-4 for 400 cells | 1e-4 for 800 cells
  dtmin = 1e-6
[]

[Outputs]
  [./Exodus]
    type = Exodus
    file_base = 1d_double_rarefaction_wave_out
    interval = 1 # 1 | 30 for 400 cells | 60 for 800 cells
  [../]
  print_perf_log = true
[]

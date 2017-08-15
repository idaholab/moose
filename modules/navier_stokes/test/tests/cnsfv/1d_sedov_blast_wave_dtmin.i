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
  xmin = 0
  xmax = 1
  nx = 400 # 400 | 800 as ref soln
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
    xy_data = '1.0 1.0'
  [../]

  [./ic_rhou]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '1.0 0.0'
  [../]

  [./ic_rhoe]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '0.0025 1e8
               1.2 1e-8'
  [../]
[]
############################################################
[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.66666666666666666667
      R = 0.71428571428571428571
    [../]
  [../]
[]
############################################################
[UserObjects]

  [./symm_bcuo]
    execute_on = 'linear'
    type = CNSFVSlipBCUserObject
  [../]

  [./rslope]
    execute_on = 'linear'
    type = CNSFVSlopeReconstructionOneD
  [../]

  [./lslope]
    execute_on = 'linear'
    type = CNSFVSlopeLimitingOneD
    scheme = 'minmod' #none | minmod | mc | superbee
  [../]

  [./riemann]
    execute_on = 'linear'
    type = CNSFVHLLCInternalSideFlux
  [../]

  [./free_bc]
    execute_on = 'linear'
    type = CNSFVFreeOutflowBoundaryFlux
  [../]

  [./symm_bc]
    execute_on = 'linear'
    type = CNSFVHLLCSlipBoundaryFlux
    bc_uo = 'symm_bcuo'
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
  [./left_mass]
    type = CNSFVBC
    boundary = 'left'
    variable = rho
    component = 'mass'
    flux = symm_bc
  [../]

  [./left_momx]
    type = CNSFVBC
    boundary = 'left'
    variable = rhou
    component = 'x-momentum'
    flux = symm_bc
  [../]

  [./left_etot]
    type = CNSFVBC
    boundary = 'left'
    variable = rhoe
    component = 'total-energy'
    flux = symm_bc
  [../]

  [./right_mass]
    type = CNSFVBC
    boundary = 'right'
    variable = rho
    component = 'mass'
    flux = free_bc
  [../]

  [./right_momx]
    type = CNSFVBC
    boundary = 'right'
    variable = rhou
    component = 'x-momentum'
    flux = free_bc
  [../]

  [./right_etot]
    type = CNSFVBC
    boundary = 'right'
    variable = rhoe
    component = 'total-energy'
    flux = free_bc
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
[Postprocessors]
  [./dt]
    type = CNSFVTimeStepLimit
    execute_on = 'initial timestep_end'
    cfl = 0.8
  [../]

  [./DT]
    type = TimestepSize
    execute_on = 'initial timestep_end'
  [../]
[]
############################################################
[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ExplicitTVDRK2
  [../]
  solve_type = 'LINEAR'

  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = dt
  [../]

  l_tol = 1e-4
  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  start_time = 0.0
  end_time   = 0.000773
  num_steps = 4  # 4 | 2000 for 400 cells | 4000 for 800 cells
  dt = 1e-6
  dtmin = 1e-8
[]

[Outputs]
  [./Exodus]
    type = Exodus
    execute_on = 'initial timestep_end final'
    file_base = 1d_sedov_blast_wave_dtmin_out
    interval = 1 # 1 | 20 for 400 cells | 40 for 800 cells
  [../]

  [./CONSOLE]
    type = Console
    output_linear = true
    output_nonlinear = true
    execute_postprocessors_on = 'none'
    interval = 1
  [../]

  print_perf_log = true
[]

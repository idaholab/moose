############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  rho  = rho
  rhou = momx
  rhov = momy
  rhoe = rhoe
  fluid_properties = fp
  slope_reconstruction = rslope
  slope_limiting = lslope
  boundary_list = 'left
                   right
                   bottom
                   top'
  boundary_condition_user_object_list = 'free_outflow_bcuo
                                         free_outflow_bcuo
                                         free_outflow_bcuo
                                         free_outflow_bcuo'
  implicit = false
[]
############################################################
[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 50
  ny = 50
[]
############################################################
[Problem]
  kernel_coverage_check = false
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
  [./free_outflow_bcuo]
    execute_on = 'linear'
    type = CNSFVFreeOutflowBCUserObject
  [../]

  [./rslope]
    execute_on = 'linear'
    type = CNSFVGreenGaussSlopeReconstruction
  [../]

  [./lslope]
    execute_on = 'linear'
    type = CNSFVMinmaxSlopeLimiting
  [../]

  [./riemann]
    execute_on = 'linear'
    type = CNSFVHLLCInternalSideFlux
  [../]

  [./free_outflow_bc]
    execute_on = 'linear'
    type = CNSFVFreeOutflowBoundaryFlux
  [../]
[]
############################################################
[Variables]
  [./rho]
  [../]

  [./momx]
  [../]

  [./momy]
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
    variable = 'rho'
    type = BoundingBoxIC
    x1 = 0.1
    y1 = 0.1
    x2 = 0.6
    y2 = 0.6
    inside = 1.0
    outside = 0.5
  [../]

  [./momx_ic]
    variable = 'momx'
    type = BoundingBoxIC
    x1 = 0.1
    y1 = 0.1
    x2 = 0.6
    y2 = 0.6
    inside = 0.7071067811865475244
    outside = 0.3535533905932737622
  [../]

  [./momy_ic]
    variable = 'momy'
    type = BoundingBoxIC
    x1 = 0.1
    y1 = 0.1
    x2 = 0.6
    y2 = 0.6
    inside = 0.7071067811865475244
    outside = 0.3535533905932737622
  [../]

  [./rhoe_ic]
    variable = 'rhoe'
    type = BoundingBoxIC
    x1 = 0.1
    y1 = 0.1
    x2 = 0.6
    y2 = 0.6
    inside = 3.0
    outside = 2.75
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
  [./time_momx]
    implicit = true
    type = TimeDerivative
    variable = momx
  [../]

  #### Time derivative of momentum in y-direction
  [./time_momy]
    implicit = true
    type = TimeDerivative
    variable = momy
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
    variable = momx
    component = 'x-momentum'
    flux = riemann
  [../]

  #### Momentum balance eqn in y-direction
  [./momy]
    type = CNSFVKernel
    variable = momy
    component = 'y-momentum'
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
    boundary = 'left right bottom top'
    variable = rho
    component = 'mass'
    flux = free_outflow_bc
  [../]

  [./momx]
    type = CNSFVBC
    boundary = 'left right bottom top'
    variable = momx
    component = 'x-momentum'
    flux = free_outflow_bc
  [../]

  [./momy]
    type = CNSFVBC
    boundary = 'left right bottom top'
    variable = momy
    component = 'y-momentum'
    flux = free_outflow_bc
  [../]

  [./etot]
    type = CNSFVBC
    boundary = 'left right bottom top'
    variable = rhoe
    component = 'total-energy'
    flux = free_outflow_bc
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
    type = ExplicitTVDRK2
  [../]
  solve_type = 'LINEAR'

  l_tol = 1e-4
  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  start_time = 0.0
  num_steps = 2 # 4 | 1500 for complete run
  dt = 2e-4
  dtmin = 1e-6
[]

[Outputs]
  [./Exodus]
    type = Exodus
    file_base = 2d_square_wave_gg_minmax_out
    interval = 1 # 1 | 10 for complete run
  [../]
  print_perf_log = true
[]

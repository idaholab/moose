rho_left = 1
E_left = 2.501505578
u_left = 1e-15

rho_right = 0.125
E_right = 1.999770935
u_right = 1e-15

middle = 0.5

[GlobalParams]
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${fparse 2 * middle}
    nx = 5
    ymin = 0
    ymax = 1
    ny = 2
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    allow_imperfect_jacobians = true
  []
[]

[Variables]
  [rho]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [rho_u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [rho_v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1e-10
  []
  [rho_E]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [mass_time]
    type = FVTimeKernel
    variable = rho
  []

  [mass_advection]
    type = CNSFVMassHLLC
    variable = rho
  []

  [momentum_x_time]
    type = FVTimeKernel
    variable = rho_u
  []

  [momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
  []

  [momentum_y_time]
    type = FVTimeKernel
    variable = rho_v
  []

  [momentum_y_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v
    momentum_component = y
  []

  [fluid_energy_time]
    type = FVTimeKernel
    variable = rho_E
  []

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_E
  []
[]

[FVBCs]
  [mass_implicit]
    type = CNSFVHLLCMassImplicitBC
    variable = rho
    fp = fp
    boundary = 'left right'
  []

  [mom_x_implicit]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_u
    momentum_component = x
    fp = fp
    boundary = 'left right'
  []

  [wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_v
    momentum_component = y
    boundary = 'top bottom'
  []

  [fluid_energy_implicit]
    type = CNSFVHLLCFluidEnergyImplicitBC
    variable = rho_E
    fp = fp
    boundary = 'left right'
  []
[]

[ICs]
  [rho_ic]
    type = FunctionIC
    variable = rho
    function = 'if (x < ${middle}, ${rho_left}, ${rho_right})'
  []

  [rho_u_ic]
    type = FunctionIC
    variable = rho_u
    function = 'if (x < ${middle}, ${fparse rho_left * u_left}, ${fparse rho_right * u_right})'
  []

  [rho_E_ic]
    type = FunctionIC
    variable = rho_E
    function = 'if (x < ${middle}, ${fparse E_left * rho_left}, ${fparse E_right * rho_right})'
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rhov = rho_v
    rho_et = rho_E
    fp = fp
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 2
  []
  l_tol = 1e-8

  # run to t = 0.15
  start_time = 0.0
  dt = 1e-1
  end_time = 10
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
[]

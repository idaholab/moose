rho_left = 1
E_left = 2.501505578
u_left = 1e-15

rho_right = 0.125
E_right = 1.999770935
u_right = 1e-15

middle = 50

[GlobalParams]
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = ${fparse 2 * middle}
    nx = 1000
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
      allow_imperfect_jacobians = true
    []
  []
[]

[Problem]
  kernel_coverage_check = false
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
  [rho_E]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxVariables]
  [rho_a]
    order = CONSTANT
    family = MONOMIAL
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

  [momentum_time]
    type = FVTimeKernel
    variable = rho_u
  []

  [momentum_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
  []

  [fluid_energy_time]
    type = FVTimeKernel
    variable = rho_E
  [../]

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_E
  []
[]

[FVBCs]
  #[./mom_pressure]
  #  type = FVMomImplicitPressureBC
  #  variable = rho_u
  #  momentum_component = x
  #  boundary = 'left right'
  #[../]

  [mass_implicit]
    type = CNSFVHLLCMassImplicitBC
    variable = rho
    fp = fp
    boundary = 'left right'
  []

  [mom_implicit]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_u
    momentum_component = x
    fp = fp
    boundary = 'left right'
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
    type = ConservedVarMaterial
    rho = rho
    rhou = rho_u
    rho_et = rho_E
    fp = fp
  []

  [fluid_props]
    type = GeneralFluidProps
    porosity = 1
    pebble_diameter = 0.06
    fp = fp
  []
[]

[Preconditioning]
  active = ''
  [./smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    petsc_options = '-snes_converged_reason'
  [../]
[]

[Executioner]
  type = Transient
  scheme = explicit-tvd-rk-2
  solve_type = LINEAR

  l_tol = 1e-4

  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  # run to t = 0.15
  start_time = 0.0
  dt = 1e-2
  end_time = 20
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
[]

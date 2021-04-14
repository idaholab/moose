mass_flux=1.28969

[GlobalParams]
  fp = fp
  flux_interp_method = 'average'
[]

[Mesh]
  [./gen_mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '.1 .1 .1 .1 .1 .5 .1 .1 .1 .1 .1'
  [../]
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
    initial_condition = ${mass_flux}
    scaling = 1e3
  []
  [rho_u]
    type = MooseVariableFVReal
    # initial_condition = 1e-15
    initial_condition = ${mass_flux}
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = 2.525e5
    scaling = 1e-2
  []
[]

[FVKernels]
  [mass_advection]
    type = NSFVMassFluxAdvection
    variable = rho
    advected_quantity = '1'
  []

  [momentum_advection]
    type = NSFVMassFluxAdvection
    variable = rho_u
    advected_quantity = 'vel_x'
  []
  [momentum_pressure]
    type = NSFVMomentumPressureFluxKernel
    variable = rho_u
    momentum_component = 'x'
    force_boundary_execution = true
    boundaries_to_not_force = 'right'
  []
  [drag]
    type = FVReaction
    variable = rho_u
    rate = 1000
  []

  [fluid_energy_advection]
    type = NSFVMassFluxAdvection
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[FVBCs]
  [rho_left]
    type = FVNeumannBC
    variable = 'rho'
    boundary = 'left'
    value = ${mass_flux}
  []
  [rho_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho
    advected_quantity = '1'
  []

  [momentum_left]
    type = FVDirichletBC
    variable = rho_u
    boundary = 'left'
    value = ${mass_flux}
  []
  [momentum_advection_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho_u
    advected_quantity = 'vel_x'
  []
  [momentum_pressure_right]
    type = CNSFVMomSpecifiedPressureBC
    specified_pressure = 1.01e5
    boundary = 'right'
    variable = rho_u
  []

  [energy_left]
    type = NSFVFluidEnergySpecifiedTemperatureBC
    variable = rho_et
    boundary = 'left'
    temperature = 273.15
    rhou = ${mass_flux}
    interp_method = 'average'
  []
  [energy_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rho_et = rho_et
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = none
[]

[Outputs]
  exodus = true
  csv = true
[]

[Debug]
  show_var_residual_norms = true
[]

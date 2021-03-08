rho_u_in=1.28969
mass_flux_in=${rho_u_in}
p_out=1.01e5
T_in=273.15

[GlobalParams]
  fp = fluid_properties_obj
  vel = 'mass_flux'
  two_term_boundary_expansion = true
  advected_interp_method = 'upwind'
[]

[Mesh]
  [cartesian_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 2
    nx = 2
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [rho]
    initial_condition = 1.28969
    type = MooseVariableFVReal
  []
  [rho_u]
    initial_condition = 1.28969
    type = MooseVariableFVReal
  []
  [rho_et]
    initial_condition = 2.525e5
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [mass_advection]
    type = FVMatAdvection
    variable = rho
    advected_quantity = '1'
  []

  [x_momentum_pressure]
    type = NSFVMomentumPressureFluxKernel
    variable = rho_u
    momentum_component = 'x'
  []
  [x_momentum_sink]
    type = FVBodyForce
    variable = rho_u
    value = ${fparse -p_out}
  []

  [fluid_energy_advection]
    type = FVMatAdvection
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[Modules]
  [FluidProperties]
    [fluid_properties_obj]
      type = IdealGasFluidProperties
    []
  []
[]

[Materials]
  [non_linear_variables]
    type = ConservedVarValuesMaterial
    rho = rho
    rho_et = rho_et
    rhou = rho_u
  []
[]

[FVBCs]
  [rho_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho
    value = ${mass_flux_in}
  []
  [rho_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho
    advected_quantity = '1'
  []

  [rho_u_left]
    type = FVDirichletBC
    boundary = 'left'
    variable = rho_u
    value = ${rho_u_in}
  []
  [rho_u_pressure]
    type = FVNeumannBC
    variable = rho_u
    boundary = right
    value = ${fparse -p_out}
  []

  [rho_et_left]
    type = NSFVFluidEnergySpecifiedTemperatureBC
    boundary = 'left'
    variable = rho_et
    temperature = ${T_in}
    rhou = ${rho_u_in}
  []
  [rho_et_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

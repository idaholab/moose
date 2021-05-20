superficial_rho_u_in=1.28969
superficial_rho_v_in=0
eps=0.9
p_out=1.01e5
T_in=273.15
cl=${p_out}
global_interp_method='average'

[GlobalParams]
  fp = fluid_properties_obj
  two_term_boundary_expansion = true
  flux_interp_method = ${global_interp_method}
  interp_method = ${global_interp_method}
[]

[Mesh]
  [cartesian_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 10
    nx = 100
    ymax = 2
    ny = 20
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[AuxVariables]
  [pressure]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [pressure]
    type = ADMaterialRealAux
    variable = pressure
    property = pressure
    execute_on = 'timestep_end'
  []
  [temperature]
    type = ADMaterialRealAux
    variable = temperature
    property = T_fluid
    execute_on = 'timestep_end'
  []
[]

[Variables]
  [rho]
    initial_condition = 1.28969
    type = MooseVariableFVReal
    scaling = 1
  []
  [superficial_rho_u]
    initial_condition = 1e-15
    type = MooseVariableFVReal
    scaling = ${fparse 1 / p_out}
  []
  [superficial_rho_v]
    initial_condition = 1e-15
    type = MooseVariableFVReal
    scaling = ${fparse 1 / p_out}
  []
  [rho_et]
    initial_condition = 2.525e5
    type = MooseVariableFVReal
    scaling = ${fparse 1 / p_out}
  []
[]

[FVKernels]
  [mass_time]
    type = FVPorosityTimeDerivative
    variable = rho
  []
  [mass_advection]
    type = NSFVMassFluxAdvection
    variable = rho
    advected_quantity = '1'
  []

  [x_momentum_time]
    type = FVTimeKernel
    variable = superficial_rho_u
  []
  [x_momentum_advection]
    type = NSFVMassFluxAdvection
    variable = superficial_rho_u
    advected_quantity = 'vel_x'
  []
  [x_momentum_pressure]
    type = PCNSFVMomentumPressureFlux
    variable = superficial_rho_u
    momentum_component = 'x'
    boundaries_to_force = 'top bottom'
  []
  [x_momentum_friction_source]
    type = PNSFVMomentumFriction
    variable = superficial_rho_u
    momentum_component = 'x'
    Darcy_name = 'cl'
    momentum_name = superficial_rhou
  []

  [y_momentum_time]
    type = FVTimeKernel
    variable = superficial_rho_v
  []
  [y_momentum_advection]
    type = NSFVMassFluxAdvection
    variable = superficial_rho_v
    advected_quantity = 'vel_y'
  []
  [y_momentum_pressure]
    type = PCNSFVMomentumPressureFlux
    variable = superficial_rho_v
    momentum_component = 'y'
    boundaries_to_force = 'top bottom'
  []
  [y_momentum_friction_source]
    type = PNSFVMomentumFriction
    variable = superficial_rho_v
    momentum_component = 'y'
    Darcy_name = 'cl'
    momentum_name = superficial_rhov
  []

  [fluid_energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [fluid_energy_advection]
    type = NSFVMassFluxAdvection
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
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = superficial_rho_u
    superficial_rhov = superficial_rho_v
    porosity = 'porosity'
  []
  [generic]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps}'
  []
  [ad_generic]
    type = ADGenericConstantVectorMaterial
    prop_names = 'cl'
    prop_values = '${cl} ${cl} ${cl}'
  []
[]

[FVBCs]
  [rho_left]
    type = PNSFVMassSpecifiedMassFluxBC
    boundary = 'left'
    variable = rho
    superficial_rhou = ${superficial_rho_u_in}
    superficial_rhov = ${superficial_rho_v_in}
  []
  [rho_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho
    advected_quantity = '1'
  []

  [superficial_rho_u_left]
    type = FVDirichletBC
    boundary = 'left'
    variable = superficial_rho_u
    value = ${superficial_rho_u_in}
  []
  [superficial_rho_u_advection_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = superficial_rho_u
    advected_quantity = 'vel_x'
  []
  [superficial_rho_u_pressure_right]
    type = PNSFVMomentumSpecifiedPressureBC
    boundary = 'right'
    variable = superficial_rho_u
    pressure = ${p_out}
    momentum_component = 'x'
  []

  [superficial_rho_v_left]
    type = FVDirichletBC
    boundary = 'left'
    variable = superficial_rho_v
    value = ${superficial_rho_v_in}
  []
  [superficial_rho_v_advection_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = superficial_rho_v
    advected_quantity = 'vel_y'
  []
  [superficial_rho_v_pressure_right]
    type = PNSFVMomentumSpecifiedPressureBC
    boundary = 'right'
    variable = superficial_rho_v
    pressure = ${p_out}
    momentum_component = 'y'
  []

  [rho_et_left]
    type = PNSFVFluidEnergySpecifiedTemperatureBC
    boundary = 'left'
    variable = rho_et
    temperature = ${T_in}
    superficial_rhou = ${superficial_rho_u_in}
    superficial_rhov = ${superficial_rho_v_in}
  []
  [rho_et_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[Executioner]
  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  type = Transient
  num_steps = 1000
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-8
    optimal_iterations = 6
  []
  steady_state_detection = true
  line_search = 'bt'
  abort_on_solve_fail = true
  verbose = true
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_max_its = 20
  steady_state_tolerance = 1e-12
[]

[Outputs]
  [out]
    type = Exodus
  []
[]

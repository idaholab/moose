molar_mass=29e-3
R=8.3145
gamma=1.4
eps_bottom=1
eps_top=0.5
eps_out=${eps_top}
p_stp=1.01e5
p_initial_bottom=${p_stp}
p_initial_top=${fparse 2 * p_stp}
p_out=${p_initial_top}
T_stp=273.15
T_initial_bottom=${T_stp}
T_initial_top=${T_stp}
T_in=${T_initial_bottom}

rho_initial_bottom=${fparse p_initial_bottom * molar_mass / (R * T_initial_bottom)}
rho_initial_top=${fparse p_initial_top * molar_mass / (R * T_initial_top)}
e_initial_bottom=${fparse p_initial_bottom / (gamma - 1) / rho_initial_bottom}
e_initial_top=${fparse p_initial_top / (gamma - 1) / rho_initial_top}
rho_et_initial_bottom=${fparse rho_initial_bottom * e_initial_bottom}
rho_et_initial_top=${fparse rho_initial_top * e_initial_top}

rho_ud_in=0
rho_vd_in=1
mass_flux_in=${rho_vd_in}

two_term_boundary_expansion=true

[GlobalParams]
  advected_interp_method = 'upwind'
  vel = 'velocity'
  # momentum_component = 'nonsense'
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    nx = 10
    ymin = 0
    ymax = 10
    ny = 100
  []
  [top]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 5 0'
    top_right = '1 10 0'
    block_id = 1
  []
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
  coord_type = RZ
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_ud]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_vd]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_et]
    type = MooseVariableFVReal
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
[]

[ICs]
  [rho_bottom]
    type = ConstantIC
    value = ${rho_initial_bottom}
    block = 0
    variable = rho
  []
  [rho_top]
    type = ConstantIC
    value = ${rho_initial_top}
    block = 1
    variable = rho
  []
  [rho_et_bottom]
    type = ConstantIC
    value = ${rho_et_initial_bottom}
    variable = rho_et
    block = 0
  []
  [rho_et_top]
    type = ConstantIC
    value = ${rho_et_initial_top}
    variable = rho_et
    block = 1
  []
[]

[AuxVariables]
  [specific_volume]
    type = MooseVariableFVReal
  []
  [superficial_vel_y]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
  []
  [vel_y]
    type = MooseVariableFVReal
  []
  [specific_internal_energy]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
  [mass_flux]
    type = MooseVariableFVReal
  []
  [momentum_flux]
    type = MooseVariableFVReal
  []
  [enthalpy_flux]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [specific_volume]
    type = SpecificVolumeAux
    variable = specific_volume
    rho = rho
    execute_on = 'timestep_end'
  []
  [superficial_vel_y]
    type = NSVelocityAux
    variable = superficial_vel_y
    rho = rho
    momentum = rho_vd
    execute_on = 'timestep_end'
  []
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
  [vel_y]
    type = ParsedAux
    variable = vel_y
    function = 'superficial_vel_y / porosity'
    args = 'superficial_vel_y porosity'
    execute_on = 'timestep_end'
  []
  [specific_internal_energy]
    type = ParsedAux
    variable = specific_internal_energy
    function = 'rho_et / rho - (vel_y * vel_y) / 2'
    args = 'rho_et rho vel_y'
    execute_on = 'timestep_end'
  []
  [pressure]
    type = ADMaterialRealAux
    variable = pressure
    property = pressure
    execute_on = 'timestep_end'
  []
  [temperature]
    type = TemperatureAux
    variable = temperature
    v = specific_volume
    e = specific_internal_energy
    fp = fp
    execute_on = 'timestep_end'
  []
  [mass_flux]
    type = ParsedAux
    variable = mass_flux
    function = 'rho_vd'
    args = 'rho_vd'
    execute_on = 'timestep_end'
  []
  [momentum_flux]
    type = ParsedAux
    variable = momentum_flux
    function = 'superficial_vel_y * rho_vd / porosity + pressure * porosity'
    args = 'superficial_vel_y rho_vd porosity pressure'
    execute_on = 'timestep_end'
  []
  [enthalpy_flux]
    type = ParsedAux
    variable = enthalpy_flux
    function = 'superficial_vel_y * (rho_et + pressure)'
    args = 'superficial_vel_y rho_et pressure'
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_time]
    type = FVPorosityTimeDerivative
    variable = rho
  []
  [mass_advection]
    type = FVMatAdvection
    variable = rho
    advected_quantity = 'superficial_rho'
  []

  [momentum_time_x]
    type = FVTimeKernel
    variable = rho_ud
  []
  [momentum_advection_x]
    type = FVMatAdvection
    variable = rho_ud
  []
  [momentum_pressure_x]
    type = NSFVPorosityMomentumPressure
    variable = rho_ud
    momentum_component = 'x'
  []
  [momentum_pressure_rz_x]
    type = NSFVPorosityMomentumPressureRZ
    variable = rho_ud
  []

  [momentum_time_y]
    type = FVTimeKernel
    variable = rho_vd
  []
  [momentum_advection_y]
    type = FVMatAdvection
    variable = rho_vd
  []
  [momentum_pressure_y]
    type = NSFVPorosityMomentumPressure
    variable = rho_vd
    momentum_component = 'y'
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [energy_advection]
    type = FVMatAdvection
    variable = rho_et
    advected_quantity = 'superficial_rho_ht'
  []
[]

[FVBCs]
  [rho_bottom]
    type = FVNeumannBC
    boundary = 'bottom'
    variable = rho
    value = ${mass_flux_in}
  []
  [rho_top]
    type = FVMatAdvectionOutflowBC
    boundary = 'top'
    variable = rho
    advected_quantity = 'superficial_rho'
  []

  [rho_ud_bottom] # implicitly includes advection and pressure
    type = FVDirichletBC
    boundary = 'bottom'
    variable = rho_ud
    value = ${rho_ud_in}
  []
  # no advection through axis/walls
  [rho_ud_pressure_axis_walls]
    type = NSFVPorosityMomentumPressureBC
    boundary = 'left right'
    variable = rho_ud
    momentum_component = 'x'
  []
  [rho_ud_advection_top]
    type = FVMatAdvectionOutflowBC
    boundary = 'top'
    variable = rho_ud
  []
  [rho_ud_pressure_top] # There is no normal in x so no "pressure flux"
    type = FVNeumannBC
    boundary = 'top'
    variable = rho_ud
    value = 0
  []

  [rho_vd_bottom] # implicitly includes advection and pressure
    type = FVDirichletBC
    boundary = 'bottom'
    variable = rho_vd
    value = ${rho_vd_in}
  []
  [rho_vd_pressure_axis_walls]
    type = NSFVPorosityMomentumPressureBC
    boundary = 'left right'
    variable = rho_vd
    momentum_component = 'y'
  []
  [rho_vd_advection_top]
    type = FVMatAdvectionOutflowBC
    boundary = 'top'
    variable = rho_vd
  []
  [rho_vd_pressure_top] # Normal in y so apply all of pressure
    type = FVNeumannBC
    boundary = 'top'
    variable = rho_vd
    value = ${fparse -p_out * eps_out}
  []

  [rho_et_bottom]
    type = NSFVFluidEnergySpecifiedTemperatureBC
    boundary = 'bottom'
    variable = rho_et
    temperature = ${T_in}
    superficial_rhou = ${rho_ud_in}
    superficial_rhov = ${rho_vd_in}
    fp = fp
  []
  [rho_et_top]
    type = FVMatAdvectionOutflowBC
    boundary = 'top'
    variable = rho_et
    advected_quantity = 'superficial_rho_ht'
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = rho_ud
    superficial_rhov = rho_vd
    fp = fp
    porosity = porosity
  []
  [porosity_bottom]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps_bottom}'
    block = 0
  []
  [porosity_top]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps_top}'
    block = 1
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
  petsc_options_value = 'lu       superlu_dist'
  nl_max_its = 10
  steady_state_tolerance = 1e-13
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'initial timestep_end'
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [mass_flux_bottom]
    type = SideIntegralVariablePostprocessor
    variable = rho_vd
    boundary = 'bottom'
  []
  [mass_flux_top]
    type = SideIntegralVariablePostprocessor
    variable = rho_vd
    boundary = 'top'
  []
[]

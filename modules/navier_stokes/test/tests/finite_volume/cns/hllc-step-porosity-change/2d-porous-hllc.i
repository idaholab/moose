# Establish initial conditions based on STP
rho_initial_left=1.28969
p_initial_left=1.01e5
p_initial_right=${fparse 2 * p_initial_left}
gamma=1.4
e_initial_left=${fparse p_initial_left / (gamma - 1) / rho_initial_left}
# No bulk velocity in the domain initially
et_initial_left=${e_initial_left}
rho_et_initial_left=${fparse rho_initial_left * et_initial_left}

rho_ud_in=1.28969
rho_vd_in=0
eps_in=1
# prescribe outlet pressure = initial pressure on right
p_out=${p_initial_right}
eps_out=0.5
T_in=273.15


[GlobalParams]
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    nx = 100
    ymin = 0
    ymax = 2
    ny = 20
  []
  [constant_again_porosity]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '5 0 0'
    top_right = '10 2 1'
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
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
  []
  [rho_ud]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_vd]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_et]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [rho_left]
    type = ConstantIC
    value = ${rho_initial_left}
    block = 0
    variable = rho
  []
  [rho_right]
    type = ConstantIC
    value = ${fparse 2 * rho_initial_left}
    block = 1
    variable = rho
  []
  [rho_et_left]
    type = ConstantIC
    value = ${rho_et_initial_left}
    variable = rho_et
    block = 0
  []
  [rho_et_right]
    type = ConstantIC
    value = ${fparse 2 * rho_et_initial_left}
    variable = rho_et
    block = 1
  []
[]

[FVKernels]
  [mass_time]
    type = FVPorosityTimeDerivative
    variable = rho
  []
  [mass_advection]
    type = PCNSFVMassHLLC
    variable = rho
    fp = fp
  []

  [momentum_time_x]
    type = FVTimeKernel
    variable = rho_ud
  []
  [momentum_x_advection]
    type = PCNSFVMomentumHLLC
    variable = rho_ud
    momentum_component = x
    fp = fp
  []

  [momentum_time_y]
    type = FVTimeKernel
    variable = rho_vd
  []
  [momentum_y_advection]
    type = PCNSFVMomentumHLLC
    variable = rho_vd
    momentum_component = y
    fp = fp
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [fluid_energy_advection]
    type = PCNSFVFluidEnergyHLLC
    variable = rho_et
    fp = fp
  []
[]

[FVBCs]
  [mass_in]
    variable = rho
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC
    boundary = left
    temperature = ${T_in}
    superficial_rhou = ${rho_ud_in}
    superficial_rhov = ${rho_vd_in}
  []
  [momentum_x_in]
    variable = rho_ud
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    boundary = left
    temperature = ${T_in}
    superficial_rhou = ${rho_ud_in}
    superficial_rhov = ${rho_vd_in}
    momentum_component = 'x'
  []
  [momentum_y_in]
    variable = rho_vd
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    boundary = left
    temperature = ${T_in}
    superficial_rhou = ${rho_ud_in}
    superficial_rhov = ${rho_vd_in}
    momentum_component = 'y'
  []
  [energy_in]
    variable = rho_et
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC
    boundary = left
    temperature = ${T_in}
    superficial_rhou = ${rho_ud_in}
    superficial_rhov = ${rho_vd_in}
  []

  [mass_out]
    variable = rho
    type = PCNSFVHLLCSpecifiedPressureMassBC
    boundary = right
    pressure = ${p_out}
  []
  [momentum_x_out]
    variable = rho_ud
    type = PCNSFVHLLCSpecifiedPressureMomentumBC
    boundary = right
    pressure = ${p_out}
    momentum_component = 'x'
  []
  [momentum_y_out]
    variable = rho_vd
    type = PCNSFVHLLCSpecifiedPressureMomentumBC
    boundary = right
    pressure = ${p_out}
    momentum_component = 'y'
  []
  [energy_out]
    variable = rho_et
    type = PCNSFVHLLCSpecifiedPressureFluidEnergyBC
    boundary = right
    pressure = ${p_out}
  []

  [momentum_x_walls]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'x'
    boundary = 'top bottom'
    variable = rho_ud
  []
  [momentum_y_walls]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'y'
    boundary = 'top bottom'
    variable = rho_vd
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    superficial_rhou = rho_ud
    superficial_rhov = rho_vd
    rho_et = rho_et
    porosity = porosity
  []
  [porosity_left]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps_in}'
    block = 0
  []
  [porosity_right]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps_out}'
    block = 1
  []
[]

[AuxVariables]
  [specific_volume]
    type = MooseVariableFVReal
  []
  [vel_x]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
  []
  [real_vel_x]
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
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
  [vel_x]
    type = NSVelocityAux
    variable = vel_x
    rho = rho
    momentum = rho_ud
    execute_on = 'timestep_end'
  []
  [real_vel_x]
    type = ParsedAux
    variable = real_vel_x
    function = 'vel_x / porosity'
    args = 'vel_x porosity'
    execute_on = 'timestep_end'
  []
  [specific_internal_energy]
    type = ParsedAux
    variable = specific_internal_energy
    function = 'rho_et / rho - (real_vel_x * real_vel_x) / 2'
    args = 'rho_et rho real_vel_x'
    execute_on = 'timestep_end'
  []
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
  [mass_flux]
    type = ParsedAux
    variable = mass_flux
    function = 'rho_ud'
    args = 'rho_ud'
    execute_on = 'timestep_end'
  []
  [momentum_flux]
    type = ParsedAux
    variable = momentum_flux
    function = 'vel_x * rho_ud / porosity + pressure * porosity'
    args = 'vel_x rho_ud porosity pressure'
    execute_on = 'timestep_end'
  []
  [enthalpy_flux]
    type = ParsedAux
    variable = enthalpy_flux
    function = 'vel_x * (rho_et + pressure)'
    args = 'vel_x rho_et pressure'
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
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
  nl_max_its = 10
  steady_state_tolerance = 1e-13
[]

[Outputs]
  [out]
    type = Exodus
  []
[]

[Debug]
  show_var_residual_norms = true
[]

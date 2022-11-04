# PorousFlow simulation of injection and production in a 2D aquifer
# Much of this file is standard porous-flow stuff.  The unusual aspects are:
# - transfer of the rates of changes of each species (kg/s) to the aquifer_geochemistry.i simulation.  This is achieved by saving these changes from the PorousFlowMassTimeDerivative residuals
# - transfer of the temperature field to the aquifer_geochemistry.i simulation
# Interesting behaviour can be simulated by this file without its "parent" simulation, exchanger.i.  exchanger.i provides mass-fractions injected via the injection_rate_massfrac_* variables, but since these are more-or-less constant throughout the duration of the exchanger.i simulation, the initial_conditions specified below may be used.  Similar, exchanger.i provides injection_temperature, but that is also constant.

injection_rate = -1.0 # kg/s/m, negative because injection as a source
production_rate = 1.0 # kg/s/m
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 14 # for better resolution, use 56 or 112
    ny = 8  # for better resolution, use 32 or 64
    xmin = -70
    xmax = 70
    ymin = -40
    ymax = 40
  []
  [injection_node]
    input = gen
    type = ExtraNodesetGenerator
    new_boundary = injection_node
    coord = '-30 0 0'
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [f0]
    initial_condition = 0.002285946
  []
  [f1]
    initial_condition = 0.0035252
  []
  [f2]
    initial_condition = 1.3741E-05
  []
  [porepressure]
    initial_condition = 2E6
  []
  [temperature]
    initial_condition = 50
    scaling = 1E-6 # fluid enthalpy is roughly 1E6
  []
[]

[BCs]
  [injection_temperature]
    type = MatchedValueBC
    variable = temperature
    v = injection_temperature
    boundary = injection_node
  []
[]

[DiracKernels]
  [inject_Na]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    multiplying_var = injection_rate_massfrac_Na
    point_file = injection.bh
    variable = f0
  []
  [inject_Cl]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    multiplying_var = injection_rate_massfrac_Cl
    point_file = injection.bh
    variable = f1
  []
  [inject_SiO2]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    multiplying_var = injection_rate_massfrac_SiO2
    point_file = injection.bh
    variable = f2
  []
  [inject_H2O]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    multiplying_var = injection_rate_massfrac_H2O
    point_file = injection.bh
    variable = porepressure
  []

  [produce_Na]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Na
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    mass_fraction_component = 0
    point_file = production.bh
    variable = f0
  []
  [produce_Cl]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Cl
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    mass_fraction_component = 1
    point_file = production.bh
    variable = f1
  []
  [produce_SiO2]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_SiO2
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    mass_fraction_component = 2
    point_file = production.bh
    variable = f2
  []
  [produce_H2O]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_H2O
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    mass_fraction_component = 3
    point_file = production.bh
    variable = porepressure
  []
  [produce_heat]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_heat
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    line_length = 1.0
    use_enthalpy = true
    point_file = production.bh
    variable = temperature
  []
[]

[UserObjects]
  [injected_mass]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Na]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Cl]
    type = PorousFlowSumQuantity
  []
  [produced_mass_SiO2]
    type = PorousFlowSumQuantity
  []
  [produced_mass_H2O]
    type = PorousFlowSumQuantity
  []
  [produced_heat]
    type = PorousFlowSumQuantity
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
    execute_on = TIMESTEP_BEGIN
  []
  [tot_kg_injected_this_timestep]
    type = PorousFlowPlotQuantity
    uo = injected_mass
  []
  [kg_Na_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Na
  []
  [kg_Cl_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Cl
  []
  [kg_SiO2_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_SiO2
  []
  [kg_H2O_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_H2O
  []
  [mole_rate_Na_produced]
    type = FunctionValuePostprocessor
    function = moles_Na
    indirect_dependencies = 'kg_Na_produced_this_timestep dt'
  []
  [mole_rate_Cl_produced]
    type = FunctionValuePostprocessor
    function = moles_Cl
    indirect_dependencies = 'kg_Cl_produced_this_timestep dt'
  []
  [mole_rate_SiO2_produced]
    type = FunctionValuePostprocessor
    function = moles_SiO2
    indirect_dependencies = 'kg_SiO2_produced_this_timestep dt'
  []
  [mole_rate_H2O_produced]
    type = FunctionValuePostprocessor
    function = moles_H2O
    indirect_dependencies = 'kg_H2O_produced_this_timestep dt'
  []
  [heat_joules_extracted_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_heat
  []
  [production_temperature]
    type = PointValue
    point = '30 0 0'
    variable = temperature
  []
[]

[Functions]
  [moles_Na]
    type = ParsedFunction
    symbol_names = 'kg_Na dt'
    symbol_values = 'kg_Na_produced_this_timestep dt'
    expression = 'kg_Na * 1000 / 22.9898 / dt'
  []
  [moles_Cl]
    type = ParsedFunction
    symbol_names = 'kg_Cl dt'
    symbol_values = 'kg_Cl_produced_this_timestep dt'
    expression = 'kg_Cl * 1000 / 35.453 / dt'
  []
  [moles_SiO2]
    type = ParsedFunction
    symbol_names = 'kg_SiO2 dt'
    symbol_values = 'kg_SiO2_produced_this_timestep dt'
    expression = 'kg_SiO2 * 1000 / 60.0843 / dt'
  []
  [moles_H2O]
    type = ParsedFunction
    symbol_names = 'kg_H2O dt'
    symbol_values = 'kg_H2O_produced_this_timestep dt'
    expression = 'kg_H2O * 1000 / 18.0152 / dt'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0
    bulk_modulus = 2E9
    viscosity = 1E-3
    density0 = 1000
    cv = 4000.0
    cp = 4000.0
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = porepressure
  temperature = temperature
  mass_fraction_vars = 'f0 f1 f2'
  save_component_rate_in = 'rate_Na rate_Cl rate_SiO2 rate_H2O' # change in kg at every node / dt
  fp = the_simple_fluid
  temperature_unit = Celsius
[]

[AuxVariables]
  [injection_temperature]
    initial_condition = 200
  []
  [injection_rate_massfrac_Na]
    initial_condition = 0.002285946
  []
  [injection_rate_massfrac_Cl]
    initial_condition = 0.0035252
  []
  [injection_rate_massfrac_SiO2]
    initial_condition = 1.3741E-05
  []
  [injection_rate_massfrac_H2O]
    initial_condition = 0.994175112
  []
  [rate_H2O]
  []
  [rate_Na]
  []
  [rate_Cl]
  []
  [rate_SiO2]
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst # this simulation has no porosity changes from dissolution
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-12 0 0   0 1E-12 0   0 0 1E-12'
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0 0 0  0 0 0  0 0 0'
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    density = 2500.0
    specific_heat_capacity = 1200.0
  []
[]

[Preconditioning]
  active = typically_efficient
  [typically_efficient]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = ' hypre    boomeramg'
  []
  [strong]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      ilu           NONZERO                   2'
  []
  [probably_too_strong]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 7.76E6 # 90 days
  dt = 1E5
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [react]
    type = TransientMultiApp
    input_files = aquifer_geochemistry.i
    clone_master_mesh = true
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [changes_due_to_flow]
    type = MultiAppCopyTransfer
    source_variable = 'rate_H2O rate_Na rate_Cl rate_SiO2 temperature'
    variable = 'pf_rate_H2O pf_rate_Na pf_rate_Cl pf_rate_SiO2 temperature'
    to_multi_app = react
  []
  [massfrac_from_geochem]
    type = MultiAppCopyTransfer
    source_variable = 'massfrac_Na massfrac_Cl massfrac_SiO2'
    variable = 'f0 f1 f2'
    from_multi_app = react
  []
[]

# Input file modified from RobPodgorney version
# - 2D instead of 3D with different resolution.  Effectively this means a 1m height of RobPodgorney aquifer is simulated.  RobPodgorney total mass flux is 2.5kg/s meaning 0.25kg/s is appropriate here
# - Celsius instead of Kelvin
# - no use of PorousFlowPointEnthalpySourceFromPostprocessor since that is not yet merged into MOOSE: a DirichletBC is used instead
# - Use of PorousFlowFullySaturated instead of PorousFlowUnsaturated, and the save_component_rate_in feature to record the change in kg of each species at each node for passing to the Geochem simulation
# - MultiApps and Transfers to transfer information between this simulation and the aquifer_geochemistry.i simulation
production_rate = 1.0 # kg/s/m
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmin = 0
    xmax = 0.0508 #m
    ymin = 0
    ymax = 0.0254 #m
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [f_H]
    initial_condition = 8.201229858451E-07
  []
  [f_Na]
    initial_condition = 2.281094143525E-03
  []
  [f_K]
    initial_condition = 2.305489507836E-04
  []
  [f_Ca]
    initial_condition = 5.818776782059E-04
  []
  [f_Mg]
    initial_condition = 1.539513498238E-07
  []
  [f_SiO2]
    initial_condition = 2.691822196469E-04
  []
  [f_Al]
    initial_condition = 4.457519474122E-08
  []
  [f_Cl]
    initial_condition = 4.744309776594E-03
  []
  [f_SO4]
    initial_condition = 9.516650880811E-06
  []
  [f_HCO3]
    initial_condition = 5.906126982324E-05
  []
  [porepressure]
    initial_condition = 2E6
    scaling = 1E6
  []
  [temperature]
    initial_condition = 60 # degC
    scaling = 1E-6 # fluid enthalpy is roughly 1E6
  []
[]

[BCs]
  [in_left]
    type = DirichletBC
    variable = porepressure
    boundary = right
    value = 3E6
  []
  # [in_left]
  #   type = PorousFlowPiecewiseLinearSink
  #   variable = porepressure
  #   boundary = 'left'
  #   pt_vals = '-1e9 1e9' # x coordinates defining g
  #   multipliers = '-1e9 1e9' # y coordinates defining g
  #   PT_shift = 2.E6   # BC pressure
  #   flux_function = 1E-5 # Variable C
  #   fluid_phase = 0
  # []
  [out_right]
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = 'right'
    pt_vals = '-1e9 1e9' # x coordinates defining g
    multipliers = '-1e9 1e9' # y coordinates defining g
    PT_shift = 1.E6   # BC pressure
    flux_function = 1E-6 # Variable C
    fluid_phase = 0
  []
[]

[DiracKernels]
  [inject_H]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 4.790385871045E-08
    variable = f_H
  []
  [inject_Na]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 7.586252963780E-07
    variable = f_Na
  []
  [inject_K]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 2.746517625125E-07
    variable = f_K
  []
  [inject_Ca]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 7.775129478597E-07
    variable = f_Ca
  []
  [inject_Mg]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 1.749872109005E-07
    variable = f_Mg
  []
  [inject_SiO2]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 4.100547515915E-06
    variable = f_SiO2
  []
  [inject_Al]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 2.502408592080E-08
    variable = f_Al
  []
  [inject_Cl]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 1.997260386272E-06
    variable = f_Cl
  []
  [inject_SO4]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 2.497372164191E-07
    variable = f_SO4
  []
  [inject_HCO3]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 5.003150992902E-06
    variable = f_HCO3
  []
  [inject_H2O]
    type = PorousFlowPointSourceFromPostprocessor
    point = ' 0.0254 0 0'
    mass_flux = 2.499865905987E-01
    variable = porepressure
  []

  [produce_H]
    type = PorousFlowPolyLineSink
    variable = f_H
    SumQuantityUO = produced_mass_H
    mass_fraction_component = 0
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_Na]
    type = PorousFlowPolyLineSink
    variable = f_Na
    SumQuantityUO = produced_mass_Na
    mass_fraction_component = 1
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_K]
    type = PorousFlowPolyLineSink
    variable = f_K
    SumQuantityUO = produced_mass_K
    mass_fraction_component = 2
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_Ca]
    type = PorousFlowPolyLineSink
    variable = f_Ca
    SumQuantityUO = produced_mass_Ca
    mass_fraction_component = 3
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_Mg]
    type = PorousFlowPolyLineSink
    variable = f_Mg
    SumQuantityUO = produced_mass_Mg
    mass_fraction_component = 4
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_SiO2]
    type = PorousFlowPolyLineSink
    variable = f_SiO2
    SumQuantityUO = produced_mass_SiO2
    mass_fraction_component = 5
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_Al]
    type = PorousFlowPolyLineSink
    variable = f_Al
    SumQuantityUO = produced_mass_Al
    mass_fraction_component = 6
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_Cl]
    type = PorousFlowPolyLineSink
    variable = f_Cl
    SumQuantityUO = produced_mass_Cl
    mass_fraction_component = 7
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_SO4]
    type = PorousFlowPolyLineSink
    variable = f_SO4
    SumQuantityUO = produced_mass_SO4
    mass_fraction_component = 8
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_HCO3]
    type = PorousFlowPolyLineSink
    variable = f_HCO3
    SumQuantityUO = produced_mass_HCO3
    mass_fraction_component = 9
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
  [produce_H2O]
    type = PorousFlowPolyLineSink
    variable = porepressure
    SumQuantityUO = produced_mass_H2O
    mass_fraction_component = 10
    point_file = production.bh
    line_length = 1
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
  []
[]

[UserObjects]
  [produced_mass_H]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Na]
    type = PorousFlowSumQuantity
  []
  [produced_mass_K]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Ca]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Mg]
    type = PorousFlowSumQuantity
  []
  [produced_mass_SiO2]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Al]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Cl]
    type = PorousFlowSumQuantity
  []
  [produced_mass_SO4]
    type = PorousFlowSumQuantity
  []
  [produced_mass_HCO3]
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
  # [heat_extracted]
  #   type = PorousFlowPlotQuantity
  #   uo = produced_heat
  # []
  # [approx_production_temperature]
  #   type = PointValue
  #   point = '100 0 0'
  #   variable = temperature
  # []
  [mass_extracted_H]
    type = PorousFlowPlotQuantity
    uo = produced_mass_H
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Na]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Na
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_K]
    type = PorousFlowPlotQuantity
    uo = produced_mass_K
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Ca]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Ca
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Mg]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Mg
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SiO2]
    type = PorousFlowPlotQuantity
    uo = produced_mass_SiO2
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Al]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Al
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Cl]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Cl
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SO4]
    type = PorousFlowPlotQuantity
    uo = produced_mass_SO4
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_HCO3]
    type = PorousFlowPlotQuantity
    uo = produced_mass_HCO3
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_H2O]
    type = PorousFlowPlotQuantity
    uo = produced_mass_H2O
    execute_on = 'initial timestep_end'
  []
  [mass_extracted]
    type = LinearCombinationPostprocessor
    pp_names = 'mass_extracted_H mass_extracted_Na mass_extracted_K mass_extracted_Ca mass_extracted_Mg mass_extracted_SiO2 mass_extracted_Al mass_extracted_Cl mass_extracted_SO4 mass_extracted_HCO3 mass_extracted_H2O'
    pp_coefs = '1 1 1 1 1 1 1 1 1 1 1'
    execute_on = 'initial timestep_end'
  []
  [dt]
    type = TimestepSize
    execute_on = 'timestep_begin'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 2E-4
    bulk_modulus = 2E9
    viscosity = 1E-3
    density0 = 980
    cv = 4000.0
    cp = 4000.0
    porepressure_coefficient = 0
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = porepressure
  temperature = temperature
  mass_fraction_vars = 'f_H f_Na f_K f_Ca f_Mg f_SiO2 f_Al f_Cl f_SO4 f_HCO3'
  save_component_rate_in = 'rate_H rate_Na rate_K rate_Ca rate_Mg rate_SiO2 rate_Al rate_Cl rate_SO4 rate_HCO3 rate_H2O' # change in kg at every node / dt
  fp = the_simple_fluid
  temperature_unit = Celsius
  pressure_unit = MPa
[]

[AuxVariables]
  [rate_H]
  []
  [rate_Na]
  []
  [rate_K]
  []
  [rate_Ca]
  []
  [rate_Mg]
  []
  [rate_SiO2]
  []
  [rate_Al]
  []
  [rate_Cl]
  []
  [rate_SO4]
  []
  [rate_HCO3]
  []
  [rate_H2O]
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.01
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '2.5 0 0  0 2.5 0  0 0 2.5'
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    density = 2750.0
    specific_heat_capacity = 900.0
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
  end_time = 31536000 #1 year

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 10
    linear_iteration_ratio = 1
    dt = 200
  []
[]

[Outputs]
  exodus = true
  csv = true
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
    source_variable = 'rate_H rate_Na rate_K rate_Ca rate_Mg rate_SiO2 rate_Al rate_Cl rate_SO4 rate_HCO3 rate_H2O temperature'
    variable = 'pf_rate_H pf_rate_Na pf_rate_K pf_rate_Ca pf_rate_Mg pf_rate_SiO2 pf_rate_Al pf_rate_Cl pf_rate_SO4 pf_rate_HCO3 pf_rate_H2O temperature'
    to_multi_app = react
  []
  [massfrac_from_geochem]
    type = MultiAppCopyTransfer
    source_variable = 'massfrac_H massfrac_Na massfrac_K massfrac_Ca massfrac_Mg massfrac_SiO2 massfrac_Al massfrac_Cl massfrac_SO4 massfrac_HCO3'
    variable = 'f_H f_Na f_K f_Ca f_Mg f_SiO2 f_Al f_Cl f_SO4 f_HCO3'
    from_multi_app = react
  []
[]

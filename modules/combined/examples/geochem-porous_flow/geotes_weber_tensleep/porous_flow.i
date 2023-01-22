#########################################
#                                       #
# File written by create_input_files.py #
#                                       #
#########################################
# PorousFlow simulation of injection and production in a simplified GeoTES aquifer
# Much of this file is standard porous-flow stuff.  The unusual aspects are:
# - transfer of the rates of changes of each species (kg.s) to the aquifer_geochemistry.i simulation.  This is achieved by saving these changes from the PorousFlowMassTimeDerivative residuals
# - transfer of the temperature field to the aquifer_geochemistry.i simulation
# Interesting behaviour can be simulated by this file without its 'parent' simulation, exchanger.i.  exchanger.i provides mass-fractions injected via the injection_rate_massfrac_* variables, but since these are more-or-less constant throughout the duration of the exchanger.i simulation, the initial_conditions specified below may be used.  Similar, exchanger.i provides injection_temperature, but that is also constant.
injection_rate = -0.02 # kg/s/m, negative because injection as a source
production_rate = 0.02 # kg/s/m, this is about the maximum that can be sustained by the aquifer, with its fairly low permeability, without porepressure becoming negative
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -75
    xmax = 75
    ymin = 0
    ymax = 40
    zmin = -25
    zmax = 25
    nx = 15
    ny = 4
    nz = 5
  []
  [aquifer]
    type = ParsedSubdomainMeshGenerator
    input = gen
    block_id = 1
    block_name = aquifer
    combinatorial_geometry = 'z >= -5 & z <= 5'
  []
  [injection_nodes]
    input = aquifer
    type = ExtraNodesetGenerator
    new_boundary = injection_nodes
    coord = '-25 0 -5; -25 0 5'
  []
  [production_nodes]
    input = injection_nodes
    type = ExtraNodesetGenerator
    new_boundary = production_nodes
    coord = '25 0 -5; 25 0 5'
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 -10'
[]

[BCs]
  [injection_temperature]
    type = MatchedValueBC
    variable = temperature
    v = injection_temperature
    boundary = injection_nodes
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
  mass_fraction_vars = 'f_H f_Cl f_SO4 f_HCO3 f_SiO2aq f_Al f_Ca f_Mg f_Fe f_K f_Na f_Sr f_F f_BOH f_Br f_Ba f_Li f_NO3 f_O2aq '
  save_component_rate_in = 'rate_H rate_Cl rate_SO4 rate_HCO3 rate_SiO2aq rate_Al rate_Ca rate_Mg rate_Fe rate_K rate_Na rate_Sr rate_F rate_BOH rate_Br rate_Ba rate_Li rate_NO3 rate_O2aq rate_H2O' # change in kg at every node / dt
  fp = the_simple_fluid
  temperature_unit = Celsius
[]

[Materials]
  [porosity_caps]
    type = PorousFlowPorosityConst # this simulation has no porosity changes from dissolution
    block = 0
    porosity = 0.01
  []
  [porosity_aquifer]
    type = PorousFlowPorosityConst # this simulation has no porosity changes from dissolution
    block = aquifer
    porosity = 0.063
  []
  [permeability_caps]
    type = PorousFlowPermeabilityConst
    block = 0
    permeability = '1E-18 0 0   0 1E-18 0   0 0 1E-18'
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityConst
    block = aquifer
    permeability = '1.7E-15 0 0   0 1.7E-15 0   0 0 4.1E-16'
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
  [TimeStepper]
    type = FunctionDT
    function = 'min(3E4, max(1E4, 0.2 * t))'
  []
[]

[Outputs]
  exodus = true
[]

[Variables]
  [f_H]
    initial_condition = -2.952985071156e-06
  []
  [f_Cl]
    initial_condition = 0.04870664551708
  []
  [f_SO4]
    initial_condition = 0.0060359986852517
  []
  [f_HCO3]
    initial_condition = 5.0897287594019e-05
  []
  [f_SiO2aq]
    initial_condition = 3.0246609868421e-05
  []
  [f_Al]
    initial_condition = 3.268028901929e-08
  []
  [f_Ca]
    initial_condition = 0.00082159428184586
  []
  [f_Mg]
    initial_condition = 1.8546347062146e-05
  []
  [f_Fe]
    initial_condition = 4.3291908204093e-05
  []
  [f_K]
    initial_condition = 6.8434768308898e-05
  []
  [f_Na]
    initial_condition = 0.033298053919671
  []
  [f_Sr]
    initial_condition = 1.2771866652177e-05
  []
  [f_F]
    initial_condition = 5.5648860174073e-06
  []
  [f_BOH]
    initial_condition = 0.0003758574621917
  []
  [f_Br]
    initial_condition = 9.0315286107068e-05
  []
  [f_Ba]
    initial_condition = 1.5637460875161e-07
  []
  [f_Li]
    initial_condition = 8.3017067912701e-05
  []
  [f_NO3]
    initial_condition = 0.00010958455036169
  []
  [f_O2aq]
    initial_condition = -7.0806852373351e-05
  []
  [porepressure]
    initial_condition = 30E6
  []
  [temperature]
    initial_condition = 92
    scaling = 1E-6 # fluid enthalpy is roughly 1E6
  []
[]

[DiracKernels]
  [inject_H]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_H
    point_file = injection.bh
    variable = f_H
  []
  [inject_Cl]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Cl
    point_file = injection.bh
    variable = f_Cl
  []
  [inject_SO4]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_SO4
    point_file = injection.bh
    variable = f_SO4
  []
  [inject_HCO3]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_HCO3
    point_file = injection.bh
    variable = f_HCO3
  []
  [inject_SiO2aq]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_SiO2aq
    point_file = injection.bh
    variable = f_SiO2aq
  []
  [inject_Al]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Al
    point_file = injection.bh
    variable = f_Al
  []
  [inject_Ca]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Ca
    point_file = injection.bh
    variable = f_Ca
  []
  [inject_Mg]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Mg
    point_file = injection.bh
    variable = f_Mg
  []
  [inject_Fe]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Fe
    point_file = injection.bh
    variable = f_Fe
  []
  [inject_K]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_K
    point_file = injection.bh
    variable = f_K
  []
  [inject_Na]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Na
    point_file = injection.bh
    variable = f_Na
  []
  [inject_Sr]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Sr
    point_file = injection.bh
    variable = f_Sr
  []
  [inject_F]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_F
    point_file = injection.bh
    variable = f_F
  []
  [inject_BOH]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_BOH
    point_file = injection.bh
    variable = f_BOH
  []
  [inject_Br]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Br
    point_file = injection.bh
    variable = f_Br
  []
  [inject_Ba]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Ba
    point_file = injection.bh
    variable = f_Ba
  []
  [inject_Li]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_Li
    point_file = injection.bh
    variable = f_Li
  []
  [inject_NO3]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_NO3
    point_file = injection.bh
    variable = f_NO3
  []
  [inject_O2aq]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_O2aq
    point_file = injection.bh
    variable = f_O2aq
  []
  [inject_H2O]
    type = PorousFlowPolyLineSink
    SumQuantityUO = injected_mass
    fluxes = ${injection_rate}
    p_or_t_vals = 0.0
    multiplying_var = injection_rate_massfrac_H2O
    point_file = injection.bh
    variable = porepressure
  []

  [produce_H]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_H
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 0
    point_file = production.bh
    variable = f_H
  []
  [produce_Cl]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Cl
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 1
    point_file = production.bh
    variable = f_Cl
  []
  [produce_SO4]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_SO4
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 2
    point_file = production.bh
    variable = f_SO4
  []
  [produce_HCO3]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_HCO3
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 3
    point_file = production.bh
    variable = f_HCO3
  []
  [produce_SiO2aq]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_SiO2aq
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 4
    point_file = production.bh
    variable = f_SiO2aq
  []
  [produce_Al]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Al
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 5
    point_file = production.bh
    variable = f_Al
  []
  [produce_Ca]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Ca
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 6
    point_file = production.bh
    variable = f_Ca
  []
  [produce_Mg]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Mg
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 7
    point_file = production.bh
    variable = f_Mg
  []
  [produce_Fe]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Fe
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 8
    point_file = production.bh
    variable = f_Fe
  []
  [produce_K]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_K
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 9
    point_file = production.bh
    variable = f_K
  []
  [produce_Na]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Na
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 10
    point_file = production.bh
    variable = f_Na
  []
  [produce_Sr]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Sr
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 11
    point_file = production.bh
    variable = f_Sr
  []
  [produce_F]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_F
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 12
    point_file = production.bh
    variable = f_F
  []
  [produce_BOH]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_BOH
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 13
    point_file = production.bh
    variable = f_BOH
  []
  [produce_Br]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Br
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 14
    point_file = production.bh
    variable = f_Br
  []
  [produce_Ba]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Ba
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 15
    point_file = production.bh
    variable = f_Ba
  []
  [produce_Li]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_Li
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 16
    point_file = production.bh
    variable = f_Li
  []
  [produce_NO3]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_NO3
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 17
    point_file = production.bh
    variable = f_NO3
  []
  [produce_O2aq]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_O2aq
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 18
    point_file = production.bh
    variable = f_O2aq
  []
  [produce_H2O]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_mass_H2O
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    mass_fraction_component = 19
    point_file = production.bh
    variable = porepressure
  []
  [produce_heat]
    type = PorousFlowPolyLineSink
    SumQuantityUO = produced_heat
    fluxes = ${production_rate}
    p_or_t_vals = 0.0
    use_enthalpy = true
    point_file = production.bh
    variable = temperature
  []
[]

[UserObjects]
  [injected_mass]
    type = PorousFlowSumQuantity
  []
  [produced_mass_H]
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
  [produced_mass_SiO2aq]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Al]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Ca]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Mg]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Fe]
    type = PorousFlowSumQuantity
  []
  [produced_mass_K]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Na]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Sr]
    type = PorousFlowSumQuantity
  []
  [produced_mass_F]
    type = PorousFlowSumQuantity
  []
  [produced_mass_BOH]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Br]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Ba]
    type = PorousFlowSumQuantity
  []
  [produced_mass_Li]
    type = PorousFlowSumQuantity
  []
  [produced_mass_NO3]
    type = PorousFlowSumQuantity
  []
  [produced_mass_O2aq]
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
  [kg_H_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_H
  []
  [kg_Cl_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Cl
  []
  [kg_SO4_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_SO4
  []
  [kg_HCO3_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_HCO3
  []
  [kg_SiO2aq_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_SiO2aq
  []
  [kg_Al_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Al
  []
  [kg_Ca_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Ca
  []
  [kg_Mg_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Mg
  []
  [kg_Fe_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Fe
  []
  [kg_K_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_K
  []
  [kg_Na_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Na
  []
  [kg_Sr_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Sr
  []
  [kg_F_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_F
  []
  [kg_BOH_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_BOH
  []
  [kg_Br_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Br
  []
  [kg_Ba_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Ba
  []
  [kg_Li_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_Li
  []
  [kg_NO3_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_NO3
  []
  [kg_O2aq_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_O2aq
  []
  [kg_H2O_produced_this_timestep]
    type = PorousFlowPlotQuantity
    uo = produced_mass_H2O
  []
  [mole_rate_H_produced]
    type = FunctionValuePostprocessor
    function = moles_H
    indirect_dependencies = 'kg_H_produced_this_timestep dt'
  []
  [mole_rate_Cl_produced]
    type = FunctionValuePostprocessor
    function = moles_Cl
    indirect_dependencies = 'kg_Cl_produced_this_timestep dt'
  []
  [mole_rate_SO4_produced]
    type = FunctionValuePostprocessor
    function = moles_SO4
    indirect_dependencies = 'kg_SO4_produced_this_timestep dt'
  []
  [mole_rate_HCO3_produced]
    type = FunctionValuePostprocessor
    function = moles_HCO3
    indirect_dependencies = 'kg_HCO3_produced_this_timestep dt'
  []
  [mole_rate_SiO2aq_produced]
    type = FunctionValuePostprocessor
    function = moles_SiO2aq
    indirect_dependencies = 'kg_SiO2aq_produced_this_timestep dt'
  []
  [mole_rate_Al_produced]
    type = FunctionValuePostprocessor
    function = moles_Al
    indirect_dependencies = 'kg_Al_produced_this_timestep dt'
  []
  [mole_rate_Ca_produced]
    type = FunctionValuePostprocessor
    function = moles_Ca
    indirect_dependencies = 'kg_Ca_produced_this_timestep dt'
  []
  [mole_rate_Mg_produced]
    type = FunctionValuePostprocessor
    function = moles_Mg
    indirect_dependencies = 'kg_Mg_produced_this_timestep dt'
  []
  [mole_rate_Fe_produced]
    type = FunctionValuePostprocessor
    function = moles_Fe
    indirect_dependencies = 'kg_Fe_produced_this_timestep dt'
  []
  [mole_rate_K_produced]
    type = FunctionValuePostprocessor
    function = moles_K
    indirect_dependencies = 'kg_K_produced_this_timestep dt'
  []
  [mole_rate_Na_produced]
    type = FunctionValuePostprocessor
    function = moles_Na
    indirect_dependencies = 'kg_Na_produced_this_timestep dt'
  []
  [mole_rate_Sr_produced]
    type = FunctionValuePostprocessor
    function = moles_Sr
    indirect_dependencies = 'kg_Sr_produced_this_timestep dt'
  []
  [mole_rate_F_produced]
    type = FunctionValuePostprocessor
    function = moles_F
    indirect_dependencies = 'kg_F_produced_this_timestep dt'
  []
  [mole_rate_BOH_produced]
    type = FunctionValuePostprocessor
    function = moles_BOH
    indirect_dependencies = 'kg_BOH_produced_this_timestep dt'
  []
  [mole_rate_Br_produced]
    type = FunctionValuePostprocessor
    function = moles_Br
    indirect_dependencies = 'kg_Br_produced_this_timestep dt'
  []
  [mole_rate_Ba_produced]
    type = FunctionValuePostprocessor
    function = moles_Ba
    indirect_dependencies = 'kg_Ba_produced_this_timestep dt'
  []
  [mole_rate_Li_produced]
    type = FunctionValuePostprocessor
    function = moles_Li
    indirect_dependencies = 'kg_Li_produced_this_timestep dt'
  []
  [mole_rate_NO3_produced]
    type = FunctionValuePostprocessor
    function = moles_NO3
    indirect_dependencies = 'kg_NO3_produced_this_timestep dt'
  []
  [mole_rate_O2aq_produced]
    type = FunctionValuePostprocessor
    function = moles_O2aq
    indirect_dependencies = 'kg_O2aq_produced_this_timestep dt'
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
    type = AverageNodalVariableValue
    boundary = production_nodes
    variable = temperature
  []
[]

[Functions]
  [moles_H]
    type = ParsedFunction
    symbol_names = 'kg_H dt'
    symbol_values = 'kg_H_produced_this_timestep dt'
    expression = 'kg_H * 1000 / 1.0079 / dt'
  []
  [moles_Cl]
    type = ParsedFunction
    symbol_names = 'kg_Cl dt'
    symbol_values = 'kg_Cl_produced_this_timestep dt'
    expression = 'kg_Cl * 1000 / 35.453 / dt'
  []
  [moles_SO4]
    type = ParsedFunction
    symbol_names = 'kg_SO4 dt'
    symbol_values = 'kg_SO4_produced_this_timestep dt'
    expression = 'kg_SO4 * 1000 / 96.0576 / dt'
  []
  [moles_HCO3]
    type = ParsedFunction
    symbol_names = 'kg_HCO3 dt'
    symbol_values = 'kg_HCO3_produced_this_timestep dt'
    expression = 'kg_HCO3 * 1000 / 61.0171 / dt'
  []
  [moles_SiO2aq]
    type = ParsedFunction
    symbol_names = 'kg_SiO2aq dt'
    symbol_values = 'kg_SiO2aq_produced_this_timestep dt'
    expression = 'kg_SiO2aq * 1000 / 60.0843 / dt'
  []
  [moles_Al]
    type = ParsedFunction
    symbol_names = 'kg_Al dt'
    symbol_values = 'kg_Al_produced_this_timestep dt'
    expression = 'kg_Al * 1000 / 26.9815 / dt'
  []
  [moles_Ca]
    type = ParsedFunction
    symbol_names = 'kg_Ca dt'
    symbol_values = 'kg_Ca_produced_this_timestep dt'
    expression = 'kg_Ca * 1000 / 40.08 / dt'
  []
  [moles_Mg]
    type = ParsedFunction
    symbol_names = 'kg_Mg dt'
    symbol_values = 'kg_Mg_produced_this_timestep dt'
    expression = 'kg_Mg * 1000 / 24.305 / dt'
  []
  [moles_Fe]
    type = ParsedFunction
    symbol_names = 'kg_Fe dt'
    symbol_values = 'kg_Fe_produced_this_timestep dt'
    expression = 'kg_Fe * 1000 / 55.847 / dt'
  []
  [moles_K]
    type = ParsedFunction
    symbol_names = 'kg_K dt'
    symbol_values = 'kg_K_produced_this_timestep dt'
    expression = 'kg_K * 1000 / 39.0983 / dt'
  []
  [moles_Na]
    type = ParsedFunction
    symbol_names = 'kg_Na dt'
    symbol_values = 'kg_Na_produced_this_timestep dt'
    expression = 'kg_Na * 1000 / 22.9898 / dt'
  []
  [moles_Sr]
    type = ParsedFunction
    symbol_names = 'kg_Sr dt'
    symbol_values = 'kg_Sr_produced_this_timestep dt'
    expression = 'kg_Sr * 1000 / 87.62 / dt'
  []
  [moles_F]
    type = ParsedFunction
    symbol_names = 'kg_F dt'
    symbol_values = 'kg_F_produced_this_timestep dt'
    expression = 'kg_F * 1000 / 18.9984 / dt'
  []
  [moles_BOH]
    type = ParsedFunction
    symbol_names = 'kg_BOH dt'
    symbol_values = 'kg_BOH_produced_this_timestep dt'
    expression = 'kg_BOH * 1000 / 61.8329 / dt'
  []
  [moles_Br]
    type = ParsedFunction
    symbol_names = 'kg_Br dt'
    symbol_values = 'kg_Br_produced_this_timestep dt'
    expression = 'kg_Br * 1000 / 79.904 / dt'
  []
  [moles_Ba]
    type = ParsedFunction
    symbol_names = 'kg_Ba dt'
    symbol_values = 'kg_Ba_produced_this_timestep dt'
    expression = 'kg_Ba * 1000 / 137.33 / dt'
  []
  [moles_Li]
    type = ParsedFunction
    symbol_names = 'kg_Li dt'
    symbol_values = 'kg_Li_produced_this_timestep dt'
    expression = 'kg_Li * 1000 / 6.941 / dt'
  []
  [moles_NO3]
    type = ParsedFunction
    symbol_names = 'kg_NO3 dt'
    symbol_values = 'kg_NO3_produced_this_timestep dt'
    expression = 'kg_NO3 * 1000 / 62.0049 / dt'
  []
  [moles_O2aq]
    type = ParsedFunction
    symbol_names = 'kg_O2aq dt'
    symbol_values = 'kg_O2aq_produced_this_timestep dt'
    expression = 'kg_O2aq * 1000 / 31.9988 / dt'
  []
  [moles_H2O]
    type = ParsedFunction
    symbol_names = 'kg_H2O dt'
    symbol_values = 'kg_H2O_produced_this_timestep dt'
    expression = 'kg_H2O * 1000 / 18.01801802 / dt'
  []
[]

[AuxVariables]
  [injection_temperature]
    initial_condition = 92
  []
  [injection_rate_massfrac_H]
    initial_condition = -2.952985071156e-06
  []
  [injection_rate_massfrac_Cl]
    initial_condition = 0.04870664551708
  []
  [injection_rate_massfrac_SO4]
    initial_condition = 0.0060359986852517
  []
  [injection_rate_massfrac_HCO3]
    initial_condition = 5.0897287594019e-05
  []
  [injection_rate_massfrac_SiO2aq]
    initial_condition = 3.0246609868421e-05
  []
  [injection_rate_massfrac_Al]
    initial_condition = 3.268028901929e-08
  []
  [injection_rate_massfrac_Ca]
    initial_condition = 0.00082159428184586
  []
  [injection_rate_massfrac_Mg]
    initial_condition = 1.8546347062146e-05
  []
  [injection_rate_massfrac_Fe]
    initial_condition = 4.3291908204093e-05
  []
  [injection_rate_massfrac_K]
    initial_condition = 6.8434768308898e-05
  []
  [injection_rate_massfrac_Na]
    initial_condition = 0.033298053919671
  []
  [injection_rate_massfrac_Sr]
    initial_condition = 1.2771866652177e-05
  []
  [injection_rate_massfrac_F]
    initial_condition = 5.5648860174073e-06
  []
  [injection_rate_massfrac_BOH]
    initial_condition = 0.0003758574621917
  []
  [injection_rate_massfrac_Br]
    initial_condition = 9.0315286107068e-05
  []
  [injection_rate_massfrac_Ba]
    initial_condition = 1.5637460875161e-07
  []
  [injection_rate_massfrac_Li]
    initial_condition = 8.3017067912701e-05
  []
  [injection_rate_massfrac_NO3]
    initial_condition = 0.00010958455036169
  []
  [injection_rate_massfrac_O2aq]
    initial_condition = -7.0806852373351e-05
  []
  [injection_rate_massfrac_H2O]
    initial_condition = 0.91032275033842
  []
  [rate_H]
  []
  [rate_Cl]
  []
  [rate_SO4]
  []
  [rate_HCO3]
  []
  [rate_SiO2aq]
  []
  [rate_Al]
  []
  [rate_Ca]
  []
  [rate_Mg]
  []
  [rate_Fe]
  []
  [rate_K]
  []
  [rate_Na]
  []
  [rate_Sr]
  []
  [rate_F]
  []
  [rate_BOH]
  []
  [rate_Br]
  []
  [rate_Ba]
  []
  [rate_Li]
  []
  [rate_NO3]
  []
  [rate_O2aq]
  []
  [rate_H2O]
  []
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
    source_variable = 'rate_H rate_Cl rate_SO4 rate_HCO3 rate_SiO2aq rate_Al rate_Ca rate_Mg rate_Fe rate_K rate_Na rate_Sr rate_F rate_BOH rate_Br rate_Ba rate_Li rate_NO3 rate_O2aq rate_H2O temperature'
    variable = 'pf_rate_H pf_rate_Cl pf_rate_SO4 pf_rate_HCO3 pf_rate_SiO2aq pf_rate_Al pf_rate_Ca pf_rate_Mg pf_rate_Fe pf_rate_K pf_rate_Na pf_rate_Sr pf_rate_F pf_rate_BOH pf_rate_Br pf_rate_Ba pf_rate_Li pf_rate_NO3 pf_rate_O2aq pf_rate_H2O temperature'
    to_multi_app = react
  []
  [massfrac_from_geochem]
    type = MultiAppCopyTransfer
    source_variable = 'massfrac_H massfrac_Cl massfrac_SO4 massfrac_HCO3 massfrac_SiO2aq massfrac_Al massfrac_Ca massfrac_Mg massfrac_Fe massfrac_K massfrac_Na massfrac_Sr massfrac_F massfrac_BOH massfrac_Br massfrac_Ba massfrac_Li massfrac_NO3 massfrac_O2aq '
    variable = 'f_H f_Cl f_SO4 f_HCO3 f_SiO2aq f_Al f_Ca f_Mg f_Fe f_K f_Na f_Sr f_F f_BOH f_Br f_Ba f_Li f_NO3 f_O2aq '
    from_multi_app = react
  []
[]

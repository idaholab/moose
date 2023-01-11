#########################################
#                                       #
# File written by create_input_files.py #
#                                       #
#########################################
# Simulates geochemistry in the aquifer.  This input file may be run in standalone fashion but it does not do anything of interest.  To simulate something interesting, run the porous_flow.i simulation which couples to this input file using MultiApps.
# This file receives pf_rate_H pf_rate_Cl pf_rate_SO4 pf_rate_HCO3 pf_rate_SiO2aq pf_rate_Al pf_rate_Ca pf_rate_Mg pf_rate_Fe pf_rate_K pf_rate_Na pf_rate_Sr pf_rate_F pf_rate_BOH pf_rate_Br pf_rate_Ba pf_rate_Li pf_rate_NO3 pf_rate_O2aq pf_rate_H2O and temperature as AuxVariables from porous_flow.i
# The pf_rate quantities are kg/s changes of fluid-component mass at each node, but the geochemistry module expects rates-of-changes of moles at every node.  Secondly, since this input file considers just 1 litre of aqueous solution at every node, the nodal_void_volume is used to convert pf_rate_* into rate_*_per_1l, which is measured in mol/s/1_litre_of_aqueous_solution.
# This file sends massfrac_H massfrac_Cl massfrac_SO4 massfrac_HCO3 massfrac_SiO2aq massfrac_Al massfrac_Ca massfrac_Mg massfrac_Fe massfrac_K massfrac_Na massfrac_Sr massfrac_F massfrac_BOH massfrac_Br massfrac_Ba massfrac_Li massfrac_NO3 massfrac_O2aq to porous_flow.i.  These are computed from the corresponding transported_* quantities.

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = '../../../../geochemistry/database/moose_geochemdb.json'
    basis_species = 'H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)'
    equilibrium_minerals = 'Siderite Pyrrhotite Dolomite Illite Anhydrite Calcite Quartz K-feldspar Kaolinite Barite Celestite Fluorite Albite Chalcedony Goethite'
  []
  [nodal_void_volume_uo]
    type = NodalVoidVolume
    porosity = porosity
    execute_on = 'initial timestep_end' # initial means this is evaluated properly for the first timestep
  []
[]

[SpatialReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'Cl-'
  swap_out_of_basis = 'NO3- H+         Fe++       Ba++   SiO2(aq) Mg++     O2(aq)   Al+++   K+     Ca++      HCO3-'
  swap_into_basis = '  NH3  Pyrrhotite K-feldspar Barite Quartz   Dolomite Siderite Calcite Illite Anhydrite Kaolinite'
# ASSUME that 1 litre of solution contains:
  constraint_species = 'H2O        Quartz     Calcite   K-feldspar Siderite  Dolomite  Anhydrite Pyrrhotite Illite    Kaolinite  Barite       Na+       Cl-       SO4--       Li+         B(OH)3      Br-         F-         Sr++        NH3'
  constraint_value = '  0.99778351 322.177447 12.111108 6.8269499  6.2844304 2.8670301 1.1912027 0.51474767 0.3732507 0.20903322 0.0001865889 1.5876606 1.5059455 0.046792579 0.013110503 0.006663119 0.001238987 0.00032108 0.000159781 0.001937302'
  constraint_meaning = 'kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition'
  constraint_unit = "kg moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles"
  prevent_precipitation = 'Fluorite Albite Goethite'
  initial_temperature = 92
  temperature = temperature
  source_species_names = 'H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq) H2O'
  source_species_rates = ' rate_H_per_1l rate_Cl_per_1l rate_SO4_per_1l rate_HCO3_per_1l rate_SiO2aq_per_1l rate_Al_per_1l rate_Ca_per_1l rate_Mg_per_1l rate_Fe_per_1l rate_K_per_1l rate_Na_per_1l rate_Sr_per_1l rate_F_per_1l rate_BOH_per_1l rate_Br_per_1l rate_Ba_per_1l rate_Li_per_1l rate_NO3_per_1l rate_O2aq_per_1l rate_H2O_per_1l'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  execute_console_output_on = '' # only CSV and exodus output for this simulation
  add_aux_molal = false # save some memory and reduce variables in output exodus
  add_aux_mg_per_kg = false # save some memory and reduce variables in output exodus
  add_aux_free_mg = false # save some memory and reduce variables in output exodus
  add_aux_activity = false # save some memory and reduce variables in output exodus
  add_aux_bulk_moles = false # save some memory and reduce variables in output exodus
  adaptive_timestepping = true
[]

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
  point = '-25 0 0'
  reactor = reactor
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
[AuxVariables]
  [temperature]
    initial_condition = 92.0
  []
  [porosity]
    initial_condition = 0.1
  []
  [nodal_void_volume]
  []
  [free_cm3_Kfeldspar] # necessary because of the minus sign in K-feldspar which does not parse correctly in the porosity AuxKernel
  []
  [pf_rate_H] # change in H mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Cl] # change in Cl mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_SO4] # change in SO4 mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_HCO3] # change in HCO3 mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_SiO2aq] # change in SiO2aq mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Al] # change in Al mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Ca] # change in Ca mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Mg] # change in Mg mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Fe] # change in Fe mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_K] # change in K mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Na] # change in Na mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Sr] # change in Sr mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_F] # change in F mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_BOH] # change in BOH mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Br] # change in Br mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Ba] # change in Ba mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Li] # change in Li mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_NO3] # change in NO3 mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_O2aq] # change in O2aq mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_H2O] # change in H2O mass (kg/s) at each node provided by the porous-flow simulation
  []
  [rate_H_per_1l]
  []
  [rate_Cl_per_1l]
  []
  [rate_SO4_per_1l]
  []
  [rate_HCO3_per_1l]
  []
  [rate_SiO2aq_per_1l]
  []
  [rate_Al_per_1l]
  []
  [rate_Ca_per_1l]
  []
  [rate_Mg_per_1l]
  []
  [rate_Fe_per_1l]
  []
  [rate_K_per_1l]
  []
  [rate_Na_per_1l]
  []
  [rate_Sr_per_1l]
  []
  [rate_F_per_1l]
  []
  [rate_BOH_per_1l]
  []
  [rate_Br_per_1l]
  []
  [rate_Ba_per_1l]
  []
  [rate_Li_per_1l]
  []
  [rate_NO3_per_1l]
  []
  [rate_O2aq_per_1l]
  []
  [rate_H2O_per_1l]
  []
  [transported_H]
  []
  [transported_Cl]
  []
  [transported_SO4]
  []
  [transported_HCO3]
  []
  [transported_SiO2aq]
  []
  [transported_Al]
  []
  [transported_Ca]
  []
  [transported_Mg]
  []
  [transported_Fe]
  []
  [transported_K]
  []
  [transported_Na]
  []
  [transported_Sr]
  []
  [transported_F]
  []
  [transported_BOH]
  []
  [transported_Br]
  []
  [transported_Ba]
  []
  [transported_Li]
  []
  [transported_NO3]
  []
  [transported_O2aq]
  []
  [transported_H2O]
  []
  [transported_mass]
  []
  [massfrac_H]
  []
  [massfrac_Cl]
  []
  [massfrac_SO4]
  []
  [massfrac_HCO3]
  []
  [massfrac_SiO2aq]
  []
  [massfrac_Al]
  []
  [massfrac_Ca]
  []
  [massfrac_Mg]
  []
  [massfrac_Fe]
  []
  [massfrac_K]
  []
  [massfrac_Na]
  []
  [massfrac_Sr]
  []
  [massfrac_F]
  []
  [massfrac_BOH]
  []
  [massfrac_Br]
  []
  [massfrac_Ba]
  []
  [massfrac_Li]
  []
  [massfrac_NO3]
  []
  [massfrac_O2aq]
  []
  [massfrac_H2O]
  []
[]

[AuxKernels]
  [free_cm3_Kfeldspar]
    type = GeochemistryQuantityAux
    variable = free_cm3_Kfeldspar
    species = 'K-feldspar'
    quantity = free_cm3
    execute_on = 'timestep_end'
  []
  [porosity_auxk]
    type = ParsedAux
    coupled_variables = 'free_cm3_Siderite free_cm3_Pyrrhotite free_cm3_Dolomite free_cm3_Illite free_cm3_Anhydrite free_cm3_Calcite free_cm3_Quartz free_cm3_Kfeldspar free_cm3_Kaolinite free_cm3_Barite free_cm3_Celestite free_cm3_Fluorite free_cm3_Albite free_cm3_Chalcedony free_cm3_Goethite'
    expression = '1000.0 / (1000.0 + free_cm3_Siderite + free_cm3_Pyrrhotite + free_cm3_Dolomite + free_cm3_Illite + free_cm3_Anhydrite + free_cm3_Calcite + free_cm3_Quartz + free_cm3_Kfeldspar + free_cm3_Kaolinite + free_cm3_Barite + free_cm3_Celestite + free_cm3_Fluorite + free_cm3_Albite + free_cm3_Chalcedony + free_cm3_Goethite)'
    variable = porosity
    execute_on = 'timestep_end'
  []
  [nodal_void_volume_auxk]
    type = NodalVoidVolumeAux
    variable = nodal_void_volume
    nodal_void_volume_uo = nodal_void_volume_uo
    execute_on = 'initial timestep_end' # initial to ensure it is properly evaluated for the first timestep
  []
  [rate_H_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_H nodal_void_volume'
    variable = rate_H_per_1l
    expression = 'pf_rate_H / 1.0079 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Cl_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Cl nodal_void_volume'
    variable = rate_Cl_per_1l
    expression = 'pf_rate_Cl / 35.453 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_SO4_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_SO4 nodal_void_volume'
    variable = rate_SO4_per_1l
    expression = 'pf_rate_SO4 / 96.0576 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_HCO3_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_HCO3 nodal_void_volume'
    variable = rate_HCO3_per_1l
    expression = 'pf_rate_HCO3 / 61.0171 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_SiO2aq_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_SiO2aq nodal_void_volume'
    variable = rate_SiO2aq_per_1l
    expression = 'pf_rate_SiO2aq / 60.0843 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Al_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Al nodal_void_volume'
    variable = rate_Al_per_1l
    expression = 'pf_rate_Al / 26.9815 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Ca_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Ca nodal_void_volume'
    variable = rate_Ca_per_1l
    expression = 'pf_rate_Ca / 40.08 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Mg_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Mg nodal_void_volume'
    variable = rate_Mg_per_1l
    expression = 'pf_rate_Mg / 24.305 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Fe_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Fe nodal_void_volume'
    variable = rate_Fe_per_1l
    expression = 'pf_rate_Fe / 55.847 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_K_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_K nodal_void_volume'
    variable = rate_K_per_1l
    expression = 'pf_rate_K / 39.0983 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Na_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Na nodal_void_volume'
    variable = rate_Na_per_1l
    expression = 'pf_rate_Na / 22.9898 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Sr_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Sr nodal_void_volume'
    variable = rate_Sr_per_1l
    expression = 'pf_rate_Sr / 87.62 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_F_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_F nodal_void_volume'
    variable = rate_F_per_1l
    expression = 'pf_rate_F / 18.9984 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_BOH_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_BOH nodal_void_volume'
    variable = rate_BOH_per_1l
    expression = 'pf_rate_BOH / 61.8329 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Br_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Br nodal_void_volume'
    variable = rate_Br_per_1l
    expression = 'pf_rate_Br / 79.904 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Ba_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Ba nodal_void_volume'
    variable = rate_Ba_per_1l
    expression = 'pf_rate_Ba / 137.33 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_Li_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Li nodal_void_volume'
    variable = rate_Li_per_1l
    expression = 'pf_rate_Li / 6.941 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_NO3_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_NO3 nodal_void_volume'
    variable = rate_NO3_per_1l
    expression = 'pf_rate_NO3 / 62.0049 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_O2aq_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_O2aq nodal_void_volume'
    variable = rate_O2aq_per_1l
    expression = 'pf_rate_O2aq / 31.9988 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [rate_H2O_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_H2O nodal_void_volume'
    variable = rate_H2O_per_1l
    expression = 'pf_rate_H2O / 18.01801802 / nodal_void_volume'
    execute_on = 'timestep_end'
  []
  [transported_H_auxk]
    type = GeochemistryQuantityAux
    variable = transported_H
    species = 'H+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Cl_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Cl
    species = 'Cl-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_SO4_auxk]
    type = GeochemistryQuantityAux
    variable = transported_SO4
    species = 'SO4--'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_HCO3_auxk]
    type = GeochemistryQuantityAux
    variable = transported_HCO3
    species = 'HCO3-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_SiO2aq_auxk]
    type = GeochemistryQuantityAux
    variable = transported_SiO2aq
    species = 'SiO2(aq)'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Al_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Al
    species = 'Al+++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Ca_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Ca
    species = 'Ca++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Mg_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Mg
    species = 'Mg++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Fe_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Fe
    species = 'Fe++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_K_auxk]
    type = GeochemistryQuantityAux
    variable = transported_K
    species = 'K+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Na_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Na
    species = 'Na+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Sr_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Sr
    species = 'Sr++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_F_auxk]
    type = GeochemistryQuantityAux
    variable = transported_F
    species = 'F-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_BOH_auxk]
    type = GeochemistryQuantityAux
    variable = transported_BOH
    species = 'B(OH)3'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Br_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Br
    species = 'Br-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Ba_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Ba
    species = 'Ba++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Li_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Li
    species = 'Li+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_NO3_auxk]
    type = GeochemistryQuantityAux
    variable = transported_NO3
    species = 'NO3-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_O2aq_auxk]
    type = GeochemistryQuantityAux
    variable = transported_O2aq
    species = 'O2(aq)'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_H2O_auxk]
    type = GeochemistryQuantityAux
    variable = transported_H2O
    species = 'H2O'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_mass_auxk]
    type = ParsedAux
    coupled_variables = ' transported_H transported_Cl transported_SO4 transported_HCO3 transported_SiO2aq transported_Al transported_Ca transported_Mg transported_Fe transported_K transported_Na transported_Sr transported_F transported_BOH transported_Br transported_Ba transported_Li transported_NO3 transported_O2aq transported_H2O'
    variable = transported_mass
    expression = 'transported_H * 1.0079 + transported_Cl * 35.453 + transported_SO4 * 96.0576 + transported_HCO3 * 61.0171 + transported_SiO2aq * 60.0843 + transported_Al * 26.9815 + transported_Ca * 40.08 + transported_Mg * 24.305 + transported_Fe * 55.847 + transported_K * 39.0983 + transported_Na * 22.9898 + transported_Sr * 87.62 + transported_F * 18.9984 + transported_BOH * 61.8329 + transported_Br * 79.904 + transported_Ba * 137.33 + transported_Li * 6.941 + transported_NO3 * 62.0049 + transported_O2aq * 31.9988 + transported_H2O * 18.01801802'
    execute_on = 'timestep_end'
  []
  [massfrac_H_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H transported_mass'
    variable = massfrac_H
    expression = 'transported_H * 1.0079 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Cl_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Cl transported_mass'
    variable = massfrac_Cl
    expression = 'transported_Cl * 35.453 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_SO4_auxk]
    type = ParsedAux
    coupled_variables = 'transported_SO4 transported_mass'
    variable = massfrac_SO4
    expression = 'transported_SO4 * 96.0576 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_HCO3_auxk]
    type = ParsedAux
    coupled_variables = 'transported_HCO3 transported_mass'
    variable = massfrac_HCO3
    expression = 'transported_HCO3 * 61.0171 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_SiO2aq_auxk]
    type = ParsedAux
    coupled_variables = 'transported_SiO2aq transported_mass'
    variable = massfrac_SiO2aq
    expression = 'transported_SiO2aq * 60.0843 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Al_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Al transported_mass'
    variable = massfrac_Al
    expression = 'transported_Al * 26.9815 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Ca_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Ca transported_mass'
    variable = massfrac_Ca
    expression = 'transported_Ca * 40.08 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Mg_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Mg transported_mass'
    variable = massfrac_Mg
    expression = 'transported_Mg * 24.305 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Fe_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Fe transported_mass'
    variable = massfrac_Fe
    expression = 'transported_Fe * 55.847 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_K_auxk]
    type = ParsedAux
    coupled_variables = 'transported_K transported_mass'
    variable = massfrac_K
    expression = 'transported_K * 39.0983 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Na_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Na transported_mass'
    variable = massfrac_Na
    expression = 'transported_Na * 22.9898 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Sr_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Sr transported_mass'
    variable = massfrac_Sr
    expression = 'transported_Sr * 87.62 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_F_auxk]
    type = ParsedAux
    coupled_variables = 'transported_F transported_mass'
    variable = massfrac_F
    expression = 'transported_F * 18.9984 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_BOH_auxk]
    type = ParsedAux
    coupled_variables = 'transported_BOH transported_mass'
    variable = massfrac_BOH
    expression = 'transported_BOH * 61.8329 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Br_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Br transported_mass'
    variable = massfrac_Br
    expression = 'transported_Br * 79.904 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Ba_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Ba transported_mass'
    variable = massfrac_Ba
    expression = 'transported_Ba * 137.33 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Li_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Li transported_mass'
    variable = massfrac_Li
    expression = 'transported_Li * 6.941 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_NO3_auxk]
    type = ParsedAux
    coupled_variables = 'transported_NO3 transported_mass'
    variable = massfrac_NO3
    expression = 'transported_NO3 * 62.0049 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_O2aq_auxk]
    type = ParsedAux
    coupled_variables = 'transported_O2aq transported_mass'
    variable = massfrac_O2aq
    expression = 'transported_O2aq * 31.9988 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_H2O_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H2O transported_mass'
    variable = massfrac_H2O
    expression = 'transported_H2O * 18.01801802 / transported_mass'
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [memory]
    type = MemoryUsage
    outputs = 'console'
  []
  [porosity]
    type = PointValue
    variable = porosity
  []
  [solution_temperature]
    type = PointValue
    variable = solution_temperature
  []
  [massfrac_H]
    type = PointValue
    variable = massfrac_H
  []
  [massfrac_Cl]
    type = PointValue
    variable = massfrac_Cl
  []
  [massfrac_SO4]
    type = PointValue
    variable = massfrac_SO4
  []
  [massfrac_HCO3]
    type = PointValue
    variable = massfrac_HCO3
  []
  [massfrac_SiO2aq]
    type = PointValue
    variable = massfrac_SiO2aq
  []
  [massfrac_Al]
    type = PointValue
    variable = massfrac_Al
  []
  [massfrac_Ca]
    type = PointValue
    variable = massfrac_Ca
  []
  [massfrac_Mg]
    type = PointValue
    variable = massfrac_Mg
  []
  [massfrac_Fe]
    type = PointValue
    variable = massfrac_Fe
  []
  [massfrac_K]
    type = PointValue
    variable = massfrac_K
  []
  [massfrac_Na]
    type = PointValue
    variable = massfrac_Na
  []
  [massfrac_Sr]
    type = PointValue
    variable = massfrac_Sr
  []
  [massfrac_F]
    type = PointValue
    variable = massfrac_F
  []
  [massfrac_BOH]
    type = PointValue
    variable = massfrac_BOH
  []
  [massfrac_Br]
    type = PointValue
    variable = massfrac_Br
  []
  [massfrac_Ba]
    type = PointValue
    variable = massfrac_Ba
  []
  [massfrac_Li]
    type = PointValue
    variable = massfrac_Li
  []
  [massfrac_NO3]
    type = PointValue
    variable = massfrac_NO3
  []
  [massfrac_O2aq]
    type = PointValue
    variable = massfrac_O2aq
  []
  [massfrac_H2O]
    type = PointValue
    variable = massfrac_H2O
  []
  [free_cm3_Siderite]
    type = PointValue
    variable = free_cm3_Siderite
  []
  [free_cm3_Pyrrhotite]
    type = PointValue
    variable = free_cm3_Pyrrhotite
  []
  [free_cm3_Dolomite]
    type = PointValue
    variable = free_cm3_Dolomite
  []
  [free_cm3_Illite]
    type = PointValue
    variable = free_cm3_Illite
  []
  [free_cm3_Anhydrite]
    type = PointValue
    variable = free_cm3_Anhydrite
  []
  [free_cm3_Calcite]
    type = PointValue
    variable = free_cm3_Calcite
  []
  [free_cm3_Quartz]
    type = PointValue
    variable = free_cm3_Quartz
  []
  [free_cm3_K-feldspar]
    type = PointValue
    variable = free_cm3_K-feldspar
  []
  [free_cm3_Kaolinite]
    type = PointValue
    variable = free_cm3_Kaolinite
  []
  [free_cm3_Barite]
    type = PointValue
    variable = free_cm3_Barite
  []
  [free_cm3_Celestite]
    type = PointValue
    variable = free_cm3_Celestite
  []
  [free_cm3_Fluorite]
    type = PointValue
    variable = free_cm3_Fluorite
  []
  [free_cm3_Albite]
    type = PointValue
    variable = free_cm3_Albite
  []
  [free_cm3_Chalcedony]
    type = PointValue
    variable = free_cm3_Chalcedony
  []
  [free_cm3_Goethite]
    type = PointValue
    variable = free_cm3_Goethite
  []
[]
[Outputs]
  exodus = true
  csv = true
[]

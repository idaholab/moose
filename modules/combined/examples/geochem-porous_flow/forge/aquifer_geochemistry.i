# Simulates geochemistry in the aquifer.  This input file may be run in standalone fashion, which will study the natural kinetically-controlled mineral changes in the same way as natural_reservoir.i.  To simulate the FORGE injection scenario, run the porous_flow.i simulation which couples to this input file using MultiApps.
# This file receives pf_rate_H pf_rate_Na pf_rate_K pf_rate_Ca pf_rate_Mg pf_rate_SiO2 pf_rate_Al pf_rate_Cl pf_rate_SO4 pf_rate_HCO3 pf_rate_H2O and temperature as AuxVariables from porous_flow.i
# The pf_rate quantities are kg/s changes of fluid-component mass at each node, but the geochemistry module expects rates-of-changes of moles at every node.  Secondly, since this input file considers just 1 litre of aqueous solution at every node, the nodal_void_volume is used to convert pf_rate_* into rate_*_per_1l, which is measured in mol/s/1_litre_of_aqueous_solution.
# This file sends massfrac_H massfrac_Na massfrac_K massfrac_Ca massfrac_Mg massfrac_SiO2 massfrac_Al massfrac_Cl massfrac_SO4 massfrac_HCO3 to porous_flow.i.  These are computed from the corresponding transported_* quantities.
# The results depend on the kinetic rates used and these are recognised to be poorly constrained by experiment
[UserObjects]
  [rate_Albite]
    type = GeochemistryKineticRate
    kinetic_species_name = Albite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 69.8E3
    one_over_T0 = 0.003354
  []
  [rate_Anhydrite]
    type = GeochemistryKineticRate
    kinetic_species_name = Anhydrite
    intrinsic_rate_constant = 1.0E-7
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 14.3E3
    one_over_T0 = 0.003354
  []
  [rate_Anorthite]
    type = GeochemistryKineticRate
    kinetic_species_name = Anorthite
    intrinsic_rate_constant = 1.0E-13
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 17.8E3
    one_over_T0 = 0.003354
  []
  [rate_Calcite]
    type = GeochemistryKineticRate
    kinetic_species_name = Calcite
    intrinsic_rate_constant = 1.0E-10
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 23.5E3
    one_over_T0 = 0.003354
  []
  [rate_Chalcedony]
    type = GeochemistryKineticRate
    kinetic_species_name = Chalcedony
    intrinsic_rate_constant = 1.0E-18
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 90.1E3
    one_over_T0 = 0.003354
  []
  [rate_Clinochl-7A]
    type = GeochemistryKineticRate
    kinetic_species_name = Clinochl-7A
    intrinsic_rate_constant = 1.0E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 88.0E3
    one_over_T0 = 0.003354
  []
  [rate_Illite]
    type = GeochemistryKineticRate
    kinetic_species_name = Illite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 29E3
    one_over_T0 = 0.003354
  []
  [rate_K-feldspar]
    type = GeochemistryKineticRate
    kinetic_species_name = K-feldspar
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 38E3
    one_over_T0 = 0.003354
  []
  [rate_Kaolinite]
    type = GeochemistryKineticRate
    kinetic_species_name = Kaolinite
    intrinsic_rate_constant = 1E-18
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 22.2E3
    one_over_T0 = 0.003354
  []
  [rate_Quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 1E-18
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 90.1E3
    one_over_T0 = 0.003354
  []
  [rate_Paragonite]
    type = GeochemistryKineticRate
    kinetic_species_name = Paragonite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 22E3
    one_over_T0 = 0.003354
  []
  [rate_Phlogopite]
    type = GeochemistryKineticRate
    kinetic_species_name = Phlogopite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 22E3
    one_over_T0 = 0.003354
  []
  [rate_Laumontite]
    type = GeochemistryKineticRate
    kinetic_species_name = Laumontite
    intrinsic_rate_constant = 1.0E-15
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 17.8E3
    one_over_T0 = 0.003354
  []
  [rate_Zoisite]
    type = GeochemistryKineticRate
    kinetic_species_name = Zoisite
    intrinsic_rate_constant = 1E-16
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 66.1E3
    one_over_T0 = 0.003354
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = '../../../../geochemistry/database/moose_geochemdb.json'
    basis_species = 'H2O H+ Na+ K+ Ca++ Mg++ SiO2(aq) Al+++ Cl- SO4-- HCO3-'
    remove_all_extrapolated_secondary_species = true
    kinetic_minerals = 'Albite Anhydrite Anorthite Calcite Chalcedony Clinochl-7A Illite K-feldspar Kaolinite Quartz Paragonite Phlogopite Zoisite Laumontite'
    kinetic_rate_descriptions = 'rate_Albite rate_Anhydrite rate_Anorthite rate_Calcite rate_Chalcedony rate_Clinochl-7A rate_Illite rate_K-feldspar rate_Kaolinite rate_Quartz rate_Paragonite rate_Phlogopite rate_Zoisite rate_Laumontite'
  []
  [nodal_void_volume_uo]
    type = NodalVoidVolume
    porosity = porosity
    execute_on = 'initial timestep_end' # "initial" means this is evaluated properly for the first timestep
  []
[]

[SpatialReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'Cl-'
  constraint_species = 'H2O              H+                  Na+              K+                 Ca++              Mg++                SiO2(aq)           Al+++               Cl-                SO4--               HCO3-'
# Following numbers are from water_60_to_220degC_out.csv
  constraint_value = '  1.0006383866109  9.5165072498215e-07 0.100020379171   0.0059389061065    0.011570884507621 4.6626763057447e-06 0.0045110404925255 5.8096968688789e-17 0.13500708594394   6.6523540147676e-05 7.7361407898089e-05'
  constraint_meaning = 'kg_solvent_water free_concentration       free_concentration    free_concentration      free_concentration     free_concentration       free_concentration      free_concentration       bulk_composition free_concentration       free_concentration'
  constraint_unit = '   kg               molal               molal            molal              molal             molal               molal              molal               moles              molal               molal'
  initial_temperature = 220
  temperature = temperature
  kinetic_species_name = '         Albite             Anorthite          K-feldspar         Quartz             Phlogopite         Paragonite         Calcite            Anhydrite          Chalcedony         Illite             Kaolinite          Clinochl-7A        Zoisite            Laumontite'
  kinetic_species_initial_value = '4.324073236492E+02 4.631370307325E+01 2.685015418378E+02 7.720095013956E+02 1.235192062541E+01 7.545461404965E-01 4.234651808835E-04 4.000485907930E-04 4.407616361072E+00 1.342524904876E+01 1.004823151125E+00 4.728132387707E-01 7.326007326007E-01 4.818116116598E-01'
  kinetic_species_unit = '         moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles'
  evaluate_kinetic_rates_always = true # otherwise will easily "run out" of dissolving species
  source_species_names = 'H2O H+ Na+ K+ Ca++ Mg++ SiO2(aq) Al+++ Cl- SO4-- HCO3-'
  source_species_rates = 'rate_H2O_per_1l rate_H_per_1l rate_Na_per_1l rate_K_per_1l rate_Ca_per_1l rate_Mg_per_1l rate_SiO2_per_1l rate_Al_per_1l rate_Cl_per_1l rate_SO4_per_1l rate_HCO3_per_1l'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  execute_console_output_on = ''
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
    dim = 2
    nx = 15
    ny = 10
    xmin = -100
    xmax = 200
    ymin = -100
    ymax = 100
  []
  [injection_node]
    input = gen
    type = ExtraNodesetGenerator
    new_boundary = injection_node
    coord = '0 0 0'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = 'max(1E6, 0.3 * t)'
  []
  end_time = 4E12
[]

[AuxVariables]
  [temperature]
    initial_condition = 220.0
  []
  [porosity]
    initial_condition = 0.01
  []
  [nodal_void_volume]
  []
  [free_cm3_Kfeldspar] # necessary because of the minus sign in K-feldspar which does not parse correctly in the porosity AuxKernel
  []
  [free_cm3_Clinochl7A] # necessary because of the minus sign in Clinochl-7A which does not parse correctly in the porosity AuxKernel
  []
  [pf_rate_H] # change in H mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Na]
  []
  [pf_rate_K]
  []
  [pf_rate_Ca]
  []
  [pf_rate_Mg]
  []
  [pf_rate_SiO2]
  []
  [pf_rate_Al]
  []
  [pf_rate_Cl]
  []
  [pf_rate_SO4]
  []
  [pf_rate_HCO3]
  []
  [pf_rate_H2O] # change in H2O mass (kg/s) at each node provided by the porous-flow simulation
  []
  [rate_H_per_1l]
  []
  [rate_Na_per_1l]
  []
  [rate_K_per_1l]
  []
  [rate_Ca_per_1l]
  []
  [rate_Mg_per_1l]
  []
  [rate_SiO2_per_1l]
  []
  [rate_Al_per_1l]
  []
  [rate_Cl_per_1l]
  []
  [rate_SO4_per_1l]
  []
  [rate_HCO3_per_1l]
  []
  [rate_H2O_per_1l]
  []
  [transported_H]
  []
  [transported_Na]
  []
  [transported_K]
  []
  [transported_Ca]
  []
  [transported_Mg]
  []
  [transported_SiO2]
  []
  [transported_Al]
  []
  [transported_Cl]
  []
  [transported_SO4]
  []
  [transported_HCO3]
  []
  [transported_H2O]
  []
  [transported_mass]
  []
  [massfrac_H]
  []
  [massfrac_Na]
  []
  [massfrac_K]
  []
  [massfrac_Ca]
  []
  [massfrac_Mg]
  []
  [massfrac_SiO2]
  []
  [massfrac_Al]
  []
  [massfrac_Cl]
  []
  [massfrac_SO4]
  []
  [massfrac_HCO3]
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
    execute_on = 'timestep_begin timestep_end'
  []
  [free_cm3_Clinochl7A]
    type = GeochemistryQuantityAux
    variable = free_cm3_Clinochl7A
    species = 'Clinochl-7A'
    quantity = free_cm3
    execute_on = 'timestep_begin timestep_end'
  []
  [porosity_auxk]
    type = ParsedAux
    coupled_variables = 'free_cm3_Albite free_cm3_Anhydrite free_cm3_Anorthite free_cm3_Calcite free_cm3_Chalcedony free_cm3_Clinochl7A free_cm3_Illite free_cm3_Kfeldspar free_cm3_Kaolinite free_cm3_Quartz free_cm3_Paragonite free_cm3_Phlogopite free_cm3_Zoisite free_cm3_Laumontite'
    expression = '1000.0 / (1000.0 + free_cm3_Albite + free_cm3_Anhydrite + free_cm3_Anorthite + free_cm3_Calcite + free_cm3_Chalcedony + free_cm3_Clinochl7A + free_cm3_Illite + free_cm3_Kfeldspar + free_cm3_Kaolinite + free_cm3_Quartz + free_cm3_Paragonite + free_cm3_Phlogopite + free_cm3_Zoisite + free_cm3_Laumontite)'
    variable = porosity
    execute_on = 'timestep_end'
  []
  [nodal_void_volume_auxk]
    type = NodalVoidVolumeAux
    variable = nodal_void_volume
    nodal_void_volume_uo = nodal_void_volume_uo
    execute_on = 'initial timestep_end' # "initial" to ensure it is properly evaluated for the first timestep
  []
  [rate_H_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_H nodal_void_volume'
    variable = rate_H_per_1l
    expression = 'pf_rate_H / 1.0079 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Na_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Na nodal_void_volume'
    variable = rate_Na_per_1l
    expression = 'pf_rate_Na / 22.9898 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_K_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_K nodal_void_volume'
    variable = rate_K_per_1l
    expression = 'pf_rate_K / 39.0983 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Ca_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Ca nodal_void_volume'
    variable = rate_Ca_per_1l
    expression = 'pf_rate_Ca / 40.08 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Mg_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Mg nodal_void_volume'
    variable = rate_Mg_per_1l
    expression = 'pf_rate_Mg / 24.305 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_SiO2_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_SiO2 nodal_void_volume'
    variable = rate_SiO2_per_1l
    expression = 'pf_rate_SiO2 / 60.0843 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Al_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Al nodal_void_volume'
    variable = rate_Al_per_1l
    expression = 'pf_rate_Al / 26.9815 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Cl_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Cl nodal_void_volume'
    variable = rate_Cl_per_1l
    expression = 'pf_rate_Cl / 35.453 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_SO4_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_SO4 nodal_void_volume'
    variable = rate_SO4_per_1l
    expression = 'pf_rate_SO4 / 96.0576 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_HCO3_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_HCO3 nodal_void_volume'
    variable = rate_HCO3_per_1l
    expression = 'pf_rate_HCO3 / 61.0171 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_H2O_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_H2O nodal_void_volume'
    variable = rate_H2O_per_1l
    expression = 'pf_rate_H2O / 18.01801802 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [transported_H_auxk]
    type = GeochemistryQuantityAux
    variable = transported_H
    species = 'H+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_Na_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Na
    species = 'Na+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_K_auxk]
    type = GeochemistryQuantityAux
    variable = transported_K
    species = 'K+'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_Ca_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Ca
    species = 'Ca++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_Mg_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Mg
    species = 'Mg++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_SiO2_auxk]
    type = GeochemistryQuantityAux
    variable = transported_SiO2
    species = 'SiO2(aq)'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_Al_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Al
    species = 'Al+++'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_Cl_auxk]
    type = GeochemistryQuantityAux
    variable = transported_Cl
    species = 'Cl-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_SO4_auxk]
    type = GeochemistryQuantityAux
    variable = transported_SO4
    species = 'SO4--'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_HCO3_auxk]
    type = GeochemistryQuantityAux
    variable = transported_HCO3
    species = 'HCO3-'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_H2O_auxk]
    type = GeochemistryQuantityAux
    variable = transported_H2O
    species = 'H2O'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_begin'
  []
  [transported_mass_auxk]
    type = ParsedAux
    coupled_variables = ' transported_H transported_Na transported_K transported_Ca transported_Mg transported_SiO2 transported_Al transported_Cl transported_SO4 transported_HCO3 transported_H2O'
    variable = transported_mass
    expression = 'transported_H * 1.0079 + transported_Cl * 35.453 + transported_SO4 * 96.0576 + transported_HCO3 * 61.0171 + transported_SiO2 * 60.0843 + transported_Al * 26.9815 + transported_Ca * 40.08 + transported_Mg * 24.305 + transported_K * 39.0983 + transported_Na * 22.9898 + transported_H2O * 18.01801802'
    execute_on = 'timestep_end'
  []
  [massfrac_H_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H transported_mass'
    variable = massfrac_H
    expression = 'transported_H * 1.0079 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Na_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Na transported_mass'
    variable = massfrac_Na
    expression = 'transported_Na * 22.9898 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_K_auxk]
    type = ParsedAux
    coupled_variables = 'transported_K transported_mass'
    variable = massfrac_K
    expression = 'transported_K * 39.0983 / transported_mass'
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
  [massfrac_SiO2_auxk]
    type = ParsedAux
    coupled_variables = 'transported_SiO2 transported_mass'
    variable = massfrac_SiO2
    expression = 'transported_SiO2 * 60.0843 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Al_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Al transported_mass'
    variable = massfrac_Al
    expression = 'transported_Al * 26.9815 / transported_mass'
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
  [massfrac_H2O_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H2O transported_mass'
    variable = massfrac_H2O
    expression = 'transported_H2O * 18.01801802 / transported_mass'
    execute_on = 'timestep_end'
  []
[]

[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

[Postprocessors]
  [temperature]
    type = PointValue
    variable = 'solution_temperature'
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
  [massfrac_Na]
    type = PointValue
    variable = massfrac_Na
  []
  [massfrac_K]
    type = PointValue
    variable = massfrac_K
  []
  [massfrac_Ca]
    type = PointValue
    variable = massfrac_Ca
  []
  [massfrac_Mg]
    type = PointValue
    variable = massfrac_Mg
  []
  [massfrac_SiO2]
    type = PointValue
    variable = massfrac_SiO2
  []
  [massfrac_Al]
    type = PointValue
    variable = massfrac_Al
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
  [massfrac_H2O]
    type = PointValue
    variable = massfrac_H2O
  []
  [cm3_Albite]
    type = PointValue
    variable = 'free_cm3_Albite'
  []
  [cm3_Anhydrite]
    type = PointValue
    variable = 'free_cm3_Anhydrite'
  []
  [cm3_Anorthite]
    type = PointValue
    variable = 'free_cm3_Anorthite'
  []
  [cm3_Calcite]
    type = PointValue
    variable = 'free_cm3_Calcite'
  []
  [cm3_Chalcedony]
    type = PointValue
    variable = 'free_cm3_Chalcedony'
  []
  [cm3_Clinochl-7A]
    type = PointValue
    variable = 'free_cm3_Clinochl-7A'
  []
  [cm3_Illite]
    type = PointValue
    variable = 'free_cm3_Illite'
  []
  [cm3_K-feldspar]
    type = PointValue
    variable = 'free_cm3_K-feldspar'
  []
  [cm3_Kaolinite]
    type = PointValue
    variable = 'free_cm3_Kaolinite'
  []
  [cm3_Quartz]
    type = PointValue
    variable = 'free_cm3_Quartz'
  []
  [cm3_Paragonite]
    type = PointValue
    variable = 'free_cm3_Paragonite'
  []
  [cm3_Phlogopite]
    type = PointValue
    variable = 'free_cm3_Phlogopite'
  []
  [cm3_Zoisite]
    type = PointValue
    variable = 'free_cm3_Zoisite'
  []
  [cm3_Laumontite]
    type = PointValue
    variable = 'free_cm3_Laumontite'
  []
  [cm3_mineral]
    type = LinearCombinationPostprocessor
    pp_names = 'cm3_Albite cm3_Anhydrite cm3_Anorthite cm3_Calcite cm3_Chalcedony cm3_Clinochl-7A cm3_Illite cm3_K-feldspar cm3_Kaolinite cm3_Quartz cm3_Paragonite cm3_Phlogopite cm3_Zoisite cm3_Laumontite'
    pp_coefs = '1 1 1 1 1 1 1 1 1 1 1 1 1 1'
  []
  [pH]
    type = PointValue
    variable = 'pH'
  []
[]

[Outputs]
  [exo]
    type = Exodus
    execute_on = final
  []
  csv = true
[]


# Minerals suggested by Stuart Simmons, but I do not include Laumontite and Zoisite as they are more stable than Anorthite so all Anorthite becomes one of these minerals which contradicts the XRD observations.  All minerals are considered in the kinetic models.
# Model of "Water 1" from "Subtask 2C.4.7 Geochemical Modeling SSimmons-VPatil.pdf" subjected to the following:
# (1) The system is equilibrated at 60deg, with pH fixed to 7.5, allowing any precipitates to form.  Note that the only minerals present in the system are those mentioned in "Subtask 2C.4.7 Geochemical Modeling SSimmons-VPatil.pdf".  If other minerals are present, the results change significantly.  Only Quartz and K-feldspar precipitate.
# (2) The system is closed (at time=0), ie the pH is no longer fixed.  The Quartz and K-feldspar precipitates are retained
# (3) The temperature is raised to 220degC (during 0<time<=1), allowing any precipitates to form or dissolve.  Quartz dissolves entirely, K-feldspar precipitate remains, and Calcite and Phlogopite precipitate.  The pH becomes 7.078.  Note the use of remove_all_extrapolated_secondary_species = true in the GeochemicalModelDefinition.  If the extrapolated secondary species are retained instead, the results are significantly different.
# (4) The following minerals are added (during 1<time<=2): Albite (16.8mol = 44% by weight), Anorthite (1.8mol = 5% by weight), K-feldspar (10.4mol = 29% by weight), Quartz (30.0mol = 18% by weight), Phlogopite (0.48mol = 2% by weight) and Illite (0.52mol = 2% by weight).  The mol numbers are approximately what has been measured by XRD, but it is not important to specify the exact composition of the rock (that will be done in the kinetic simulations): what is important here is that there is *some* precipitate.
# (5) The free moles precipitated are Albite 16.38, Anorthite 1.785, K-feldspar 10.68, Quartz 30.82, Phlogopite 0.52, Paragonite 0.44, Calcite 0.0004, Anhydrite 0.0004, Chalcedony 0, Illite 0, Kaolinite 0, Clinochl-7A 0.  Calcite is constrained by the initial HCO3- concentration and Anhydrite by the initial SO4-- concentration, and both have only been observed in trace quantities in agreement with this simulation
# (6) The free mole numbers of the basis species that are now in equilibrium with the minerals are extracted, which is the key output of this simulation.  Note that the original composition of "Water 1" is largely irrelevant.  As mentioned, the HCO3- and SO4-- concentrations constrain Calcite and Anhydrite.  Also, adding the minerals causes the pH to change to 6.16.
[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = '../../../../geochemistry/database/moose_geochemdb.json'
    basis_species = 'H2O H+ Na+ K+ Ca++ Mg++ SiO2(aq) Al+++ Cl- SO4-- HCO3-'
    equilibrium_minerals = 'Albite Anhydrite Anorthite Calcite Chalcedony Clinochl-7A Illite K-feldspar Kaolinite Quartz Paragonite Phlogopite'
    remove_all_extrapolated_secondary_species = true
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'Cl-'
  constraint_species = 'H2O H+      Na+  K+    Ca++    Mg++      SiO2(aq) Al+++    Cl-  SO4--  HCO3-'
  constraint_value = '  1.0 3.16E-8 0.12 0.016 0.68E-3 0.0008E-3 3.7E-3   0.004E-3 0.15 0.5E-3 1.4E-3'
  constraint_meaning = 'kg_solvent_water activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition'
  constraint_unit = 'kg dimensionless moles moles moles moles moles moles moles moles moles'
  initial_temperature = 60
  remove_fixed_activity_name = 'H+'
  remove_fixed_activity_time = 0
  temperature = 220
  source_species_names = 'Albite Anorthite K-feldspar Quartz Phlogopite Illite'
  source_species_rates = 'Albite_rate Anorthite_rate K-feldspar_rate Quartz_rate Phlogopite_rate Illite_rate'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  mol_cutoff = 1E-100
  execute_console_output_on = 'timestep_end' # only CSV output
  solver_info = true
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<1, 1, if(t<1.01, 0.01, 1))'
  []
  end_time = 2
[]

[AuxVariables]
  [Albite_rate]
  []
  [Anorthite_rate]
  []
  [K-feldspar_rate]
  []
  [Quartz_rate]
  []
  [Phlogopite_rate]
  []
  [Illite_rate]
  []
  [transported_H2O]
  []
  [transported_H+]
  []
  [transported_Na+]
  []
  [transported_K+]
  []
  [transported_Ca++]
  []
  [transported_Mg++]
  []
  [transported_SiO2]
  []
  [transported_Al+++]
  []
  [transported_Cl-]
  []
  [transported_SO4--]
  []
  [transported_HCO3-]
  []
[]
[AuxKernels]
  [Albite_rate]
    type = FunctionAux
    variable = Albite_rate
    function = 'if(t>1, 16.8, 0)'
    execute_on = timestep_begin
  []
  [Anorthite_rate]
    type = FunctionAux
    variable = Anorthite_rate
    function = 'if(t>1, 1.8, 0)'
    execute_on = timestep_begin
  []
  [K-feldspar_rate]
    type = FunctionAux
    variable = K-feldspar_rate
    function = 'if(t>1, 10.4, 0)'
    execute_on = timestep_begin
  []
  [Quartz_rate]
    type = FunctionAux
    variable = Quartz_rate
    function = 'if(t>1, 30.0, 0)'
    execute_on = timestep_begin
  []
  [Phlogopite_rate]
    type = FunctionAux
    variable = Phlogopite_rate
    function = 'if(t>1, 0.48, 0)'
    execute_on = timestep_begin
  []
  [Illite_rate]
    type = FunctionAux
    variable = Illite_rate
    function = 'if(t>1, 0.52, 0)'
    execute_on = timestep_begin
  []
  [transported_H2O]
    type = GeochemistryQuantityAux
    species = 'H2O'
    variable = transported_H2O
    quantity = transported_moles_in_original_basis
  []
  [transported_H+]
    type = GeochemistryQuantityAux
    species = 'H+'
    variable = transported_H+
    quantity = transported_moles_in_original_basis
  []
  [transported_Na+]
    type = GeochemistryQuantityAux
    species = 'Na+'
    variable = transported_Na+
    quantity = transported_moles_in_original_basis
  []
  [transported_K+]
    type = GeochemistryQuantityAux
    species = 'K+'
    variable = transported_K+
    quantity = transported_moles_in_original_basis
  []
  [transported_Ca++]
    type = GeochemistryQuantityAux
    species = 'Ca++'
    variable = transported_Ca++
    quantity = transported_moles_in_original_basis
  []
  [transported_Mg++]
    type = GeochemistryQuantityAux
    species = 'Mg++'
    variable = transported_Mg++
    quantity = transported_moles_in_original_basis
  []
  [transported_SiO2]
    type = GeochemistryQuantityAux
    species = 'SiO2(aq)'
    variable = transported_SiO2
    quantity = transported_moles_in_original_basis
  []
  [transported_Al+++]
    type = GeochemistryQuantityAux
    species = 'Al+++'
    variable = transported_Al+++
    quantity = transported_moles_in_original_basis
  []
  [transported_Cl-]
    type = GeochemistryQuantityAux
    species = 'Cl-'
    variable = transported_Cl-
    quantity = transported_moles_in_original_basis
  []
  [transported_SO4--]
    type = GeochemistryQuantityAux
    species = 'SO4--'
    variable = transported_SO4--
    quantity = transported_moles_in_original_basis
  []
  [transported_HCO3-]
    type = GeochemistryQuantityAux
    species = 'HCO3-'
    variable = transported_HCO3-
    quantity = transported_moles_in_original_basis
  []
[]
[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]
[Postprocessors]
  [kg_solvent_water]
    type = PointValue
    variable = kg_solvent_H2O
  []
  [free_cm3_Albite]
    type = PointValue
    variable = free_cm3_Albite
  []
  [free_cm3_Anhydrite]
    type = PointValue
    variable = free_cm3_Anhydrite
  []
  [free_cm3_Anorthite]
    type = PointValue
    variable = free_cm3_Anorthite
  []
  [free_cm3_Calcite]
    type = PointValue
    variable = free_cm3_Calcite
  []
  [free_cm3_Chalcedony]
    type = PointValue
    variable = free_cm3_Chalcedony
  []
  [free_cm3_Clinochl-7A]
    type = PointValue
    variable = free_cm3_Clinochl-7A
  []
  [free_cm3_Illite]
    type = PointValue
    variable = free_cm3_Illite
  []
  [free_cm3_K-feldspar]
    type = PointValue
    variable = free_cm3_K-feldspar
  []
  [free_cm3_Kaolinite]
    type = PointValue
    variable = free_cm3_Kaolinite
  []
  [free_cm3_Quartz]
    type = PointValue
    variable = free_cm3_Quartz
  []
  [free_cm3_Paragonite]
    type = PointValue
    variable = free_cm3_Paragonite
  []
  [free_cm3_Phlogopite]
    type = PointValue
    variable = free_cm3_Phlogopite
  []
  [molal_H+]
    type = PointValue
    variable = molal_H+
  []
  [molal_Na+]
    type = PointValue
    variable = molal_Na+
  []
  [molal_K+]
    type = PointValue
    variable = molal_K+
  []
  [molal_Ca++]
    type = PointValue
    variable = molal_Ca++
  []
  [molal_Mg++]
    type = PointValue
    variable = molal_Mg++
  []
  [molal_SiO2]
    type = PointValue
    variable = molal_SiO2(aq)
  []
  [molal_Al+++]
    type = PointValue
    variable = molal_Al+++
  []
  [molal_SO4--]
    type = PointValue
    variable = molal_SO4--
  []
  [molal_HCO3-]
    type = PointValue
    variable = molal_HCO3-
  []
  [bulk_moles_Cl-]
    type = PointValue
    variable = bulk_moles_Cl-
  []
  [transported_H2O]
    type = PointValue
    variable = transported_H2O
  []
  [transported_H+]
    type = PointValue
    variable = transported_H+
  []
  [transported_Na+]
    type = PointValue
    variable = transported_Na+
  []
  [transported_K+]
    type = PointValue
    variable = transported_K+
  []
  [transported_Ca++]
    type = PointValue
    variable = transported_Ca++
  []
  [transported_Mg++]
    type = PointValue
    variable = transported_Mg++
  []
  [transported_SiO2]
    type = PointValue
    variable = transported_SiO2
  []
  [transported_Al+++]
    type = PointValue
    variable = transported_Al+++
  []
  [transported_Cl-]
    type = PointValue
    variable = transported_Cl-
  []
  [transported_SO4--]
    type = PointValue
    variable = transported_SO4--
  []
  [transported_HCO3-]
    type = PointValue
    variable = transported_HCO3-
  []
  [pH]
    type = PointValue
    variable = pH
  []
[]
[Outputs]
  csv = true
[]


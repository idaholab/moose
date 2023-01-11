#########################################
#                                       #
# File written by create_input_files.py #
#                                       #
#########################################
# Model of the heat-exchanger
# The input fluid to the heat exchanger is determined by AuxVariables called production_temperature, production_rate_H, production_rate_Cl, production_rate_SO4, production_rate_HCO3, production_rate_SiO2aq, production_rate_Al, production_rate_Ca, production_rate_Mg, production_rate_Fe, production_rate_K, production_rate_Na, production_rate_Sr, production_rate_F, production_rate_BOH, production_rate_Br, production_rate_Ba, production_rate_Li, production_rate_NO3, production_rate_O2aq, production_rate_H2O.  These come from Postprocessors in the porous_flow.i simulation that measure the fluid composition at the production well.
# Given the input fluid, the exchanger cools/heats the fluid, removing any precipitates, and injects fluid back to porous_flow.i at temperature output_temperature and composition given by massfrac_H, etc.

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = '../../../../geochemistry/database/moose_geochemdb.json'
    basis_species = 'H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)'
    equilibrium_minerals = 'Siderite Pyrrhotite Dolomite Illite Anhydrite Calcite Quartz K-feldspar Kaolinite Barite Celestite Fluorite Albite Chalcedony Goethite'
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  include_moose_solve = false
  geochemistry_reactor_name = reactor
  swap_out_of_basis = 'NO3- O2(aq)'
  swap_into_basis = '  NH3  HS-'
  charge_balance_species = 'Cl-'
# initial conditions are unimportant because in exchanger mode all existing fluid is flushed from the system before adding the produced water
  constraint_species = 'H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NH3 HS-'
  constraint_value = '1.0 1E-6 1E-6 1E-18 1E-18 1E-18    1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18'
  constraint_meaning = 'kg_solvent_water bulk_composition bulk_composition free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration free_concentration'
  constraint_unit = "kg moles moles molal molal molal molal molal molal molal molal molal molal molal molal molal molal molal molal molal"
  prevent_precipitation = 'Fluorite Albite Goethite'
  initial_temperature = 92
  mode = 4
  temperature = ramp_temperature # ramp up to 160degC over ~1 day so that aquifer geochemistry simulation can easily converge
  cold_temperature = 92
  heating_increments = 10
  source_species_names = ' H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq) H2O'
  source_species_rates = ' production_rate_H production_rate_Cl production_rate_SO4 production_rate_HCO3 production_rate_SiO2aq production_rate_Al production_rate_Ca production_rate_Mg production_rate_Fe production_rate_K production_rate_Na production_rate_Sr production_rate_F production_rate_BOH production_rate_Br production_rate_Ba production_rate_Li production_rate_NO3 production_rate_O2aq production_rate_H2O'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
[]

[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

[AuxVariables]
  [ramp_temperature]
    initial_condition = 92
  []
  [production_temperature]
    initial_condition = 92 # the production_T Transfer lags one timestep behind for some reason, so give this a reasonable initial condition
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
  [dumped_Siderite]
  []
  [dumped_Pyrrhotite]
  []
  [dumped_Dolomite]
  []
  [dumped_Illite]
  []
  [dumped_Anhydrite]
  []
  [dumped_Calcite]
  []
  [dumped_Quartz]
  []
  [dumped_K-feldspar]
  []
  [dumped_Kaolinite]
  []
  [dumped_Barite]
  []
  [dumped_Celestite]
  []
  [dumped_Fluorite]
  []
  [dumped_Albite]
  []
  [dumped_Chalcedony]
  []
  [dumped_Goethite]
  []
# The production_* Transfers lag one timestep behind for some reason (when the porous_flow simulation has finished, it correctly computes mole_rate_*_produced, but the Transfer gets the mole_rate_*_produced from the previous timestep), so give the production_rate_* reasonable initial conditions, otherwise they will be zero at the start of the simulation.
  [production_rate_H]
    initial_condition = -0.00058596786807342
  []
  [production_rate_Cl]
    initial_condition = 0.274767413291287
  []
  [production_rate_SO4]
    initial_condition = 0.012567456786868922
  []
  [production_rate_HCO3]
    initial_condition = 0.0001668295857850308
  []
  [production_rate_SiO2aq]
    initial_condition = 0.00010068057668449495
  []
  [production_rate_Al]
    initial_condition = 2.4224219572143877e-07
  []
  [production_rate_Ca]
    initial_condition = 0.0040997718654983036
  []
  [production_rate_Mg]
    initial_condition = 0.00015261342984691217
  []
  [production_rate_Fe]
    initial_condition = 0.0001550375425863269
  []
  [production_rate_K]
    initial_condition = 0.0003500651859998926
  []
  [production_rate_Na]
    initial_condition = 0.2896767602995328
  []
  [production_rate_Sr]
    initial_condition = 2.915285700108879e-05
  []
  [production_rate_F]
    initial_condition = 5.8582680830041476e-05
  []
  [production_rate_BOH]
    initial_condition = 0.0012157199878760335
  []
  [production_rate_Br]
    initial_condition = 0.00022605948665165203
  []
  [production_rate_Ba]
    initial_condition = 2.2773554030672105e-07
  []
  [production_rate_Li]
    initial_condition = 0.0023920780265869763
  []
  [production_rate_NO3]
    initial_condition = 0.000353470613973057
  []
  [production_rate_O2aq]
    initial_condition = -0.00044255942331181803
  []
  [production_rate_H2O]
    initial_condition = 10.10458252764496
  []
[]

[AuxKernels]
  [ramp_temperature]
    type = FunctionAux
    variable = ramp_temperature
    function = 'min(160, max(92, 92 + (160 - 92) * t / 1E5))'
  []
  [transported_H_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_H
    species = 'H+'
  []
  [transported_Cl_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Cl
    species = 'Cl-'
  []
  [transported_SO4_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_SO4
    species = 'SO4--'
  []
  [transported_HCO3_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_HCO3
    species = 'HCO3-'
  []
  [transported_SiO2aq_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_SiO2aq
    species = 'SiO2(aq)'
  []
  [transported_Al_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Al
    species = 'Al+++'
  []
  [transported_Ca_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Ca
    species = 'Ca++'
  []
  [transported_Mg_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Mg
    species = 'Mg++'
  []
  [transported_Fe_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Fe
    species = 'Fe++'
  []
  [transported_K_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_K
    species = 'K+'
  []
  [transported_Na_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Na
    species = 'Na+'
  []
  [transported_Sr_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Sr
    species = 'Sr++'
  []
  [transported_F_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_F
    species = 'F-'
  []
  [transported_BOH_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_BOH
    species = 'B(OH)3'
  []
  [transported_Br_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Br
    species = 'Br-'
  []
  [transported_Ba_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Ba
    species = 'Ba++'
  []
  [transported_Li_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_Li
    species = 'Li+'
  []
  [transported_NO3_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_NO3
    species = 'NO3-'
  []
  [transported_O2aq_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_O2aq
    species = 'O2(aq)'
  []
  [transported_H2O_auxk]
    type = GeochemistryQuantityAux
    quantity = transported_moles_in_original_basis
    variable = transported_H2O
    species = 'H2O'
  []
  [transported_mass_auxk]
    type = ParsedAux
    coupled_variables = ' transported_H transported_Cl transported_SO4 transported_HCO3 transported_SiO2aq transported_Al transported_Ca transported_Mg transported_Fe transported_K transported_Na transported_Sr transported_F transported_BOH transported_Br transported_Ba transported_Li transported_NO3 transported_O2aq transported_H2O'
    variable = transported_mass
    expression = ' transported_H * 1.0079 + transported_Cl * 35.453 + transported_SO4 * 96.0576 + transported_HCO3 * 61.0171 + transported_SiO2aq * 60.0843 + transported_Al * 26.9815 + transported_Ca * 40.08 + transported_Mg * 24.305 + transported_Fe * 55.847 + transported_K * 39.0983 + transported_Na * 22.9898 + transported_Sr * 87.62 + transported_F * 18.9984 + transported_BOH * 61.8329 + transported_Br * 79.904 + transported_Ba * 137.33 + transported_Li * 6.941 + transported_NO3 * 62.0049 + transported_O2aq * 31.9988 + transported_H2O * 18.01801802'
  []
  [massfrac_H_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_H'
    variable = massfrac_H
    expression = '1.0079 * transported_H / transported_mass'
  []
  [massfrac_Cl_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Cl'
    variable = massfrac_Cl
    expression = '35.453 * transported_Cl / transported_mass'
  []
  [massfrac_SO4_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_SO4'
    variable = massfrac_SO4
    expression = '96.0576 * transported_SO4 / transported_mass'
  []
  [massfrac_HCO3_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_HCO3'
    variable = massfrac_HCO3
    expression = '61.0171 * transported_HCO3 / transported_mass'
  []
  [massfrac_SiO2aq_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_SiO2aq'
    variable = massfrac_SiO2aq
    expression = '60.0843 * transported_SiO2aq / transported_mass'
  []
  [massfrac_Al_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Al'
    variable = massfrac_Al
    expression = '26.9815 * transported_Al / transported_mass'
  []
  [massfrac_Ca_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Ca'
    variable = massfrac_Ca
    expression = '40.08 * transported_Ca / transported_mass'
  []
  [massfrac_Mg_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Mg'
    variable = massfrac_Mg
    expression = '24.305 * transported_Mg / transported_mass'
  []
  [massfrac_Fe_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Fe'
    variable = massfrac_Fe
    expression = '55.847 * transported_Fe / transported_mass'
  []
  [massfrac_K_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_K'
    variable = massfrac_K
    expression = '39.0983 * transported_K / transported_mass'
  []
  [massfrac_Na_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Na'
    variable = massfrac_Na
    expression = '22.9898 * transported_Na / transported_mass'
  []
  [massfrac_Sr_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Sr'
    variable = massfrac_Sr
    expression = '87.62 * transported_Sr / transported_mass'
  []
  [massfrac_F_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_F'
    variable = massfrac_F
    expression = '18.9984 * transported_F / transported_mass'
  []
  [massfrac_BOH_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_BOH'
    variable = massfrac_BOH
    expression = '61.8329 * transported_BOH / transported_mass'
  []
  [massfrac_Br_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Br'
    variable = massfrac_Br
    expression = '79.904 * transported_Br / transported_mass'
  []
  [massfrac_Ba_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Ba'
    variable = massfrac_Ba
    expression = '137.33 * transported_Ba / transported_mass'
  []
  [massfrac_Li_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Li'
    variable = massfrac_Li
    expression = '6.941 * transported_Li / transported_mass'
  []
  [massfrac_NO3_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_NO3'
    variable = massfrac_NO3
    expression = '62.0049 * transported_NO3 / transported_mass'
  []
  [massfrac_O2aq_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_O2aq'
    variable = massfrac_O2aq
    expression = '31.9988 * transported_O2aq / transported_mass'
  []
  [massfrac_H2O_auxk]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_H2O'
    variable = massfrac_H2O
    expression = '18.01801802 * transported_H2O / transported_mass'
  []
  [dumped_Siderite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Siderite
    species = Siderite
    quantity = moles_dumped
  []
  [dumped_Pyrrhotite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Pyrrhotite
    species = Pyrrhotite
    quantity = moles_dumped
  []
  [dumped_Dolomite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Dolomite
    species = Dolomite
    quantity = moles_dumped
  []
  [dumped_Illite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Illite
    species = Illite
    quantity = moles_dumped
  []
  [dumped_Anhydrite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Anhydrite
    species = Anhydrite
    quantity = moles_dumped
  []
  [dumped_Calcite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Calcite
    species = Calcite
    quantity = moles_dumped
  []
  [dumped_Quartz_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Quartz
    species = Quartz
    quantity = moles_dumped
  []
  [dumped_K-feldspar_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_K-feldspar
    species = K-feldspar
    quantity = moles_dumped
  []
  [dumped_Kaolinite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Kaolinite
    species = Kaolinite
    quantity = moles_dumped
  []
  [dumped_Barite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Barite
    species = Barite
    quantity = moles_dumped
  []
  [dumped_Celestite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Celestite
    species = Celestite
    quantity = moles_dumped
  []
  [dumped_Fluorite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Fluorite
    species = Fluorite
    quantity = moles_dumped
  []
  [dumped_Albite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Albite
    species = Albite
    quantity = moles_dumped
  []
  [dumped_Chalcedony_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Chalcedony
    species = Chalcedony
    quantity = moles_dumped
  []
  [dumped_Goethite_auxk]
    type = GeochemistryQuantityAux
    variable = dumped_Goethite
    species = Goethite
    quantity = moles_dumped
  []
[]

[Postprocessors]
  [cumulative_moles_precipitated_Siderite]
    type = PointValue
    variable = dumped_Siderite
  []
  [cumulative_moles_precipitated_Pyrrhotite]
    type = PointValue
    variable = dumped_Pyrrhotite
  []
  [cumulative_moles_precipitated_Dolomite]
    type = PointValue
    variable = dumped_Dolomite
  []
  [cumulative_moles_precipitated_Illite]
    type = PointValue
    variable = dumped_Illite
  []
  [cumulative_moles_precipitated_Anhydrite]
    type = PointValue
    variable = dumped_Anhydrite
  []
  [cumulative_moles_precipitated_Calcite]
    type = PointValue
    variable = dumped_Calcite
  []
  [cumulative_moles_precipitated_Quartz]
    type = PointValue
    variable = dumped_Quartz
  []
  [cumulative_moles_precipitated_K-feldspar]
    type = PointValue
    variable = dumped_K-feldspar
  []
  [cumulative_moles_precipitated_Kaolinite]
    type = PointValue
    variable = dumped_Kaolinite
  []
  [cumulative_moles_precipitated_Barite]
    type = PointValue
    variable = dumped_Barite
  []
  [cumulative_moles_precipitated_Celestite]
    type = PointValue
    variable = dumped_Celestite
  []
  [cumulative_moles_precipitated_Fluorite]
    type = PointValue
    variable = dumped_Fluorite
  []
  [cumulative_moles_precipitated_Albite]
    type = PointValue
    variable = dumped_Albite
  []
  [cumulative_moles_precipitated_Chalcedony]
    type = PointValue
    variable = dumped_Chalcedony
  []
  [cumulative_moles_precipitated_Goethite]
    type = PointValue
    variable = dumped_Goethite
  []
  [production_temperature]
    type = PointValue
    variable = production_temperature
  []
  [mass_heated_this_timestep]
    type = PointValue
    variable = transported_mass
  []
[]

[Outputs]
  csv = true
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

[MultiApps]
  [porous_flow_sim]
    type = TransientMultiApp
    input_files = porous_flow.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [injection_T]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'solution_temperature'
    variable = 'injection_temperature'
  []
  [injection_H]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_H'
    variable = 'injection_rate_massfrac_H'
  []
  [injection_Cl]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Cl'
    variable = 'injection_rate_massfrac_Cl'
  []
  [injection_SO4]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_SO4'
    variable = 'injection_rate_massfrac_SO4'
  []
  [injection_HCO3]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_HCO3'
    variable = 'injection_rate_massfrac_HCO3'
  []
  [injection_SiO2aq]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_SiO2aq'
    variable = 'injection_rate_massfrac_SiO2aq'
  []
  [injection_Al]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Al'
    variable = 'injection_rate_massfrac_Al'
  []
  [injection_Ca]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Ca'
    variable = 'injection_rate_massfrac_Ca'
  []
  [injection_Mg]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Mg'
    variable = 'injection_rate_massfrac_Mg'
  []
  [injection_Fe]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Fe'
    variable = 'injection_rate_massfrac_Fe'
  []
  [injection_K]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_K'
    variable = 'injection_rate_massfrac_K'
  []
  [injection_Na]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Na'
    variable = 'injection_rate_massfrac_Na'
  []
  [injection_Sr]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Sr'
    variable = 'injection_rate_massfrac_Sr'
  []
  [injection_F]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_F'
    variable = 'injection_rate_massfrac_F'
  []
  [injection_BOH]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_BOH'
    variable = 'injection_rate_massfrac_BOH'
  []
  [injection_Br]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Br'
    variable = 'injection_rate_massfrac_Br'
  []
  [injection_Ba]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Ba'
    variable = 'injection_rate_massfrac_Ba'
  []
  [injection_Li]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Li'
    variable = 'injection_rate_massfrac_Li'
  []
  [injection_NO3]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_NO3'
    variable = 'injection_rate_massfrac_NO3'
  []
  [injection_O2aq]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_O2aq'
    variable = 'injection_rate_massfrac_O2aq'
  []
  [injection_H2O]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_H2O'
    variable = 'injection_rate_massfrac_H2O'
  []

  [production_T]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = production_temperature
    variable = production_temperature
  []
  [production_H]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_H_produced
    variable = production_rate_H
  []
  [production_Cl]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Cl_produced
    variable = production_rate_Cl
  []
  [production_SO4]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_SO4_produced
    variable = production_rate_SO4
  []
  [production_HCO3]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_HCO3_produced
    variable = production_rate_HCO3
  []
  [production_SiO2aq]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_SiO2aq_produced
    variable = production_rate_SiO2aq
  []
  [production_Al]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Al_produced
    variable = production_rate_Al
  []
  [production_Ca]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Ca_produced
    variable = production_rate_Ca
  []
  [production_Mg]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Mg_produced
    variable = production_rate_Mg
  []
  [production_Fe]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Fe_produced
    variable = production_rate_Fe
  []
  [production_K]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_K_produced
    variable = production_rate_K
  []
  [production_Na]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Na_produced
    variable = production_rate_Na
  []
  [production_Sr]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Sr_produced
    variable = production_rate_Sr
  []
  [production_F]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_F_produced
    variable = production_rate_F
  []
  [production_BOH]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_BOH_produced
    variable = production_rate_BOH
  []
  [production_Br]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Br_produced
    variable = production_rate_Br
  []
  [production_Ba]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Ba_produced
    variable = production_rate_Ba
  []
  [production_Li]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Li_produced
    variable = production_rate_Li
  []
  [production_NO3]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_NO3_produced
    variable = production_rate_NO3
  []
  [production_O2aq]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_O2aq_produced
    variable = production_rate_O2aq
  []
  [production_H2O]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_H2O_produced
    variable = production_rate_H2O
  []
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../../geochemistry/database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)"
    equilibrium_minerals = "Siderite Pyrrhotite Dolomite Illite Anhydrite Calcite Quartz K-feldspar Kaolinite Barite Celestite Fluorite Albite Chalcedony Goethite"
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "NO3- H+         Fe++       Ba++   SiO2(aq) Mg++     O2(aq)   Al+++   K+     Ca++      HCO3-"
  swap_into_basis = "  NH3  Pyrrhotite K-feldspar Barite Quartz   Dolomite Siderite Calcite Illite Anhydrite Kaolinite"
  charge_balance_species = "Cl-"
  constraint_species = "H2O        Quartz     Calcite   K-feldspar Siderite  Dolomite  Anhydrite Pyrrhotite Illite    Kaolinite  Barite       Na+       Cl-       SO4--       Li+         B(OH)3      Br-         F-         Sr++        NH3"
  constraint_value = "  0.99778351 322.177447 12.111108 6.8269499  6.2844304 2.8670301 1.1912027 0.51474767 0.3732507 0.20903322 0.0001865889 1.5876606 1.5059455 0.046792579 0.013110503 0.006663119 0.001238987 0.00032108 0.000159781 0.001937302"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles"
  prevent_precipitation = "Fluorite Albite Goethite"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  initial_temperature = 92
  mode = 1 # dump all minerals at the start of each time-step
  temperature = temp_controller
  execute_console_output_on = '' # only CSV output for this problem
  stoichiometric_ionic_str_using_Cl_only = true
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 1.0
[]

[AuxVariables]
  [temp_controller]
  []
  [Anhydrite_mol]
  []
  [Dolomite_mol]
  []
  [Pyrrhotite_mol]
  []
  [K-feldspar_mol]
  []
  [Barite_mol]
  []
  [Quartz_mol]
  []
  [Calcite_mol]
  []
  [Illite_mol]
  []
  [Kaolinite_mol]
  []
[]
[AuxKernels]
  [temp_controller_auxk]
    type = FunctionAux
    variable = temp_controller
    function = '92 + (160 - 92) * t'
    execute_on = timestep_begin
  []
  [Anhydrite_mol_auxk]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Anhydrite_mol
    species = Anhydrite
    quantity = moles_dumped
  []
  [Dolomite_mol_auxk]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Dolomite_mol
    species = Dolomite
    quantity = moles_dumped
  []
  [Pyrrhotite_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Pyrrhotite_mol
    species = Pyrrhotite
    quantity = moles_dumped
  []
  [K-feldspar_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = K-feldspar_mol
    species = K-feldspar
    quantity = moles_dumped
  []
  [Barite_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Barite_mol
    species = Barite
    quantity = moles_dumped
  []
  [Quartz_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Quartz_mol
    species = Quartz
    quantity = moles_dumped
  []
  [Calcite_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Calcite_mol
    species = Calcite
    quantity = moles_dumped
  []
  [Illite_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Illite_mol
    species = Illite
    quantity = moles_dumped
  []
  [Kaolinite_mol]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = Kaolinite_mol
    species = Kaolinite
    quantity = moles_dumped
  []
[]
[GlobalParams]
  point = '0 0 0'
[]
[Postprocessors]
  [temperature]
    type = PointValue
    variable = temp_controller
  []
  [Anhydrite_mol]
    type = PointValue
    variable = Anhydrite_mol
  []
  [Dolomite_mol]
    type = PointValue
    variable = Dolomite_mol
  []
  [Pyrrhotite_mol]
    type = PointValue
    variable = Pyrrhotite_mol
  []
  [K-feldspar_mol]
    type = PointValue
    variable = K-feldspar_mol
  []
  [Barite_mol]
    type = PointValue
    variable = Barite_mol
  []
  [Quartz_mol]
    type = PointValue
    variable = Quartz_mol
  []
  [Calcite_mol]
    type = PointValue
    variable = Calcite_mol
  []
  [Illite_mol]
    type = PointValue
    variable = Illite_mol
  []
  [Kaolinite_mol]
    type = PointValue
    variable = Kaolinite_mol
  []
[]
[Outputs]
  csv = true
[]

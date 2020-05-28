#Progressively remove H2O until virtually none remains, all the while removing any minerals that precipitate
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "H+"
  swap_into_basis = "  CO2(g)"
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O              CO2(g)       Cl-                Na+                SO4--              Mg++               Ca++               K+                 HCO3-"
  constraint_value = "  1.0              0.0003162278 0.5656             0.4850             0.02924            0.05501            0.01063            0.010576055        0.002412"
  constraint_meaning = "kg_solvent_water fugacity     moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
  close_system_at_time = 0
  source_species_names = "H2O"
  source_species_rates = "-1.0" # 1kg H2O = 55.51 moles, each time step removes 1 mole
  max_ionic_strength = 100
  ramp_max_ionic_strength = 0
  mode = mode
  execute_console_output_on = 'initial final'
[]

[AuxVariables]
  [./mode]
  [../]
[]

[AuxKernels]
  [./mode]
    type = FunctionAux
    variable = mode
    function = 'if(t<=1.0, 1.0, 2.0)' # initial "dump" then "flow_through"
    execute_on = 'timestep_begin'
  [../]
[]

[Postprocessors]
  [./anydrite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Anhydrite'
  [../]
  [./bischofite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Bischofite'
  [../]
  [./bloedite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Bloedite'
  [../]
  [./carnallite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Carnallite'
  [../]
  [./dolomite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Dolomite'
  [../]
  [./epsomite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Epsomite'
  [../]
  [./gypsum]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Gypsum'
  [../]
  [./halite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Halite'
  [../]
  [./hexahydrite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Hexahydrite'
  [../]
  [./kainite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Kainite'
  [../]
  [./kieserite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Kieserite'
  [../]
  [./magnesite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Magnesite'
  [../]
[]

[Executioner]
  type = Transient
  dt = 5
  end_time = 54
[]


[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Ca++ Mg++ Na+ K+ SO4-- HCO3-"
    equilibrium_minerals = "Anhydrite Bischofite Bloedite Carnallite Dolomite Epsomite Gypsum Halite Hexahydrite Kainite Kieserite Magnesite"
    equilibrium_gases = "CO2(g)"
  [../]
[]


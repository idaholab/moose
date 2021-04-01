#Add K-feldspar and observe precipiates forming
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Na+ K+ Ca++ Mg++ Al+++ SiO2(aq) Cl- SO4-- HCO3-"
  constraint_value = "  1.0 1E-5 2.175E-04 2.558E-05 3.743E-04 1.234E-04 3.706E-08 4.993E-05 8.463E-04 8.328E-05 8.194E-04"
  constraint_meaning = "kg_solvent_water activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg             dimensionless moles           moles              moles              moles              moles              moles              moles              moles              moles"
  ramp_max_ionic_strength_initial = 0
  close_system_at_time = 0
  source_species_names = "K-feldspar"
  source_species_rates = "1.37779E-3" # 0.15cm^3 of K-feldspar (molar volume = 108.87 cm^3/mol) = 1.37779E-3 mol
  remove_fixed_activity_name = "H+"
  remove_fixed_activity_time = 0
  execute_console_output_on = 'final'
[]

[Postprocessors]
  [cm3_K-feldspar]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_K-feldspar'
  []
  [cm3_Kaolinite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Kaolinite'
  []
  [cm3_Muscovite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Muscovite'
  []
  [cm3_Quartz]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Quartz'
  []
  [cm3_Phengite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Phengite'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ K+ Ca++ Mg++ Al+++ SiO2(aq) Cl- SO4-- HCO3-"
    equilibrium_minerals = "K-feldspar Kaolinite Muscovite Quartz Phengite"
  []
[]


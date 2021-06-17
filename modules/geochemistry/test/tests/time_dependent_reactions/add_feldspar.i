#Add K-feldspar and observe precipiates forming
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Na+              K+               Ca++             Mg++             Al+++            SiO2(aq)         Cl-              SO4--            HCO3-"
  constraint_value = "  1.0              -5            5                1                15               3                1                3                30               8                50"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless mg               mg               mg               mg               ug               mg               mg               mg               mg"
  source_species_names = "K-feldspar"
  source_species_rates = "1.37779E-3" # 0.15cm^3 of K-feldspar (molar volume = 108.87 cm^3/mol) = 1.37779E-3 mol
  remove_fixed_activity_name = "H+"
  remove_fixed_activity_time = 0
  ramp_max_ionic_strength_initial = 0 # not needed for this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = '' # only CSV output for this example
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
  dt = 0.01
  end_time = 1
[]

[Outputs]
  csv = true
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ K+ Ca++ Mg++ Al+++ SiO2(aq) Cl- SO4-- HCO3-"
    equilibrium_minerals = "K-feldspar Kaolinite Muscovite Quartz Phengite"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]


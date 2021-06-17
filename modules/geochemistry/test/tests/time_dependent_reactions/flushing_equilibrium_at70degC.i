# Alkali flushing of a reservoir (an example of flushing)
# This input file finds the molality of SiO2(aq), which is needed because when Quartz is specified as kinetic it cannot appear in the basis
[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Ca++ HCO3- Mg++ K+ Al+++ SiO2(aq)"
    equilibrium_minerals = "Calcite Dolomite-ord Muscovite Kaolinite Quartz"
  []
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  swap_into_basis = "Calcite Dolomite-ord Muscovite Kaolinite Quartz"
  swap_out_of_basis = "HCO3- Mg++ K+ Al+++ SiO2(aq)"
  constraint_species = "H2O              H+            Cl-              Na+              Ca++             Calcite      Dolomite-ord Muscovite    Kaolinite    Quartz"
  constraint_value = "  1.0              -5            1.0              1.0              0.2              9.88249      3.652471265  1.2792268    1.2057878    226.99243"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition bulk_composition free_mineral free_mineral free_mineral free_mineral free_mineral"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles        moles        moles        moles        moles"
  temperature = 70.0
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = ''
  abs_tol = 1E-14
[]

[GlobalParams]
  point = '0 0 0'
[]
[Postprocessors]
  [pH]
    type = PointValue
    variable = "pH"
  []
  [molality_sio2]
    type = PointValue
    variable = 'molal_SiO2(aq)'
  []
[]

[Outputs]
  csv = true
[]

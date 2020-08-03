#Pyrite is added, and the fugacity of O2(g) is not fixed
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "O2(aq) Fe++"
  swap_into_basis = "O2(g) Hematite"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              Hematite                   H+         Ca++               Mg++               Na+                HCO3-              SO4--              Cl-                O2(g)"
  constraint_value = "  1.0              6.26E-6                    3.16228E-7 9.98E-5            4.114E-5           8.7E-5             2.95E-4            3.123E-5           1.4103E-4          0.2"
  constraint_meaning = "kg_solvent_water free_moles_mineral_species activity   moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species fugacity"
  remove_fixed_activity_name = "H+ O2(g)"
  remove_fixed_activity_time = '0  0'
  source_species_names = "Pyrite"
  source_species_rates = 8.336E-6 # = 1mg(pyrite)/second, 1mg(pyrite) = 8.34E-6
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  abs_tol = 1E-13
  execute_console_output_on = '' # only CSV output is required
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Fe++ Ca++ Mg++ Na+ HCO3- SO4-- Cl- O2(aq)"
    equilibrium_minerals = "Hematite Pyrite"
    equilibrium_gases = "O2(g)"
    piecewise_linear_interpolation = true # for comparison with GWB
  [../]
[]

[Postprocessors]
  [./mg_Hematite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_mg_Hematite'
  [../]
  [./mg_Pyrite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_mg_Pyrite'
  [../]
  [./pH]
    type = PointValue
    point = '0 0 0'
    variable = 'pH'
  [../]
  [./molal_CO2aq]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_CO2(aq)'
  [../]
  [./molal_HCO3-]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_HCO3-'
  [../]
  [./molal_SO4--]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_SO4--'
  [../]
  [./molal_Fe++]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_Fe++'
  [../]
  [./molal_O2aq]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_O2(aq)'
  [../]
[]
[Outputs]
  csv = true
[]

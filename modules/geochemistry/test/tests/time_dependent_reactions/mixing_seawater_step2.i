# Finds the equilibrium configuration for seawater, step 2.  (The fluid can then mixed into the fluid defined in mixing.i)
# Note:
# The Geochemists Workbench software first equilibrates the system at pH=8.1 without allowing precipitates to form,
# THEN closes the system and allows precipitates to form.
# This MOOSE input file does the second stage: precipitation with fixed H+ and O2(aq) bulk moles
[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+                   Cl-    Na+    Mg++    SO4--   Ca++    K+      HCO3-  Ba++    SiO2(aq) Sr++    Zn++    Cu+      Al+++    Fe++     Mn++     O2(aq)"
  constraint_value = "1.0                -4.6588015784953e-05 559E-3 480E-3 54.5E-3 29.5E-3 10.5E-3 10.1E-3 2.4E-3 0.20E-6 0.17E-3  0.09E-3 0.01E-6 0.007E-6 0.005E-6 0.001E-6 0.001E-6 0.00012300200124001"
  constraint_meaning = "kg_solvent_water bulk_composition   bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles"
  min_initial_molality = 1E-15
  temperature = 4
  abs_tol = 1e-14
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = ''
[]

[Postprocessors]
  [H2O]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_H2O"
  []
  [Cl-]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Cl-"
  []
  [Na+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Na+"
  []
  [Mg++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Mg++"
  []
  [SO4--]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_SO4--"
  []
  [K+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_K+"
  []
  [Ca++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Ca++"
  []
  [HCO3-]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_HCO3-"
  []
  [O2aq]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_O2(aq)"
  []
  [SiO2aq]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_SiO2(aq)"
  []
  [bulk_Dolomite-ord]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Dolomite-ord"
  []
  [free_mg_Dolomite-ord]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Dolomite-ord"
  []
  [bulk_Strontianite]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Strontianite"
  []
  [free_mg_Strontianite]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Strontianite"
  []
  [bulk_Barite]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Barite"
  []
  [free_mg_Barite]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Barite"
  []
  [Zn++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Zn++"
  []
  [bulk_Pyrolusite]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Pyrolusite"
  []
  [free_mg_Pyrolusite]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Pyrolusite"
  []
  [bulk_Muscovite]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Muscovite"
  []
  [free_mg_Muscovite]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Muscovite"
  []
  [bulk_Nontronit-Na]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Nontronit-Na"
  []
  [free_mg_Nontronit-Na]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Nontronit-Na"
  []
  [Cu+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Cu+"
  []
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Mg++ SO4-- Ca++ K+ HCO3- Ba++ SiO2(aq) Sr++ Zn++ Cu+ Al+++ Fe++ Mn++ O2(aq)"
    equilibrium_minerals = "Anhydrite Pyrite Talc Amrph^silica Barite Dolomite-ord Muscovite Nontronit-Na Pyrolusite Strontianite"
  []
[]

[Outputs]
  csv = true
[]


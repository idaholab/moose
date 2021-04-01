# Finds the equilibrium configuration for seawater, step 1.  (The fluid can then mixed into the fluid defined in mixing.i)
# Note:
# The Geochemists Workbench software first equilibrates the system at pH=8.1 without allowing precipitates to form,
# THEN closes the system and allows precipitates to form.
# This MOOSE input file does the first stage: equilibration of the system at pH=8.1 without any precipitation
# The key quantities to find are the bulk moles for H+ and O2(aq)
[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+          Cl-    Na+    Mg++    SO4--   Ca++    K+      HCO3-  Ba++    SiO2(aq) Sr++    Zn++    Cu+      Al+++    Fe++     Mn++     O2(aq)"
  constraint_value = "1.0                7.943282E-9 559E-3 480E-3 54.5E-3 29.5E-3 10.5E-3 10.1E-3 2.4E-3 0.20E-6 0.17E-3  0.09E-3 0.01E-6 0.007E-6 0.005E-6 0.001E-6 0.001E-6 123E-6"
  constraint_meaning = "kg_solvent_water activity    bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition free_concentration"
  constraint_unit = "kg dimensionless moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles molal"
  temperature = 4
  abs_tol = 1e-14
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-5
[]

[Postprocessors]
  [H+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_H+"
  []
  [O2aq]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_O2(aq)"
  []
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Mg++ SO4-- Ca++ K+ HCO3- Ba++ SiO2(aq) Sr++ Zn++ Cu+ Al+++ Fe++ Mn++ O2(aq)"
  []
[]

[Outputs]
  csv = true
[]


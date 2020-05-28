#Finds the equilibrium configuration for seawater.  This is then mixed into the fluid defined in mixing_2.i
[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+     Cl-    Na+    Mg++    SO4--   Ca++    K+      HCO3-  Ba++    SiO2(aq) Sr++    Zn++    Cu+   Al+++ Fe++  Mn++  O2(aq)"
  constraint_value = "1.0 7.943E-9 559E-3 480E-3 54.5E-3 29.5E-3 10.5E-3 10.1E-3 2.4E-3 0.20E-6 0.17E-3  0.09E-3 0.01E-6 0.007E-6 0.005E-6 0.001E-6 0.001E-6 123E-6"
  constraint_meaning = "kg_solvent_water activity moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species free_molality"
  temperature = 4
[]

[Postprocessors]
  [./H2O]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_H2O"
  [../]
  [./H+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_H+"
  [../]
  [./Cl-]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Cl-"
  [../]
  [./Na+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Na+"
  [../]
  [./Mg++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Mg++"
  [../]
  [./SO4--]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_SO4--"
  [../]
  [./K+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_K+"
  [../]
  [./HCO3-]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_HCO3-"
  [../]
  [./Ba++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Ba++"
  [../]
  [./SiO2aq]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_SiO2(aq)"
  [../]
  [./Sr++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Sr++"
  [../]
  [./Zn++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Zn++"
  [../]
  [./Cu+]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Cu+"
  [../]
  [./Al+++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Al+++"
  [../]
  [./Fe++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Fe++"
  [../]
  [./Mn++]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_Mn++"
  [../]
  [./O2aq]
    type = PointValue
    point = '0 0 0'
    variable = "bulk_moles_O2(aq)"
  [../]
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Mg++ SO4-- Ca++ K+ HCO3- Ba++ SiO2(aq) Sr++ Zn++ Cu+ Al+++ Fe++ Mn++ O2(aq)"
  [../]
[]

[Outputs]
  csv = true
[]


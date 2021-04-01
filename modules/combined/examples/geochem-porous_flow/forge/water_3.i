# Equilibrium model "Water 3" from "Subtask 2C.4.7 Geochemical Modeling SSimmons-VPatil.pdf".  The steps followed in this input file are:
# 1. The initial equilibrium is found at 20degC.  This is the temperature at which the bulk composition was measured, and at this temperature most species are supersaturated.  However, since measurements were performed in the absence of free minerals, their precipitation must be retarded in some way, so all minerals are prevented from precipitating in the model
# 2. The pH constraint is removed and the system is raised to 70degC, which is the injection temperature.  This causes the pH to drop from 6.2 to 6.1, and only Kaolinite and Illite are supersaturated
# 3. The free molality of the species is measured for use in other models
[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = '../../../../geochemistry/database/moose_geochemdb.json'
    basis_species = 'H2O H+ Na+ K+ Ca++ Mg++ SiO2(aq) Al+++ Cl- SO4-- HCO3-'
    equilibrium_minerals = 'Albite Anhydrite Anorthite Calcite Chalcedony Clinochl-7A Illite K-feldspar Kaolinite Quartz Paragonite Phlogopite Zoisite Laumontite'
    remove_all_extrapolated_secondary_species = true
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'Cl-'
  constraint_species = 'H2O H+      Na+     K+      Ca++    Mg++    SiO2(aq) Al+++   Cl-     SO4--   HCO3-'
  constraint_value = '  1.0 6.31E-7 1.32E-4 2.81E-5 7.76E-5 2.88E-5 2.73E-4  3.71E-6 1.41E-5 1.04E-5 3.28E-4'
  constraint_meaning = 'kg_solvent_water activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition'
  constraint_unit = '   kg            dimensionless moles moles moles moles moles moles moles moles moles'
  prevent_precipitation = 'Albite Anhydrite Anorthite Calcite Chalcedony Clinochl-7A Illite K-feldspar Kaolinite Quartz Paragonite Phlogopite Zoisite Laumontite'
  initial_temperature = 20
  remove_fixed_activity_name = 'H+'
  remove_fixed_activity_time = 0
  temperature = 70
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  execute_console_output_on = 'final' # only CSV output needed
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

[AuxVariables]
  [transported_H2O]
  []
  [transported_H+]
  []
  [transported_Na+]
  []
  [transported_K+]
  []
  [transported_Ca++]
  []
  [transported_Mg++]
  []
  [transported_SiO2]
  []
  [transported_Al+++]
  []
  [transported_Cl-]
  []
  [transported_SO4--]
  []
  [transported_HCO3-]
  []
[]

[AuxKernels]
  [transported_H2O]
    type = GeochemistryQuantityAux
    species = 'H2O'
    variable = transported_H2O
    quantity = transported_moles_in_original_basis
  []
  [transported_H+]
    type = GeochemistryQuantityAux
    species = 'H+'
    variable = transported_H+
    quantity = transported_moles_in_original_basis
  []
  [transported_Na+]
    type = GeochemistryQuantityAux
    species = 'Na+'
    variable = transported_Na+
    quantity = transported_moles_in_original_basis
  []
  [transported_K+]
    type = GeochemistryQuantityAux
    species = 'K+'
    variable = transported_K+
    quantity = transported_moles_in_original_basis
  []
  [transported_Ca++]
    type = GeochemistryQuantityAux
    species = 'Ca++'
    variable = transported_Ca++
    quantity = transported_moles_in_original_basis
  []
  [transported_Mg++]
    type = GeochemistryQuantityAux
    species = 'Mg++'
    variable = transported_Mg++
    quantity = transported_moles_in_original_basis
  []
  [transported_SiO2]
    type = GeochemistryQuantityAux
    species = 'SiO2(aq)'
    variable = transported_SiO2
    quantity = transported_moles_in_original_basis
  []
  [transported_Al+++]
    type = GeochemistryQuantityAux
    species = 'Al+++'
    variable = transported_Al+++
    quantity = transported_moles_in_original_basis
  []
  [transported_Cl-]
    type = GeochemistryQuantityAux
    species = 'Cl-'
    variable = transported_Cl-
    quantity = transported_moles_in_original_basis
  []
  [transported_SO4--]
    type = GeochemistryQuantityAux
    species = 'SO4--'
    variable = transported_SO4--
    quantity = transported_moles_in_original_basis
  []
  [transported_HCO3-]
    type = GeochemistryQuantityAux
    species = 'HCO3-'
    variable = transported_HCO3-
    quantity = transported_moles_in_original_basis
  []
[]

[Postprocessors]
  [temperature]
    type = PointValue
    variable = 'solution_temperature'
  []
  [bulk_Cl]
    type = PointValue
    variable = 'bulk_moles_Cl-'
  []
  [kg_solvent_H2O]
    type = PointValue
    variable = 'kg_solvent_H2O'
  []
  [molal_H+]
    type = PointValue
    variable = 'molal_H+'
  []
  [molal_Na+]
    type = PointValue
    variable = 'molal_Na+'
  []
  [molal_K+]
    type = PointValue
    variable = 'molal_K+'
  []
  [molal_Ca++]
    type = PointValue
    variable = 'molal_Ca++'
  []
  [molal_Mg++]
    type = PointValue
    variable = 'molal_Mg++'
  []
  [molal_SiO2aq]
    type = PointValue
    variable = 'molal_SiO2(aq)'
  []
  [molal_Al+++]
    type = PointValue
    variable = 'molal_Al+++'
  []
  [molal_Cl-]
    type = PointValue
    variable = 'molal_Cl-'
  []
  [molal_SO4--]
    type = PointValue
    variable = 'molal_SO4--'
  []
  [molal_HCO3-]
    type = PointValue
    variable = 'molal_HCO3-'
  []
  [transported_H2O]
    type = PointValue
    variable = transported_H2O
  []
  [transported_H+]
    type = PointValue
    variable = transported_H+
  []
  [transported_Na+]
    type = PointValue
    variable = transported_Na+
  []
  [transported_K+]
    type = PointValue
    variable = transported_K+
  []
  [transported_Ca++]
    type = PointValue
    variable = transported_Ca++
  []
  [transported_Mg++]
    type = PointValue
    variable = transported_Mg++
  []
  [transported_SiO2]
    type = PointValue
    variable = transported_SiO2
  []
  [transported_Al+++]
    type = PointValue
    variable = transported_Al+++
  []
  [transported_Cl-]
    type = PointValue
    variable = transported_Cl-
  []
  [transported_SO4--]
    type = PointValue
    variable = transported_SO4--
  []
  [transported_HCO3-]
    type = PointValue
    variable = transported_HCO3-
  []
[]

[Outputs]
  csv = true
[]


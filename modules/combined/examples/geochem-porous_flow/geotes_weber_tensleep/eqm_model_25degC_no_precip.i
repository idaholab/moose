[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../../geochemistry/database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)"
    equilibrium_minerals = "Siderite Pyrrhotite Dolomite Illite Anhydrite Calcite Quartz K-feldspar Kaolinite Barite Celestite Fluorite Albite Chalcedony Goethite"
  []
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "NO3- O2(aq)"
  swap_into_basis = "  NH3  HS-"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+       Cl-         SO4--       HCO3-       HS-         SiO2(aq)    Al+++       Ca++        Mg++        Fe++        K+          Na+         Sr++        F-         B(OH)3      Br-         Ba++        Li+         NH3"
  constraint_value = "  1.0              3.467E-7 1.619044933 0.062774835 0.065489838 0.003840583 0.001597755 0.000129719 0.013448104 0.001851471 0.000787867 0.048851229 1.587660615 0.000159781 0.00032108 0.006663119 0.001238987 0.000101944 0.013110503 0.001937302"
  constraint_meaning = "kg_solvent_water activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg dimensionless moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles"
  prevent_precipitation = "Barite Siderite Pyrrhotite Dolomite Illite Anhydrite Calcite Quartz K-feldspar Kaolinite Celestite Fluorite Albite Chalcedony Goethite"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  temperature = 25
  stoichiometric_ionic_str_using_Cl_only = true
  precision = 5
[]

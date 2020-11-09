# This is basically a copy of spatial_4.i but with some more console output that might help pinpoint the threading problem
[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl-"
  [../]
[]

[SpatialReactionSolver]
    model_definition = definition
    charge_balance_species = "Cl-"
    constraint_species = "H2O H+ Cl-"
    constraint_value = "  55.5 1E-5 1E-5"
    constraint_meaning = "moles_bulk_water moles_bulk_species moles_bulk_species"
    source_species_names = HCl
    source_species_rates = HCl_rate
[]

[Postprocessors]
  [./cl0]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0 0 0'
  [../]
  [./cl1]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.1 0 0'
  [../]
  [./cl2]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.2 0 0'
  [../]
  [./cl3]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.3 0 0'
  [../]
  [./cl4]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.4 0 0'
  [../]
  [./cl5]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.5 0 0'
  [../]
  [./cl6]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.6 0 0'
  [../]
  [./cl7]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.7 0 0'
  [../]
  [./cl8]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.8 0 0'
  [../]
  [./cl9]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '0.9 0 0'
  [../]
  [./cl10]
    type = PointValue
    variable = 'bulk_moles_Cl-'
    point = '1.0 0 0'
  [../]
[]
[VectorPostprocessors]
  [./bulk_Cl]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    sort_by = x
    num_points = 11
    variable = 'bulk_moles_Cl-'
  [../]
[]

[AuxVariables]
  [./HCl_rate]
  [../]
[]

[AuxKernels]
  [./HCl_rate]
    type = FunctionAux
    variable = HCl_rate
    function = '1E-5 * x'
    execute_on = timestep_begin # so the Reactor gets the correct value
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 1
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
[]

[Outputs]
  csv = true
[]


sample_file = "models/sampled_combinations.csv"

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]
[Variables]
  [dummy_var]
  []
[]
[Problem]
  kernel_coverage_check = false
[]

[Functions]
  [vmStress_fcn]
    type = PiecewiseConstant
    data_file = ${sample_file}
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [temperature_fcn]
    type = PiecewiseConstant
    data_file = ${sample_file}
    x_index_in_file = 0
    y_index_in_file = 2
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [epStrain_fcn]
    type = PiecewiseConstant
    data_file = ${sample_file}
    x_index_in_file = 0
    y_index_in_file = 3
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [celldd_fcn]
    type = PiecewiseConstant
    data_file = ${sample_file}
    x_index_in_file = 0
    y_index_in_file = 4
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [walldd_fcn]
    type = PiecewiseConstant
    data_file = ${sample_file}
    x_index_in_file = 0
    y_index_in_file = 5
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [env_fcn]
    type = PiecewiseConstant
    data_file = ${sample_file}
    x_index_in_file = 0
    y_index_in_file = 6
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
[]

[Materials]
  [in_materials]
    type = GenericFunctionMaterial
    prop_names = 'vmStress temperature epStrain celldd walldd env'
    prop_values = 'vmStress_fcn temperature_fcn epStrain_fcn celldd_fcn walldd_fcn env_fcn'
  []
[]

[NEML2]
  input = 'models/laromance_matl_interp.i'
  [all]
    model = 'combined_model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL'
    moose_inputs =      'epStrain vmStress temperature celldd walldd env'
    neml2_inputs =      'state/ep state/s forces/T forces/cell_dd forces/wall_dd forces/env_fac'

    moose_output_types = 'MATERIAL MATERIAL MATERIAL'
    moose_outputs = 'ep_rate cell_rate wall_rate'
    neml2_outputs = 'state/ep_rate state/cell_rate state/wall_rate'
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-1 # Nothing is really being solved here, so loose tolerances are okay
  dt = 1
  dtmin=1
  end_time = 40 #242
  timestep_tolerance = 1e-3
[]

[Postprocessors]
  [cell_rate_pp]
    type = ElementAverageMaterialProperty
    mat_prop = cell_rate
  []
  [wall_rate_pp]
    type = ElementAverageMaterialProperty
    mat_prop = wall_rate
  []
  [creep_rate_pp]
    type = ElementAverageMaterialProperty
    mat_prop = ep_rate
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL TIMESTEP_END FINAL'
[]

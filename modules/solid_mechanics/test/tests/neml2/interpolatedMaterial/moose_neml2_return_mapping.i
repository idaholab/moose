# NEML2 file in MPA
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        new_system = true
        add_variables = true
        formulation = TOTAL
        volumetric_locking_correction = true
      []
    []
  []
[]

[BCs]
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [zfix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pressure_x]
    type = Pressure
    variable = disp_x
    boundary = right
    function = pressure_fcn
  []
[]

# -- interpolation grids in  'models/random_value_grid.json'
# "in_stress": [0.0, 30, 60, 100, 130, 160, 200, 230, 260, 320.0],
# "in_temperature": [600, 650, 750, 800, 850, 900, 950, 1000, 1100, 1200.0],
# "in_plastic_strain": [0.0, 0.0001, 0.001, 0.01],
# "in_cell": [1000000.0, 10000000000000.0],
# "in_wall": [1000000000000.0, 10000000000000.0],
# "in_env": [1e-09, 1e-06],

[Functions]
  [pressure_fcn]
    type = ParsedFunction
    expression = 200 #MPa
  []
[]
[AuxVariables]
  [T]
    initial_condition = 750 #K
  []
[]
[Materials]
  [init_dd]
    type = GenericConstantMaterial
    prop_names = 'init_cell_dd init_wall_dd init_envFac'
    prop_values = '1e11 8e12 2e-8'
  []
[]

[NEML2]
  # this is a NEML2 model definition, located under bison/data
  input = 'models/interp_matl_radial_return.i'
  model = 'model'
  verbose = true
  mode = PARSE_ONLY
  device = 'cpu'
  enable_AD = true
[]

[UserObjects]
  [temperature]
    type = MOOSEVariableToNEML2
    moose_variable = T
    neml2_variable = forces/T
  []
  [envFac]
    type = MOOSERealMaterialPropertyToNEML2
    moose_material_property = init_envFac
    neml2_variable = forces/env_fac
  []

  [strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = mechanical_strain
    neml2_variable = forces/E
  []
  [input_old_Ep]
    type = MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = plastic_strain
    neml2_variable = old_state/Ep
  []
  [input_old_ep]
    type = MOOSEOldRealMaterialPropertyToNEML2
    moose_material_property = effective_plastic_strain
    neml2_variable = old_state/ep
  []

  [old_cell_dd]
    type = MOOSEOldRealMaterialPropertyToNEML2
    moose_material_property = cell_dd
    neml2_variable = old_state/cell_dd
  []
  [old_wall_dd]
    type = MOOSEOldRealMaterialPropertyToNEML2
    moose_material_property = wall_dd
    neml2_variable = old_state/wall_dd
  []

  [model]
    type = ExecuteNEML2Model
    model = model
    gather_uos = 'temperature strain input_old_Ep input_old_ep envFac old_cell_dd old_wall_dd'
    enable_AD = true
  []
[]

[Materials]
  [neml2_stress_jacobian]
    type = NEML2StressToMOOSE
    execute_neml2_model_uo = model
    neml2_stress_output = state/S
    neml2_strain_input = forces/E
  []
  [neml2_flow_rate]
    type = NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/Ep
    moose_material_property = accumulated_inelastic_strain
  []
  [stateful_Ep]
    type = NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/Ep
    moose_material_property = plastic_strain
  []
  [stateful_ep]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/ep
    moose_material_property = effective_plastic_strain
  []
  [stateful_ep_rate]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/ep_rate
    moose_material_property = effective_plastic_strain_rate
  []
  [stateful_vm_stress]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/s
    moose_material_property = vonmises_stress
  []
  [cell_rate]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/cell_rate
    moose_material_property = cell_rate
  []
  [wall_rate]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/wall_rate
    moose_material_property = wall_rate
  []
  [cell_dd]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/cell_dd
    moose_material_property = cell_dd
    moose_material_property_init = init_cell_dd
  []
  [wall_dd]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/wall_dd
    moose_material_property = wall_dd
    moose_material_property_init = init_wall_dd
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  dt = 5
  dtmin = 5
  num_steps = 20
  residual_and_jacobian_together = true
[]

[Postprocessors]
  [effective_plastic_strain]
    type = ElementAverageMaterialProperty
    mat_prop = effective_plastic_strain
  []
  [creep_rate]
    type = ElementAverageMaterialProperty
    mat_prop = effective_plastic_strain_rate
  []
  [rhom_rate]
    type = ElementAverageMaterialProperty
    mat_prop = cell_rate
  []
  [rhoi_rate]
    type = ElementAverageMaterialProperty
    mat_prop = wall_rate
  []
  [rhom_dd]
    type = ElementAverageMaterialProperty
    mat_prop = cell_dd
  []
  [rhoi_dd]
    type = ElementAverageMaterialProperty
    mat_prop = wall_dd
  []
  [vm_stress]
    type = ElementAverageMaterialProperty
    mat_prop = vonmises_stress
  []
  [run_time]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  []
[]

[Outputs]
  csv = true
[]

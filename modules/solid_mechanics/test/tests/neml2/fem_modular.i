neml2_input = elasticity
N = 2

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = ${N}
    ny = ${N}
    nz = ${N}
  []
[]

[NEML2]
  input = 'models/${neml2_input}.i'
  model = 'model'
  verbose = true
  mode = PARSE_ONLY
  device = 'cpu'
[]

[UserObjects]
  active = 'model input_strain'
  [input_temperature]
    type = MOOSEVariableToNEML2
    moose_variable = T
    neml2_variable = forces/T
  []
  [input_strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = mechanical_strain
    neml2_variable = forces/E
  []
  [input_old_strain]
    type = MOOSEOldRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = mechanical_strain
    neml2_variable = old_forces/E
  []
  [input_old_stress]
    type = MOOSEOldRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = small_stress
    neml2_variable = old_state/S
  []
  [input_old_ep]
    type = MOOSEOldRealMaterialPropertyToNEML2
    moose_material_property = equivalent_plastic_strain
    neml2_variable = old_state/internal/ep
  []
  [input_old_Kp]
    type = MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = kinematic_plastic_strain
    neml2_variable = old_state/internal/Kp
  []
  [input_old_X1]
    type = MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = backstress_1
    neml2_variable = old_state/internal/X1
  []
  [input_old_X2]
    type = MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = backstress_2
    neml2_variable = old_state/internal/X2
  []
  [input_old_Ep]
    type = MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2
    moose_material_property = plastic_strain
    neml2_variable = old_state/internal/Ep
  []
  [input_old_gamma]
    type = MOOSEOldRealMaterialPropertyToNEML2
    moose_material_property = consistency_parameter
    neml2_variable = old_state/internal/gamma
  []

  [model]
    type = ExecuteNEML2Model
    model = model
    # add other gatherers here if needed
    gather_uos = 'input_strain'
  []
[]

[Materials]
  # add other outputs here if needed
  active = 'output_stress_jacobian'
  [output_stress_jacobian]
    type = NEML2StressToMOOSE
    execute_neml2_model_uo = model
    neml2_stress_output = state/S
    neml2_strain_input = forces/E
  []
  [output_ep]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/internal/ep
    moose_material_property = equivalent_plastic_strain
  []
  [output_Kp]
    type = NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/internal/Kp
    moose_material_property = kinematic_plastic_strain
  []
  [output_X1]
    type = NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/internal/X1
    moose_material_property = backstress_1
  []
  [output_X2]
    type = NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/internal/X2
    moose_material_property = backstress_2
  []
  [output_Ep]
    type = NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/internal/Ep
    moose_material_property = plastic_strain
  []
  [output_gamma]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    neml2_variable = state/internal/gamma
    moose_material_property = consistency_parameter
  []
[]

[AuxVariables]
  [T]
  []
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
  [xdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = t
    preset = false
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  dt = 1e-3
  dtmin = 1e-3
  num_steps = 5
  residual_and_jacobian_together = true
[]

[Outputs]
  file_base = '${neml2_input}'
  exodus = true
[]

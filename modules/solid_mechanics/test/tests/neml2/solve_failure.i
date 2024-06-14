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
  input = 'models/viscoplasticity_isokinharden.i'
  model = 'model'
  verbose = true
  mode = PARSE_ONLY
  device = 'cpu'
[]

[UserObjects]
  [gather_strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = mechanical_strain
    neml2_variable = forces/E
  []
  [gather_temperature]
    type = MOOSEVariableToNEML2
    moose_variable = T
    neml2_variable = forces/T
  []

  [model]
    type = ExecuteNEML2Model
    model = model
    # add gather_temperature here if needed
    gather_uos = 'gather_strain'
  []
[]

[Materials]
  [neml2_stress_jacobian]
    type = NEML2StressToMOOSE
    execute_neml2_model_uo = model
    neml2_stress_output = state/S
    neml2_strain_input = forces/E
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

[Postprocessors]
  [t]
    type = TimePostprocessor
    outputs = none
  []
  [dt]
    type = TimestepSize
    outputs = none
  []
[]

[UserObjects]
  [fail]
    type = Terminator
    expression = 't=3e-3 & dt=1e-3'
    fail_mode = SOFT
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  dt = 1e-3
  dtmin = 1e-4
  end_time = 5e-3
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]

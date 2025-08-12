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

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        new_system = true
        add_variables = true
        formulation = TOTAL
        volumetric_locking_correction = true
      []
    []
  []
[]

[NEML2]
  input = 'exact_kinematics_neml2.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'MATERIAL             MATERIAL             POSTPROCESSOR POSTPROCESSOR MATERIAL          MATERIAL'
    moose_inputs = '     deformation_gradient initial_orientation  time          time          plastic_defgrad   crss'
    neml2_inputs = '     forces/F             forces/r             forces/t      old_forces/t  old_state/Fp      old_state/tauc'

    moose_output_types = 'MATERIAL          MATERIAL        MATERIAL'
    moose_outputs = '     neml2_pk2_stress  plastic_defgrad crss'
    neml2_outputs = '     state/full_S      state/Fp        state/tauc'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = '     neml2_pk2_jacobian'
    neml2_derivatives = '     state/full_S forces/F'

    initialize_outputs = '      plastic_defgrad'
    initialize_output_values = 'initial_plastic_defgrad'
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = 'none'
  []
[]

[Materials]
  [copy]
    type = ComputeLagrangianStressCustomPK2
    custom_pk2_stress = 'neml2_pk2_stress'
    custom_pk2_jacobian = 'neml2_pk2_jacobian'
    large_kinematics = true
  []
  [initial_orientation]
    type = GenericConstantRealVectorValue
    vector_name = 'initial_orientation'
    vector_values = '-0.54412095 -0.34931944 0.12600655'
  []
  [initial_plastic_defgrad]
    type = GenericConstantRankTwoTensor
    tensor_name = 'initial_plastic_defgrad'
    tensor_values = '1 1 1'
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
  dt = 5e-3
  dtmin = 1e-3
  num_steps = 5
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]

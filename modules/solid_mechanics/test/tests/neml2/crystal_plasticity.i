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
  input = 'models/crystal_plasticity.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'MATERIAL                           POSTPROCESSOR POSTPROCESSOR MATERIAL                  MATERIAL                  MATERIAL'
    moose_inputs = '     spatial_velocity_increment         time          time          elastic_strain            orientation               slip_hardening'
    neml2_inputs = '     forces/spatial_velocity_increment  forces/t      old_forces/t  old_state/elastic_strain  old_state/orientation     old_state/internal/slip_hardening'

    moose_output_types = 'MATERIAL                            MATERIAL                  MATERIAL                  MATERIAL'
    moose_outputs = '     neml2_cauchy_stress                 elastic_strain            orientation               slip_hardening'
    neml2_outputs = '     state/internal/full_cauchy_stress   state/elastic_strain      state/orientation         state/internal/slip_hardening'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = '     neml2_cauchy_jacobian'
    neml2_derivatives = '     state/internal/full_cauchy_stress forces/spatial_velocity_increment'

    initialize_outputs = '      orientation'
    initialize_output_values = 'initial_orientation'
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Materials]
  [copy]
    type = ComputeLagrangianCauchyCustomStress
    custom_cauchy_stress = 'neml2_cauchy_stress'
    custom_cauchy_jacobian = 'neml2_cauchy_jacobian'
    large_kinematics = true
  []
  [initial_orientation]
    type = GenericConstantRealVectorValue
    vector_name = 'initial_orientation'
    vector_values = '-0.54412095 -0.34931944 0.12600655'
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
  file_base = 'crystal_plasticity'
  exodus = true
[]
